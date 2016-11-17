/*
  Copyright (C) 2014 Parrot SA

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.
  * Neither the name of Parrot nor the names
  of its contributors may be used to endorse or promote products
  derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/
/**
 * @file ARNETWORKAL_WifiNetwork.c
 * @brief wifi network manager allow to send over wifi network.
 * @date 25/04/2013
 * @author frederic.dhaeyer@parrot.com
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <stdlib.h>

#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <libARSAL/ARSAL.h>

#include <libARNetworkAL/ARNETWORKAL_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Error.h>
#include "ARNETWORKAL_Manager.h"
#include "Wifi/ARNETWORKAL_WifiNetwork.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORKAL_WIFINETWORK_TAG                     "ARNETWORKAL_WifiNetwork"
#define ARNETWORKAL_WIFINETWORK_SENDING_BUFFER_SIZE     (ARNETWORKAL_WIFINETWORK_MAX_DATA_BUFFER_SIZE + offsetof(ARNETWORKAL_Frame_t, dataPtr))
#define ARNETWORKAL_WIFINETWORK_RECEIVING_BUFFER_SIZE   (ARNETWORKAL_WIFINETWORK_MAX_DATA_BUFFER_SIZE + offsetof(ARNETWORKAL_Frame_t, dataPtr))

#define ARNETWORKAL_BW_PROGRESS_EACH_SEC 1
#define ARNETWORKAL_BW_NB_ELEMS 10

/*****************************************
 *
 *             private header:
 *
 *****************************************/

typedef struct _ARNETWORKAL_WifiNetworkObject_
{
    int socket;
    int socketBufferSize;
    int fifo[2];
    uint8_t *buffer;
    uint8_t *currentFrame;
    uint32_t size;
    uint32_t timeoutSec;
    struct timespec lastDataReceivedDate;
    uint8_t isDisconnected;
    uint8_t recvIsFlushed;
    ARNETWORKAL_Manager_OnDisconnect_t onDisconnect;
    void* onDisconnectCustomData;
    /* Bandwidth measure */
    ARSAL_Sem_t bw_sem;
    ARSAL_Sem_t bw_threadRunning;
    int bw_index;
    uint32_t bw_elements[ARNETWORKAL_BW_NB_ELEMS];
    uint32_t bw_current;
} ARNETWORKAL_WifiNetworkObject;


/**
 * @brief Check if the wifi network is stay too long without receive.
 * @param receiverObject wifi receiver object
 * @return 0 if is false and 1 in otherwise
 */
static uint8_t ARNETWORKAL_WifiNetwork_IsTooLongWithoutReceive(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_WifiNetworkObject *receiverObject);


/**
 * @brief Flush the receive socket.
 * @param receiverObject wifi receiver object
 */
static void ARNETWORKAL_WifiNetwork_FlushReceiveSocket (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_WifiNetworkObject *receiverObject);

/**
 * @brief Gets the available size of the send buffer
 * @param senderObject wifi sender object
 * @return The size (in bytes) available in the send buffer (negative means that the size could not be computed)
 */
static int ARNETWORKAL_WifiNetwork_GetAvailableSendSize (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_WifiNetworkObject *senderObject);

