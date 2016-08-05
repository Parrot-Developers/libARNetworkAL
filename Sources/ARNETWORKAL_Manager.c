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
 * @file ARNETWORKAL_Manager.c
 * @brief network manager allow to send over network.
 * @date 25/04/2013
 * @author frederic.dhaeyer@parrot.com
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/
#include <config.h>
#include <stdlib.h>

#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <libARSAL/ARSAL_Print.h>

#include <libARNetworkAL/ARNETWORKAL_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Error.h>
#include "Wifi/ARNETWORKAL_WifiNetwork.h"

#if defined(HAVE_COREBLUETOOTH_COREBLUETOOTH_H)
#include "BLE/ARNETWORKAL_BLENetwork.h"
#endif

#include "ARNETWORKAL_Manager.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORKAL_MANAGER_TAG "ARNETWORKAL_Manager"

/*****************************************
 *
 *             private header:
 *
 *****************************************/


/*****************************************
 *
 *             implementation :
 *
 *****************************************/
ARNETWORKAL_Manager_t* ARNETWORKAL_Manager_New (eARNETWORKAL_ERROR *error)
{
    /** -- Create a new Manager -- */

    /** local declarations */
    ARNETWORKAL_Manager_t *manager = NULL;
    eARNETWORKAL_ERROR localError = ARNETWORKAL_OK;
    /** Create the Manager */
    manager = calloc (1, sizeof(ARNETWORKAL_Manager_t));
    if (manager != NULL)
    {
        /** Initialize to default values */
        manager->maxIds = ARNETWORKAL_MANAGER_DEFAULT_ID_MAX;
    }
    else
    {
        localError = ARNETWORKAL_ERROR_ALLOC;
    }

    /** delete the Manager if an error occurred */
    if (localError != ARNETWORKAL_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_MANAGER_TAG, "[%p] error: %d occurred \n", manager, localError);
        ARNETWORKAL_Manager_Delete (&manager);
    }

    /** return the error */
    if (error != NULL)
    {
        *error = localError;
    }

    return manager;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitWifiNetwork (ARNETWORKAL_Manager_t *manager, const char *addr, int sendingPort, int receivingPort, int recvTimeoutSec)
{
    /** -- Initialize the Wifi Network -- */

    /** local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /** check paratemters*/
    if ((manager == NULL) || (addr == NULL))
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_WifiNetwork_New(manager);
    }

    if (error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_WifiNetwork_Connect (manager, addr, sendingPort);
    }

    if (error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_WifiNetwork_Bind (manager, receivingPort, recvTimeoutSec);
    }

    if(error == ARNETWORKAL_OK)
    {
        manager->pushFrame = ARNETWORKAL_WifiNetwork_PushFrame;
        manager->popFrame = ARNETWORKAL_WifiNetwork_PopFrame;
        manager->send = ARNETWORKAL_WifiNetwork_Send;
        manager->receive = ARNETWORKAL_WifiNetwork_Receive;
        manager->unlock = ARNETWORKAL_WifiNetwork_Signal;
        manager->getBandwidth = ARNETWORKAL_WifiNetwork_GetBandwidth;
        manager->bandwidthThread = ARNETWORKAL_WifiNetwork_BandwidthThread;
        manager->maxIds = ARNETWORKAL_MANAGER_WIFI_ID_MAX;
        manager->maxBufferSize = ARNETWORKAL_WIFINETWORK_MAX_DATA_BUFFER_SIZE;
        manager->setOnDisconnectCallback = ARNETWORKAL_WifiNetwork_SetOnDisconnectCallback;
        manager->setRecvBufferSize = ARNETWORKAL_WifiNetwork_SetRecvBufferSize;
        manager->getRecvBufferSize = ARNETWORKAL_WifiNetwork_GetRecvBufferSize;
        manager->setSendBufferSize = ARNETWORKAL_WifiNetwork_SetSendBufferSize;
        manager->getSendBufferSize = ARNETWORKAL_WifiNetwork_GetSendBufferSize;
        manager->setRecvClassSelector = ARNETWORKAL_WifiNetwork_SetRecvClassSelector;
        manager->getRecvClassSelector = ARNETWORKAL_WifiNetwork_GetRecvClassSelector;
        manager->setSendClassSelector = ARNETWORKAL_WifiNetwork_SetSendClassSelector;
        manager->getSendClassSelector = ARNETWORKAL_WifiNetwork_GetSendClassSelector;
    }
    else
    {
        ARNETWORKAL_WifiNetwork_Delete(manager);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CancelWifiNetwork (ARNETWORKAL_Manager_t *manager)
{
    /* -- Cancel the initWifiNetwork -- */

    /* local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_WifiNetwork_Cancel(manager);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseWifiNetwork (ARNETWORKAL_Manager_t *manager)
{
    /* -- Close the Wifi Network -- */

    /* local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_WifiNetwork_Delete(manager);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitBLENetwork (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec, int *notificationIDs, int numberOfNotificationID)
{
    /* local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

#if defined(HAVE_COREBLUETOOTH_COREBLUETOOTH_H)
    /* -- Initialize the BLE Network -- */
    /* check parameters*/
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_BLENetwork_New(manager);
    }

    if (error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_BLENetwork_Connect(manager, deviceManager, device, recvTimeoutSec, notificationIDs, numberOfNotificationID);
    }

    if(error == ARNETWORKAL_OK)
    {
        manager->pushFrame = ARNETWORKAL_BLENetwork_PushFrame;
        manager->popFrame = ARNETWORKAL_BLENetwork_PopFrame;
        manager->send = ARNETWORKAL_BLENetwork_Send;
        manager->receive = ARNETWORKAL_BLENetwork_Receive;
        manager->unlock = ARNETWORKAL_BLENetwork_Unlock;
        manager->getBandwidth = ARNETWORKAL_BLENetwork_GetBandwidth;
        manager->bandwidthThread = ARNETWORKAL_BLENetwork_BandwidthThread;
        manager->maxIds = ARNETWORKAL_MANAGER_BLE_ID_MAX;
        manager->maxBufferSize = ARNETWORKAL_BLENETWORK_MAX_BUFFER_SIZE;
        manager->setOnDisconnectCallback = ARNETWORKAL_BLENetwork_SetOnDisconnectCallback;
    }
    else
    {
        ARNETWORKAL_BLENetwork_Delete(manager);
    }

#else
    error = ARNETWORKAL_ERROR_NETWORK_TYPE;
#endif

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CancelBLENetwork (ARNETWORKAL_Manager_t *manager)
{
    /* Cancel initBLENetwork */
    /* local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

#if defined(HAVE_COREBLUETOOTH_COREBLUETOOTH_H)
    /* check parameters*/
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if( error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_BLENetwork_Cancel(manager);
    }
#else
    error = ARNETWORKAL_ERROR_NETWORK_TYPE;
#endif

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseBLENetwork (ARNETWORKAL_Manager_t *manager)
{
    /** local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

#if defined(HAVE_COREBLUETOOTH_COREBLUETOOTH_H)
    /** -- Close the BLE Network -- */
    if(manager)
    {
        error = ARNETWORKAL_BLENetwork_Delete(manager);
    }
#else
    error = ARNETWORKAL_ERROR_NETWORK_TYPE;
#endif

    return error;
}

void ARNETWORKAL_Manager_Delete (ARNETWORKAL_Manager_t **manager)
{
    /** -- Delete the Manager -- */

    if (manager != NULL)
    {
        if ((*manager) != NULL)
        {
            if ((*manager)->dumpFile != NULL)
            {
                fflush ((*manager)->dumpFile);
                fsync (fileno ((*manager)->dumpFile));
                fclose ((*manager)->dumpFile);
            }
            free (*manager);
            (*manager) = NULL;
        }
    }
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_Unlock (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR err = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        err = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->unlock == NULL)
    {
        err = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        err = manager->unlock(manager);
    }
    return err;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_GetBandwidth (ARNETWORKAL_Manager_t *manager, uint32_t *uploadBw, uint32_t *downloadBw)
{
    eARNETWORKAL_ERROR err = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        err = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->getBandwidth == NULL)
    {
        err = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        err = manager->getBandwidth (manager, uploadBw, downloadBw);
    }
    return err;
}

void* ARNETWORKAL_Manager_BandwidthThread (void *manager)
{
    if (manager == NULL)
    {
        return (void *)0;
    }
    ARNETWORKAL_Manager_t *trueManager = (ARNETWORKAL_Manager_t *)manager;
    if (trueManager->bandwidthThread == NULL)
    {
        return (void *)0;
    }
    return trueManager->bandwidthThread (manager);
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_SetOnDisconnectCallback (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData)
{
    /* -- set the OnDisconnect Callback -- */

    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->setOnDisconnectCallback == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->setOnDisconnectCallback (manager, onDisconnectCallback, customData);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_SetSendBufferSize(ARNETWORKAL_Manager_t *manager, int bufferSize)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->setSendBufferSize == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->setSendBufferSize(manager, bufferSize);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_SetRecvBufferSize(ARNETWORKAL_Manager_t *manager, int bufferSize)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->setRecvBufferSize == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->setRecvBufferSize(manager, bufferSize);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_GetSendBufferSize(ARNETWORKAL_Manager_t *manager, int *bufferSize)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->getSendBufferSize == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->getSendBufferSize(manager, bufferSize);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_GetRecvBufferSize(ARNETWORKAL_Manager_t *manager, int *bufferSize)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->getRecvBufferSize == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->getRecvBufferSize(manager, bufferSize);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_SetSendClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR classSelector)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->setSendClassSelector == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->setSendClassSelector(manager, classSelector);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_SetRecvClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR classSelector)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->setRecvClassSelector == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->setRecvClassSelector(manager, classSelector);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_GetSendClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR *classSelector)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->getSendClassSelector == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->getSendClassSelector(manager, classSelector);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_GetRecvClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR *classSelector)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->getRecvClassSelector == NULL)
    {
        error = ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED;
    }
    else
    {
        error = manager->getRecvClassSelector(manager, classSelector);
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_EnableDataDump(ARNETWORKAL_Manager_t *manager, const char *logDir, const char *name)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    char logPath[512] = "";
    struct stat st;

    if (manager == NULL || logDir == NULL || name == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (manager->dumpFile != NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else if (stat (logDir, &st) < 0)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
        ARSAL_PRINT (ARSAL_PRINT_INFO, ARNETWORKAL_MANAGER_TAG, "[%p] Disabling dump directory '%s' unavailable", manager, logDir);
    }
    else
    {
        snprintf (logPath, sizeof(logPath), "%s/arnetworkal-%s.log", logDir, name);
        ARSAL_Print_DumpRotateFiles(logPath, 4);
        manager->dumpFile = fopen (logPath, "wb");
        if (manager->dumpFile == NULL)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_MANAGER_TAG, "[%p] Unable to create dump file '%s'", manager, logPath);
        }
        else
        {
            ARSAL_PRINT (ARSAL_PRINT_INFO, ARNETWORKAL_MANAGER_TAG, "[%p] Dump enabled in file '%s'", manager, logPath);
        }
    }
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_DumpData(ARNETWORKAL_Manager_t *manager, uint8_t tag, const void *data, size_t size, size_t sizeDump, const struct timespec *ts)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if (manager == NULL || data == NULL || manager->dumpFile == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    else
    {
        ARSAL_Print_DumpData(manager->dumpFile, tag, data, size, sizeDump, ts);
    }
    return error;
}
