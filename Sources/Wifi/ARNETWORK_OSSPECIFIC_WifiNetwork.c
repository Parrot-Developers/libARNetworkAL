/**
 *  @file ARNETWORK_OSSPECIFIC_Manager.c
 *  @brief network manager allow to send over network.
 *  @date 25/04/2013
 *  @author frederic.dhaeyer@parrot.com
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

#include <libARSAL/ARSAL_Socket.h>
#include <libARSAL/ARSAL_Print.h>

#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Manager.h>
#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Error.h>
#include "ARNETWORK_OSSPECIFIC_Manager.h"
#include "Wifi/ARNETWORK_OSSPECIFIC_WifiNetwork.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORK_OSSPECIFIC_MANAGER_TAG "ARNETWORK_OSSPECIFIC_WifiNetwork"

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
eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_New (ARNETWORK_OSSPECIFIC_Manager_t *manager)
{
    eARNETWORK_OSSPECIFIC_ERROR error = ARNETWORK_OSSPECIFIC_OK;
    int connectError;

    /** Check parameters */
    if(manager == NULL)
    {
    	error = ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER;
    }

    /** Allocate sender object */
   if(error == ARNETWORK_OSSPECIFIC_OK)
    {
    	manager->senderObject = malloc(sizeof(int));
    	if(manager->senderObject != NULL)
    	{
    		*(int *)manager->senderObject = -1;
    	}
    	else
    	{
    		error = ARNETWORK_OSSPECIFIC_ERROR_ALLOC;
    	}
    }

   /** Allocate receiver object */
    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
    	manager->receiverObject = malloc(sizeof(int));
    	if(manager->receiverObject != NULL)
    	{
    		*((int *)manager->receiverObject) = -1;
     	}
    	else
    	{
    		error = ARNETWORK_OSSPECIFIC_ERROR_ALLOC;
    	}
    }

    return error;
}

eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_Delete (ARNETWORK_OSSPECIFIC_Manager_t *manager)
{
    eARNETWORK_OSSPECIFIC_ERROR error = ARNETWORK_OSSPECIFIC_OK;

    if(manager == NULL)
    {
    	error = ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
		if (manager->senderObject)
		{
			free (manager->senderObject);
			manager->senderObject = NULL;
		}

		if(manager->senderObject)
		{
			free (manager->receiverObject);
			manager->receiverObject = NULL;
		}
    }
}

eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_Connect (ARNETWORK_OSSPECIFIC_Manager_t *manager, const char *addr, int port)
{
    /** -- Connect the socket in UDP to a port of an address -- */
    /** local declarations */
    struct sockaddr_in sendSin;
    eARNETWORK_OSSPECIFIC_ERROR error = ARNETWORK_OSSPECIFIC_OK;
    int connectError;

    /** Check parameters */
    if((manager == NULL) || (manager->senderObject == NULL))
    {
    	error = ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER;
    }

    /** Create sender Object */
    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
        *(int *)manager->senderObject = ARSAL_Socket_Create (AF_INET, SOCK_DGRAM, 0);
        if(manager->senderObject < 0)
        {
        	error = ARNETWORK_OSSPECIFIC_ERROR_SOCKET_CREATION;
        }
    }

    /** Initialize socket */
    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
    	sendSin.sin_addr.s_addr = inet_addr (addr);
        sendSin.sin_family = AF_INET;
        sendSin.sin_port = htons (port);

        connectError = ARSAL_Socket_Connect (*(int *)manager->senderObject, (struct sockaddr*) &sendSin, sizeof (sendSin));

        if (connectError != 0)
        {
            switch (errno)
            {
            case EACCES:
                error = ARNETWORK_OSSPECIFIC_ERROR_SOCKET_PERMISSION_DENIED;
                break;

            default:
                error = ARNETWORK_OSSPECIFIC_ERROR_SOCKET;
                break;
            }
        }
    }

    return error;
}

eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_Bind (ARNETWORK_OSSPECIFIC_Manager_t *manager, unsigned short port, int timeoutSec)
{
    /** -- receiving data present on the socket -- */

    /** local declarations */
    struct timeval timeout;
    struct sockaddr_in recvSin;
    eARNETWORK_OSSPECIFIC_ERROR error = ARNETWORK_OSSPECIFIC_OK;
    int errorBind = 0;

    /** Check parameters */
    if((manager == NULL) || (manager->receiverObject == NULL))
    {
    	error = ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER;
    }

    /** Create sender Object */
    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
        *(int *)manager->receiverObject = ARSAL_Socket_Create (AF_INET, SOCK_DGRAM, 0);
        if(manager->receiverObject < 0)
        {
        	error = ARNETWORK_OSSPECIFIC_ERROR_SOCKET_CREATION;
        }
    }

    /** socket initialization */
    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
		recvSin.sin_addr.s_addr = htonl (INADDR_ANY);
		recvSin.sin_family = AF_INET;
		recvSin.sin_port = htons (port);

		/** set the socket timeout */
		timeout.tv_sec = timeoutSec;
		timeout.tv_usec = 0;
		ARSAL_Socket_Setsockopt (*(int *)manager->receiverObject, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (timeout));

		errorBind = ARSAL_Socket_Bind (*(int *)manager->receiverObject, (struct sockaddr*)&recvSin, sizeof (recvSin));

		if (errorBind !=0)
		{
			switch (errno)
			{
			case EACCES:
				error = ARNETWORK_OSSPECIFIC_ERROR_SOCKET_PERMISSION_DENIED;
				break;

			default:
				error = ARNETWORK_OSSPECIFIC_ERROR_SOCKET;
				break;
			}
		}
    }

    return error;
}