/*****************************************
 *
 *             implementation :
 *
 *****************************************/
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_New (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /* Check parameters */
    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    /* Allocate sender object */
    if(error == ARNETWORKAL_OK)
    {
        manager->senderObject = malloc(sizeof(ARNETWORKAL_WifiNetworkObject));
        if(manager->senderObject != NULL)
        {
            ARNETWORKAL_WifiNetworkObject *wifiObj = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
            wifiObj->socket = -1;
            wifiObj->socketBufferSize = -1;
            wifiObj->fifo[0] = -1;
            wifiObj->fifo[1] = -1;
            memset(&(wifiObj->lastDataReceivedDate), 0, sizeof(struct timespec));
            wifiObj->isDisconnected = 0;
            wifiObj->recvIsFlushed = 0;
            wifiObj->onDisconnect = NULL;
            wifiObj->onDisconnectCustomData = NULL;
            wifiObj->bw_index = 0;
            wifiObj->bw_current = 0;
            int i;
            for (i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
            {
                wifiObj->bw_elements[i] = 0;
            }
            ARSAL_Sem_Init (&wifiObj->bw_sem, 0, 0);
            ARSAL_Sem_Init (&wifiObj->bw_threadRunning, 0, 1);
        }
        else
        {
            error = ARNETWORKAL_ERROR_ALLOC;
        }
    }

    /* Allocate sender buffer */
    if(error == ARNETWORKAL_OK)
    {
        ((ARNETWORKAL_WifiNetworkObject *)manager->senderObject)->buffer = (uint8_t *)malloc(sizeof(uint8_t) * ARNETWORKAL_WIFINETWORK_SENDING_BUFFER_SIZE);
        if(((ARNETWORKAL_WifiNetworkObject *)manager->senderObject)->buffer != NULL)
        {
            ((ARNETWORKAL_WifiNetworkObject *)manager->senderObject)->size = 0;
            ((ARNETWORKAL_WifiNetworkObject *)manager->senderObject)->currentFrame = ((ARNETWORKAL_WifiNetworkObject *)manager->senderObject)->buffer;
        }
        else
        {
            error = ARNETWORKAL_ERROR_ALLOC;
        }
    }

    /* Allocate receiver object */
    if(error == ARNETWORKAL_OK)
    {
        manager->receiverObject = malloc(sizeof(ARNETWORKAL_WifiNetworkObject));
        if(manager->receiverObject != NULL)
        {
            ARNETWORKAL_WifiNetworkObject *wifiObj = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
            wifiObj->socket = -1;
            wifiObj->socketBufferSize = -1;
            wifiObj->fifo[0] = -1;
            wifiObj->fifo[1] = -1;
            memset(&(wifiObj->lastDataReceivedDate), 0, sizeof(struct timespec));
            wifiObj->isDisconnected = 0;
            wifiObj->recvIsFlushed = 0;
            wifiObj->onDisconnect = NULL;
            wifiObj->onDisconnectCustomData = NULL;
            wifiObj->bw_index = 0;
            wifiObj->bw_current = 0;
            int i;
            for (i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
            {
                wifiObj->bw_elements[i] = 0;
            }
            ARSAL_Sem_Init (&wifiObj->bw_sem, 0, 0);
            ARSAL_Sem_Init (&wifiObj->bw_threadRunning, 0, 1);
        }
        else
        {
            error = ARNETWORKAL_ERROR_ALLOC;
        }
    }

    /* Allocate receiver buffer */
    if(error == ARNETWORKAL_OK)
    {
        ((ARNETWORKAL_WifiNetworkObject *)manager->receiverObject)->buffer = (uint8_t *)malloc(sizeof(uint8_t) * ARNETWORKAL_WIFINETWORK_RECEIVING_BUFFER_SIZE);
        if(((ARNETWORKAL_WifiNetworkObject *)manager->receiverObject)->buffer != NULL)
        {
            ((ARNETWORKAL_WifiNetworkObject *)manager->receiverObject)->size = 0;
        }
        else
        {
            error = ARNETWORKAL_ERROR_ALLOC;
        }
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Signal(ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    int err = 0;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORKAL_OK)
    {
        char * buff = "x";
        if (manager->senderObject)
        {
            ARNETWORKAL_WifiNetworkObject *object = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
            if (object->fifo[1] != -1)
            {
                err = write (object->fifo[1], buff, 1);
                if (err < 0) {
                    err = errno;
                    ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "write() error: %d %s", err, strerror(err));
                }
            }
        }
        if (manager->receiverObject)
        {
            ARNETWORKAL_WifiNetworkObject *object = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
            if (object->fifo[1] != -1)
            {
                err = write (object->fifo[1], buff, 1);
                if (err < 0) {
                    err = errno;
                    ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "write() error: %d %s", err, strerror(err));
                }
            }
        }
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetBandwidth (ARNETWORKAL_Manager_t *manager, uint32_t *uploadBw, uint32_t *downloadBw)
{
    eARNETWORKAL_ERROR err = ARNETWORKAL_OK;
    if (manager == NULL ||
        manager->senderObject == NULL ||
        manager->receiverObject == NULL)
    {
        err = ARNETWORKAL_ERROR_BAD_PARAMETER;
        return err;
    }

    ARNETWORKAL_WifiNetworkObject *sender = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    ARNETWORKAL_WifiNetworkObject *reader = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;

    if (uploadBw != NULL)
    {
        uint32_t up = 0;
        int i;
        for (i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
        {
            up += sender->bw_elements[i];
        }
        up /= (ARNETWORKAL_BW_NB_ELEMS * ARNETWORKAL_BW_PROGRESS_EACH_SEC);
        *uploadBw = up;
    }
    if (downloadBw != NULL)
    {
        uint32_t down = 0;
        int i;
        for (i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
        {
            down += reader->bw_elements[i];
        }
        down /= (ARNETWORKAL_BW_NB_ELEMS * ARNETWORKAL_BW_PROGRESS_EACH_SEC);
        *downloadBw = down;
    }
    return err;
}

void *ARNETWORKAL_WifiNetwork_BandwidthThread (void *param)
{
    if (param == NULL)
    {
        return (void *)0;
    }

    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *)param;
    ARNETWORKAL_WifiNetworkObject *sender = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    ARNETWORKAL_WifiNetworkObject *reader = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;

    ARSAL_Sem_Wait (&sender->bw_threadRunning);
    ARSAL_Sem_Wait (&reader->bw_threadRunning);

    const struct timespec timeout = {
        .tv_sec = ARNETWORKAL_BW_PROGRESS_EACH_SEC,
        .tv_nsec = 0,
    };
    // We read only on sender sem as both will be set when closing
    int waitRes = ARSAL_Sem_Timedwait (&sender->bw_sem, &timeout);
    int loopCondition = (waitRes == -1) && (errno == ETIMEDOUT);
    while (loopCondition)
    {
        sender->bw_index++;
        sender->bw_index %= ARNETWORKAL_BW_NB_ELEMS;
        sender->bw_elements[sender->bw_index] = sender->bw_current;
        sender->bw_current = 0;

        reader->bw_index++;
        reader->bw_index %= ARNETWORKAL_BW_NB_ELEMS;
        reader->bw_elements[reader->bw_index] = reader->bw_current;
        reader->bw_current = 0;

        // Update loop condition
        waitRes = ARSAL_Sem_Timedwait (&sender->bw_sem, &timeout);
        loopCondition = (waitRes == -1) && (errno == ETIMEDOUT);
    }

    ARSAL_Sem_Post (&reader->bw_threadRunning);
    ARSAL_Sem_Post (&sender->bw_threadRunning);

    return (void *)0;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Cancel (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    /* do nothink : wifi connection is not blocking */

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Delete (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        if (manager->senderObject)
        {
            ARNETWORKAL_WifiNetworkObject *sender = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;

            if (sender->socket != -1)
            {
                ARSAL_Socket_Close(sender->socket);
                sender->socket = -1;
            }

            close (sender->fifo[0]);
            close (sender->fifo[1]);

            if(sender->buffer)
            {
                free (sender->buffer);
                sender->buffer = NULL;
            }

            ARSAL_Sem_Post (&sender->bw_sem);
            ARSAL_Sem_Wait (&sender->bw_threadRunning);
            ARSAL_Sem_Destroy (&sender->bw_sem);
            ARSAL_Sem_Destroy (&sender->bw_threadRunning);

            free (manager->senderObject);
            manager->senderObject = NULL;
        }

        if(manager->receiverObject)
        {
            ARNETWORKAL_WifiNetworkObject *reader = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;

            if (reader->socket != -1)
            {
                ARSAL_Socket_Close(reader->socket);
                reader->socket = -1;
            }

            close (reader->fifo[0]);
            close (reader->fifo[1]);

            if(reader->buffer)
            {
                free (reader->buffer);
                reader->buffer = NULL;
            }

            ARSAL_Sem_Post (&reader->bw_sem);
            ARSAL_Sem_Wait (&reader->bw_threadRunning);
            ARSAL_Sem_Destroy (&reader->bw_sem);
            ARSAL_Sem_Destroy (&reader->bw_threadRunning);

            free (manager->receiverObject);
            manager->receiverObject = NULL;
        }
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Connect (ARNETWORKAL_Manager_t *manager, const char *addr, int port)
{
    /** -- Connect the socket in UDP to a port of an address -- */
    /** local declarations */
    struct sockaddr_in sendSin;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    ARNETWORKAL_WifiNetworkObject *wifiSender = NULL;
    int err;

    /** Check parameters */
    if((manager == NULL) || (manager->senderObject == NULL))
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    /** Create sender Object */
    if(error == ARNETWORKAL_OK)
    {
        wifiSender = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
        wifiSender->socket = ARSAL_Socket_Create (AF_INET, SOCK_DGRAM, 0);
        if(wifiSender->socket < 0)
        {
            error = ARNETWORKAL_ERROR_WIFI_SOCKET_CREATION;
        }
        if (pipe(wifiSender->fifo) != 0)
        {
            error = ARNETWORKAL_ERROR_FIFO_INIT;
        }
    }

    /** Initialize socket */
    if(error == ARNETWORKAL_OK)
    {
        int sockfd = wifiSender->socket;
#if HAVE_DECL_SO_NOSIGPIPE
        /* Remove SIGPIPE */
        int set = 1;
        ARSAL_Socket_Setsockopt (sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

        /* get the socket buffer size */
        int bufferSize;
        socklen_t size = sizeof (bufferSize);
        err = ARSAL_Socket_Getsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&bufferSize, &size);
        if (err < 0) {
            err = errno;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "ARSAL_Socket_Getsockopt() failed; err=%d", err);
        }

        wifiSender->socketBufferSize = bufferSize;

        sendSin.sin_addr.s_addr = inet_addr (addr);
        sendSin.sin_family = AF_INET;
        sendSin.sin_port = htons (port);

        int flags = fcntl(sockfd, F_GETFL, 0);
        err = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
        if (err < 0) {
            err = errno;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "fcntl() failed; err=%d", err);
        }

        err = ARSAL_Socket_Connect (sockfd, (struct sockaddr*) &sendSin, sizeof (sendSin));
        if (err < 0)
        {
            err = errno;
            switch (err)
            {
            case EACCES:
                error = ARNETWORKAL_ERROR_WIFI_SOCKET_PERMISSION_DENIED;
                break;

            default:
                ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] connect fd=%d addr='%s' port=%d: error='%s'", manager, sockfd, addr, port, strerror(err));
                error = ARNETWORKAL_ERROR_WIFI;
                break;
            }
        }
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Bind (ARNETWORKAL_Manager_t *manager, unsigned short port, int timeoutSec)
{
    /** -- receiving data present on the socket -- */

    /** local declarations */
    struct timespec timeout;
    struct sockaddr_in recvSin;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    int err = 0;
    ARNETWORKAL_WifiNetworkObject *wifiReceiver = NULL;
    int flags = 0;

    /** Check parameters */
    if((manager == NULL) || (manager->receiverObject == NULL))
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    /** Create sender Object */
    if(error == ARNETWORKAL_OK)
    {
        wifiReceiver = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;

        wifiReceiver->socket = ARSAL_Socket_Create (AF_INET, SOCK_DGRAM, 0);
        if (wifiReceiver->socket < 0)
        {
            error = ARNETWORKAL_ERROR_WIFI_SOCKET_CREATION;
        }
        if (pipe(wifiReceiver->fifo) != 0)
        {
            error = ARNETWORKAL_ERROR_FIFO_INIT;
        }
        wifiReceiver->timeoutSec = timeoutSec;
    }

    /** socket initialization */
    if(error == ARNETWORKAL_OK)
    {
        recvSin.sin_addr.s_addr = htonl (INADDR_ANY);
        recvSin.sin_family = AF_INET;
        recvSin.sin_port = htons (port);

        /** set the socket timeout */
        timeout.tv_sec = timeoutSec;
        timeout.tv_nsec = 0;
        err = ARSAL_Socket_Setsockopt (wifiReceiver->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));
        if (err < 0) {
            err = errno;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "ARSAL_Socket_Setsockopt() failed; err=%d", err);
        }

        /* set the socket non blocking */
        flags = fcntl(wifiReceiver->socket, F_GETFL, 0);
        err = fcntl(wifiReceiver->socket, F_SETFL, flags | O_NONBLOCK);
        if (err < 0) {
            err = errno;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "fcntl() failed; err=%d", err);
        }

        err = ARSAL_Socket_Bind (wifiReceiver->socket, (struct sockaddr*)&recvSin, sizeof (recvSin));
        if (err < 0)
        {
            err = errno;
            switch (err)
            {
            case EACCES:
                error = ARNETWORKAL_ERROR_WIFI_SOCKET_PERMISSION_DENIED;
                break;

            default:
                ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] bind fd=%d, addr='0.0.0.0', port=%d: error='%s'", manager, wifiReceiver->socket, port, strerror(err));
                error = ARNETWORKAL_ERROR_WIFI;
                break;
            }
        }
    }

    return error;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_PushFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;
    ARNETWORKAL_WifiNetworkObject *wifiSendObj = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    uint32_t nextSize = wifiSendObj->size + frame->size;

    if(nextSize > ARNETWORKAL_WIFINETWORK_SENDING_BUFFER_SIZE)
    {
        result = ARNETWORKAL_MANAGER_RETURN_BUFFER_FULL;
    }

    if(result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        int availableSpaceInSocket = ARNETWORKAL_WifiNetwork_GetAvailableSendSize (manager, wifiSendObj);
        if (availableSpaceInSocket < 0)
        {
            // We could not get the available size, accept the frame anyway
        }
        else if (((uint32_t)availableSpaceInSocket) < nextSize)
        {
            // The available size is too small for the frame
            result = ARNETWORKAL_MANAGER_RETURN_BUFFER_FULL;
        }
    }

    if(result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        uint32_t droneEndianUInt32 = 0;

        /** Add type */
        memcpy (wifiSendObj->currentFrame, &frame->type, sizeof (uint8_t));
        wifiSendObj->currentFrame += sizeof (uint8_t);
        wifiSendObj->size += sizeof (uint8_t);

        /** Add frame type */
        memcpy (wifiSendObj->currentFrame, &frame->id, sizeof (uint8_t));
        wifiSendObj->currentFrame += sizeof (uint8_t);
        wifiSendObj->size += sizeof (uint8_t);

        /** Add frame sequence number */
        memcpy (wifiSendObj->currentFrame, &(frame->seq), sizeof (uint8_t));
        wifiSendObj->currentFrame += sizeof (uint8_t);
        wifiSendObj->size += sizeof (uint8_t);

        /** Add frame size */
        droneEndianUInt32 =  htodl (frame->size);
        memcpy (wifiSendObj->currentFrame, &droneEndianUInt32, sizeof (uint32_t));
        wifiSendObj->currentFrame += sizeof (uint32_t);
        wifiSendObj->size += sizeof (uint32_t);

        /** Add frame data */
        uint32_t dataSize = frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);
        memcpy (wifiSendObj->currentFrame, frame->dataPtr, dataSize);
        wifiSendObj->currentFrame += dataSize;
        wifiSendObj->size += dataSize;

        if (manager->dumpFile != NULL)
        {
            ARSAL_Print_DumpData (manager->dumpFile, ARNETWORKAL_DUMP_TAG_FRAME_PUSHED, wifiSendObj->currentFrame - frame->size, frame->size, 0, NULL);
        }
    }

    return result;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_PopFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;
    ARNETWORKAL_WifiNetworkObject *wifiRecvObj = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;

    /** -- get a Frame of the receiving buffer -- */
    /** if the receiving buffer not contain enough data for the frame head*/
    if (wifiRecvObj->currentFrame > ((wifiRecvObj->buffer + wifiRecvObj->size) - offsetof (ARNETWORKAL_Frame_t, dataPtr)))
    {
        if (wifiRecvObj->currentFrame == (wifiRecvObj->buffer + wifiRecvObj->size))
        {
            result = ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY;
        }
        else
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /** Get the frame from the buffer */
        /** get type */
        memcpy (&(frame->type), wifiRecvObj->currentFrame, sizeof (uint8_t));
        wifiRecvObj->currentFrame += sizeof (uint8_t) ;

        /** get id */
        memcpy (&(frame->id), wifiRecvObj->currentFrame, sizeof (uint8_t));
        wifiRecvObj->currentFrame += sizeof (uint8_t);

        /** get seq */
        memcpy (&(frame->seq), wifiRecvObj->currentFrame, sizeof (uint8_t));
        wifiRecvObj->currentFrame += sizeof (uint8_t);

        /** get size */
        memcpy (&(frame->size), wifiRecvObj->currentFrame, sizeof (uint32_t));
        wifiRecvObj->currentFrame += sizeof(uint32_t);
        /** convert the endianness */
        frame->size = dtohl (frame->size);

        /** get data address */
        frame->dataPtr = wifiRecvObj->currentFrame;

        /** if the receiving buffer not contain enough data for the full frame */
        if (wifiRecvObj->currentFrame > ((wifiRecvObj->buffer + wifiRecvObj->size) - (frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr))))
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /** offset the readingPointer on the next frame */
        wifiRecvObj->currentFrame = wifiRecvObj->currentFrame + frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);

        if (manager->dumpFile != NULL)
        {
            ARSAL_Print_DumpData (manager->dumpFile, ARNETWORKAL_DUMP_TAG_FRAME_POPPED, wifiRecvObj->currentFrame - frame->size, frame->size, 0, NULL);
        }
    }
    else
    {
        /** reset the reading pointer to the start of the buffer */
        wifiRecvObj->currentFrame = wifiRecvObj->buffer;
        wifiRecvObj->size = 0;

        /** reset frame */
        frame->type = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED;
        frame->id = 0;
        frame->seq = 0;
        frame->size = 0;
        frame->dataPtr = NULL;
    }

    return result;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_Send(ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;
    ARNETWORKAL_WifiNetworkObject *senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    ARNETWORKAL_WifiNetworkObject *receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
    int err = 0;

    if(senderObject->size != 0)
    {
        ssize_t bytes = ARSAL_Socket_Send(senderObject->socket, senderObject->buffer, senderObject->size, 0);
        if(bytes > -1)
        {
            if (manager->dumpFile != NULL)
            {
                ARSAL_Print_DumpData (manager->dumpFile, ARNETWORKAL_DUMP_TAG_DATA_SENT, senderObject->buffer, senderObject->size, 0, NULL);
            }
            senderObject->size = 0;
            senderObject->currentFrame = senderObject->buffer;
            senderObject->bw_current += bytes;
        }
        else
        {
            err = errno;
            switch (err)
            {
            case EAGAIN:
                ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Socket buffer full (errno = %d , %s)", manager, err, strerror(err));
                senderObject->size = 0;
                senderObject->currentFrame = senderObject->buffer;
                break;
            default:
                ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Socket send error (errno = %d , %s)", manager, err, strerror(err));
                /* check the disconnection */
                if (senderObject->isDisconnected == 0)
                {
                    /* wifi disconnected */
                    senderObject->isDisconnected = 1;

                    if ((senderObject->onDisconnect != NULL) && ((receiverObject == NULL) || (receiverObject->isDisconnected == 0)))
                    {
                        /* Disconnect callback */
                        senderObject->onDisconnect (manager, senderObject->onDisconnectCustomData);
                    }
                }
                break;
            }

            result = ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
        }
    }

    return result;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_Receive(ARNETWORKAL_Manager_t *manager)
{

    /** -- receiving data present on the socket -- */

    /** local declarations */
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;
    ARNETWORKAL_WifiNetworkObject *receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
    ARNETWORKAL_WifiNetworkObject *senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;

    // Create a fd_set to select on both the socket and the "cancel" pipe
    fd_set set;
    fd_set exceptSet;
    FD_ZERO (&set);
    FD_SET (receiverObject->socket, &set);
    FD_SET (receiverObject->fifo[0], &set);
    FD_ZERO (&exceptSet);
    FD_SET (receiverObject->socket, &exceptSet);
    FD_SET (receiverObject->fifo[0], &exceptSet);
    // Get the max fd +1 for select call
    int maxFd = (receiverObject->socket > receiverObject->fifo[0]) ? receiverObject->socket +1 : receiverObject->fifo[0] +1;
    // Create the timeout object
    struct timeval tv = { receiverObject->timeoutSec, 0 };

    /* initialize the lastDataReceivedDate at the first running of the function */
    if ((receiverObject->lastDataReceivedDate.tv_sec == 0) && (receiverObject->lastDataReceivedDate.tv_nsec == 0))
    {
        ARSAL_Time_GetTime(&(receiverObject->lastDataReceivedDate));
    }

    // Wait for either file to be reading for a read
    int err = select (maxFd, &set, NULL, &exceptSet, &tv);

    if (FD_ISSET(receiverObject->socket, &exceptSet))
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "FOUND SOCKET ERROR FD_ISSET(except) %d", FD_ISSET(receiverObject->socket, &exceptSet));
    }
    if (err < 0)
    {
        // Read error
        result = ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
        receiverObject->size = 0;

        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "select ERROR err %d", err);
    }
    else
    {
        // No read error (Timeout or FD ready)
        if (FD_ISSET(receiverObject->socket, &set))
        {
            /* If wifi network is too long without receive */
            if ((receiverObject->recvIsFlushed == 0 ) && (ARNETWORKAL_WifiNetwork_IsTooLongWithoutReceive(manager, receiverObject) != 0))
            {
                /* the data in the socket are too old*/
                /* flush the socket  */
                ARNETWORKAL_WifiNetwork_FlushReceiveSocket (manager, receiverObject);
            }
            else
            {
                // If the socket is ready, read data
                int size = ARSAL_Socket_Recv (receiverObject->socket, receiverObject->buffer, ARNETWORKAL_WIFINETWORK_RECEIVING_BUFFER_SIZE, 0);
                if (size > 0)
                {
                    // Save the number of bytes read
                    receiverObject->size = size;
                    receiverObject->bw_current += size;

                    if (manager->dumpFile != NULL)
                    {
                        ARSAL_Print_DumpData (manager->dumpFile, ARNETWORKAL_DUMP_TAG_DATA_RECEIVED, receiverObject->buffer, receiverObject->size, 0, NULL);
                    }

                    /* Data received reset the reception flush state */
                    receiverObject->recvIsFlushed = 0;
                }
                else if (size == 0)
                {
                    // Should never go here (if the socket is ready, some data must be available)
                    // But the case in handled.
                    result = ARNETWORKAL_MANAGER_RETURN_NO_DATA_AVAILABLE;
                    receiverObject->size = 0;
                }
                else
                {
                    // Error in recv call
                    result = ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
                    receiverObject->size = 0;
                }

                /* Check if the thread has not been stopped too long between ARSAL_Socket_Recv() and the save of the reception date. */
                if (ARNETWORKAL_WifiNetwork_IsTooLongWithoutReceive(manager, receiverObject) == 0)
                {
                    /* save the date of the reception */
                    ARSAL_Time_GetTime(&(receiverObject->lastDataReceivedDate));
                }
            }
        }
        else
        {
            // If the socket is not ready, it is either a timeout or a signal
            // In any case, report this as a "no data" call
            result = ARNETWORKAL_MANAGER_RETURN_NO_DATA_AVAILABLE;
            receiverObject->size = 0;

            /* check the disconnection */
            if ((receiverObject->isDisconnected != 1) && (! FD_ISSET(receiverObject->fifo[0], &set)))
            {
                /* check if the connection is lost */
                if (ARNETWORKAL_WifiNetwork_IsTooLongWithoutReceive(manager, receiverObject) != 0)
                {
                    /* wifi disconnected */
                    receiverObject->isDisconnected = 1;

                    if ((receiverObject->onDisconnect != NULL) && ((senderObject == NULL) || (senderObject->isDisconnected == 0)))
                    {
                        ARSAL_PRINT(ARSAL_PRINT_INFO, ARNETWORKAL_WIFINETWORK_TAG, "[%p] connection lost (too long time without reception)", manager);

                        /* Disconnect callback */
                        receiverObject->onDisconnect (manager, receiverObject->onDisconnectCustomData);
                    }
                }
            }
        }

        if (FD_ISSET(receiverObject->fifo[0], &set))
        {
            // If the fifo is ready for a read, dump bytes from it (so it won't be ready next time)
            char dump[10];
            err = read (receiverObject->fifo[0], &dump, 10);
            if (err < 0) {
                err = errno;
                ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "read() failed; err=%d", err);
            }
        }
    }

    receiverObject->currentFrame = receiverObject->buffer;

    return result;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetOnDisconnectCallback (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData)
{
    /* -- set the OnDisconnect Callback -- */

    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    ARNETWORKAL_WifiNetworkObject *receiverObject = NULL;
    ARNETWORKAL_WifiNetworkObject *senderObject = NULL;

    if ((manager == NULL) || (onDisconnectCallback == NULL))
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */

    if (error == ARNETWORKAL_OK)
    {
        receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
        if (receiverObject == NULL)
        {
            error = ARNETWORKAL_ERROR_BAD_PARAMETER;
        }
        /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */
    }
    /* No else: skipped by an error */

    if (error == ARNETWORKAL_OK)
    {
        senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
        if (senderObject == NULL)
        {
            error = ARNETWORKAL_ERROR_BAD_PARAMETER;
        }
        /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */
    }
    /* No else: skipped by an error */

    if (error == ARNETWORKAL_OK)
    {
        receiverObject->onDisconnect = onDisconnectCallback;
        receiverObject->onDisconnectCustomData = customData;

        senderObject->onDisconnect = onDisconnectCallback;
        senderObject->onDisconnectCustomData = customData;
    }
    /* No else: skipped by an error */

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetSendBufferSize(ARNETWORKAL_Manager_t *manager, int bufferSize)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    ARNETWORKAL_WifiNetworkObject *senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    int err = ARSAL_Socket_Setsockopt (senderObject->socket, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    if (err == 0)
    {
        eARNETWORKAL_ERROR getErr = ARNETWORKAL_WifiNetwork_GetSendBufferSize(manager, &(senderObject->socketBufferSize));
        if (getErr != ARNETWORKAL_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Unable to get back send socket buffer size, using set-value", manager);
            senderObject->socketBufferSize = bufferSize;
        }
        ARSAL_PRINT(ARSAL_PRINT_INFO, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Setting send socket size to %d, actual size is %d", manager, bufferSize, senderObject->socketBufferSize);
    }
    else
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error while setting send socket buffer size", manager);
        error = ARNETWORKAL_ERROR_WIFI_SOCKET_SETOPT;
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetRecvBufferSize(ARNETWORKAL_Manager_t *manager, int bufferSize)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    ARNETWORKAL_WifiNetworkObject *receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
    int err = ARSAL_Socket_Setsockopt (receiverObject->socket, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
    if (err == 0)
    {
        eARNETWORKAL_ERROR getErr = ARNETWORKAL_WifiNetwork_GetRecvBufferSize(manager, &(receiverObject->socketBufferSize));
        if (getErr != ARNETWORKAL_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Unable to get back recv socket buffer size, using set-value", manager);
            receiverObject->socketBufferSize = bufferSize;
        }
        ARSAL_PRINT(ARSAL_PRINT_INFO, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Setting recv socket size to %d, actual size is %d", manager, bufferSize, receiverObject->socketBufferSize);
    }
    else
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error while setting recv socket buffer size", manager);
        error = ARNETWORKAL_ERROR_WIFI_SOCKET_SETOPT;
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetSendBufferSize(ARNETWORKAL_Manager_t *manager, int *bufferSize)
{
    if (bufferSize == NULL) { return ARNETWORKAL_ERROR_BAD_PARAMETER; }
    socklen_t size = sizeof(*bufferSize);
    ARNETWORKAL_WifiNetworkObject *senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    int err = ARSAL_Socket_Getsockopt (senderObject->socket, SOL_SOCKET, SO_SNDBUF, bufferSize, &size);
    // The kernel doubles the size we put on setsockopt, so we divide by two to get the usable size
    *bufferSize /= 2;
    eARNETWORKAL_ERROR error = (err == 0) ? ARNETWORKAL_OK : ARNETWORKAL_ERROR_WIFI_SOCKET_GETOPT;
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetRecvBufferSize(ARNETWORKAL_Manager_t *manager, int *bufferSize)
{
    if (bufferSize == NULL) { return ARNETWORKAL_ERROR_BAD_PARAMETER; }
    socklen_t size = sizeof(*bufferSize);
    ARNETWORKAL_WifiNetworkObject *receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
    int err = ARSAL_Socket_Getsockopt (receiverObject->socket, SOL_SOCKET, SO_RCVBUF, bufferSize, &size);
    // The kernel doubles the size we put on setsockopt, so we divide by two to get the usable size
    *bufferSize /= 2;
    eARNETWORKAL_ERROR error = (err == 0) ? ARNETWORKAL_OK : ARNETWORKAL_ERROR_WIFI_SOCKET_GETOPT;
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetSendClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR classSelector)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_ERROR;
    ARNETWORKAL_WifiNetworkObject *senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
	int cs = classSelector;
	error = ARNETWORKAL_OK;
	int err = ARSAL_Socket_Setsockopt (senderObject->socket, IPPROTO_IP, IP_TOS, &cs, sizeof(cs));
	if (err != 0)
	{
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error while setting recv socket class selector", manager);
            error = ARNETWORKAL_ERROR_WIFI_SOCKET_SETOPT;
	}
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetRecvClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR classSelector)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_ERROR;
    ARNETWORKAL_WifiNetworkObject *receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
    int cs = classSelector;
	error = ARNETWORKAL_OK;
	int err = ARSAL_Socket_Setsockopt (receiverObject->socket, IPPROTO_IP, IP_TOS, &cs, sizeof(cs));
	if (err != 0)
	{
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error while setting recv socket class selector", manager);
            error = ARNETWORKAL_ERROR_WIFI_SOCKET_SETOPT;
	}
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetSendClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR *classSelector)
{
    if (classSelector == NULL) { return ARNETWORKAL_ERROR_BAD_PARAMETER; }
    int cs = -1;
    socklen_t size = sizeof(cs);
    ARNETWORKAL_WifiNetworkObject *senderObject = (ARNETWORKAL_WifiNetworkObject *)manager->senderObject;
    int err = ARSAL_Socket_Getsockopt (senderObject->socket, IPPROTO_IP, IP_TOS, &cs, &size);
    eARNETWORKAL_ERROR error = (err == 0) ? ARNETWORKAL_OK : ARNETWORKAL_ERROR_WIFI_SOCKET_GETOPT;
    if (err == 0)
    {
        *classSelector = cs;
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetRecvClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR *classSelector)
{
    if (classSelector == NULL) { return ARNETWORKAL_ERROR_BAD_PARAMETER; }
    int cs = -1;
    socklen_t size = sizeof(cs);
    ARNETWORKAL_WifiNetworkObject *receiverObject = (ARNETWORKAL_WifiNetworkObject *)manager->receiverObject;
    int err = ARSAL_Socket_Getsockopt (receiverObject->socket, IPPROTO_IP, IP_TOS, &cs, &size);
    eARNETWORKAL_ERROR error = (err == 0) ? ARNETWORKAL_OK : ARNETWORKAL_ERROR_WIFI_SOCKET_GETOPT;
    if (err == 0)
    {
        *classSelector = cs;
    }
    return error;
}


/*****************************************
 *
 *             private implementation :
 *
 *****************************************/

static uint8_t ARNETWORKAL_WifiNetwork_IsTooLongWithoutReceive (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_WifiNetworkObject *receiverObject)
{
    /* -- Check if the wifi network is stay too long without receive. -- */

    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    uint8_t isTooLongWithoutReceive = 0;
    struct timespec currentDate = {0, 0};
    int32_t timeWithoutReception = 0; /* time in millisecond */

    /* check parameters */
    if(receiverObject == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */

    if (error == ARNETWORKAL_OK)
    {
        /* get the time without reception */
        ARSAL_Time_GetTime(&currentDate);
        timeWithoutReception = ARSAL_Time_ComputeTimespecMsTimeDiff (&(receiverObject->lastDataReceivedDate), &currentDate);

        /* Check if the wifi network is stay too long without receive */
        if (timeWithoutReception > ARNETWORKAL_WIFINETWORK_DISCONNECT_TIMEOUT_MS)
        {
            isTooLongWithoutReceive = 1;
        }
    }
    /* No else: skipped by an error */

    if(error != ARNETWORKAL_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error occurred : %s", manager, ARNETWORKAL_Error_ToString (error));
    }
    /* No else: no error to print */

    return isTooLongWithoutReceive;
}

static void ARNETWORKAL_WifiNetwork_FlushReceiveSocket (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_WifiNetworkObject *receiverObject)
{
    /* -- flush the receive socket -- */

    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    int sizeRecv = 0;
    int err = 0;


    /* check parameters */
    if(receiverObject == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */

    if (error == ARNETWORKAL_OK)
    {
        while ((receiverObject->recvIsFlushed == 0) && (error == ARNETWORKAL_OK))
        {
            sizeRecv = ARSAL_Socket_Recv (receiverObject->socket, receiverObject->buffer, ARNETWORKAL_WIFINETWORK_RECEIVING_BUFFER_SIZE, 0);

            if (sizeRecv == 0)
            {
                /* Socket shutdown */
                receiverObject->recvIsFlushed = 1;
            }
            else if (sizeRecv == -1)
            {
                err = errno;
                switch (err)
                {
                case EAGAIN:
                    /* No data */
                    receiverObject->recvIsFlushed = 1;
                    break;

                default:
                    ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] error = %d (%s)", manager, err, strerror(err));
                    error = ARNETWORKAL_ERROR_WIFI;
                    break;
                }
            }
            /* No Else : data has been read and dropped */
        }
    }
    /* No else: skipped by an error */

    if(error != ARNETWORKAL_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error occurred : %s", manager, ARNETWORKAL_Error_ToString (error));
    }
    /* No else: no error to print */
}

static int ARNETWORKAL_WifiNetwork_GetAvailableSendSize (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_WifiNetworkObject *senderObject)
{
    int currentBytesInSocket;
    int err;
    int sockfd = senderObject->socket;
    int buffSize = senderObject->socketBufferSize;
    int available = -1;
    if (buffSize < 0)
    {
        return -1;
    }

    err = ioctl(sockfd, TIOCOUTQ, &currentBytesInSocket);
    if (err >= 0)
    {
        available = buffSize - currentBytesInSocket;
        if (available < 0)
        {
            ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Available size %d < 0 ! (buff = %d, current = %d)", manager, available, buffSize, currentBytesInSocket);
            available = 0; // Set to 0 so we will refuse the data
        }
    }
    else
    {
        err = errno;
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_WIFINETWORK_TAG, "[%p] Error during ioctl %d (%s)", manager, err, strerror(err));
        if (err == ENXIO)
        {
            // On iOS (and maybe other system), the ioctl(...TIOCOUTQ...) is not supported and fails with errno ENXIO
            // In this case, we set the socket buffer size to -1 to avoid future calls to the ioctl
            ARSAL_PRINT(ARSAL_PRINT_INFO, ARNETWORKAL_WIFINETWORK_TAG, "[%p] ioctl failed with error ENXIO, stop trying to get available socket buffer size", manager);
            senderObject->socketBufferSize = -1;
        }
    }

    return available;
}
