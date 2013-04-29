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

#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Manager.h>
#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Error.h>
#include "ARNETWORK_OSSPECIFIC_Manager.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORK_OSSPECIFICMANAGER_TAG "ARNETWORK_OSSPECIFIC_Manager"

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

ARNETWORK_OSSPECIFIC_Manager_t* ARNETWORK_OSSPECIFIC_Manager_New (eARNETWORK_OSSPECIFIC_ERROR *error)
{
    /** -- Create a new Manager -- */

    /** local declarations */
    ARNETWORK_OSSPECIFIC_Manager_t *manager = NULL;
    eARNETWORK_OSSPECIFIC_ERROR localError = ARNETWORK_OSSPECIFIC_OK;
    /** Create the Manager */
    manager = malloc (sizeof (ARNETWORK_OSSPECIFIC_Manager_t));
    if (manager != NULL)
    {
        /** Initialize to default values */

    }
    else
    {
    	localError = ARNETWORK_OSSPECIFIC_ERROR_ALLOC;
    }

    /** return the error */
    if (error != NULL)
    {
        *error = localError;
    }

    return manager;
}

void ARNETWORK_OSSPECIFIC_Manager_Delete (ARNETWORK_OSSPECIFIC_Manager_t **manager)
{
    /** -- Delete the Manager -- */

    /** local declarations */
    ARNETWORK_OSSPECIFIC_Manager_t *localManager = NULL;

    if (localManager)
    {
    	localManager = *manager;

        if (localManager)
        {
            free (localManager);
            localManager = NULL;
        }

        *manager = NULL;
    }
}

//eARNETWORK_ERROR ARNETWORK_Manager_SocketsInit (ARNETWORK_Manager_t *managerPtr, const char *addr, int sendingPort, int receivingPort, int recvTimeoutSec)
//{
//    /** -- initialize UDP sockets of sending and receiving the data. -- */
//
//    /** local declarations */
//    eARNETWORK_ERROR error = ARNETWORK_OK;
//
//    /** check paratemters*/
//    if (managerPtr == NULL || addr== NULL)
//    {
//        error = ARNETWORK_ERROR_BAD_PARAMETER;
//    }
//
//    if (error == ARNETWORK_OK)
//    {
//        error = ARNETWORK_Sender_Connect (managerPtr->senderPtr, addr, sendingPort);
//    }
//
//    if (error == ARNETWORK_OK)
//    {
//        error = ARNETWORK_Receiver_Bind (managerPtr->receiverPtr, receivingPort, recvTimeoutSec);
//    }
//
//    return error;
//}
