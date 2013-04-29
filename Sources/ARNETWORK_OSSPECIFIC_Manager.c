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

#define ARNETWORK_OSSPECIFIC_MANAGER_TAG "ARNETWORK_OSSPECIFIC_Manager"

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
    manager = malloc (sizeof(ARNETWORK_OSSPECIFIC_Manager_t));
    if (manager != NULL)
    {
        /** Initialize to default values */
    	manager->pushNextFrameCallback = (ARNETWORK_OSSPECIFIC_Manager_PushNextFrame_Callback_t)NULL;
    	manager->popNextFrameCallback = (ARNETWORK_OSSPECIFIC_Manager_PopNextFrame_Callback_t)NULL;
    	manager->sendingCallback = (ARNETWORK_OSSPECIFIC_Manager_Sending_Callback_t)NULL;
    	manager->receivingCallback = (ARNETWORK_OSSPECIFIC_Manager_Receiving_Callback_t)NULL;
    	manager->type = ARNETWORK_OSSPECIFIC_MANAGER_NETWORK_TYPE_DEFAULT;
    	manager->receiverObject = (void *)NULL;
    	manager->senderObject = (void *)NULL;
    }
    else
    {
    	localError = ARNETWORK_OSSPECIFIC_ERROR_ALLOC;
    }

    /** delete the Manager if an error occurred */
    if (localError != ARNETWORK_OSSPECIFIC_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_OSSPECIFIC_MANAGER_TAG, "error: %d occurred \n", localError);
        ARNETWORK_OSSPECIFIC_Manager_Delete (&manager);
    }

    /** return the error */
    if (error != NULL)
    {
        *error = localError;
    }

    return manager;
}

void ARNETWORK_OSSPECIFIC_Manager_InitWiFiNetwork (ARNETWORK_OSSPECIFIC_Manager_t *manager, const char *addr, int sendingPort, int receivingPort, int recvTimeoutSec)
{
    /** -- Intitialize the Wifi Network -- */

    /** local declarations */
    eARNETWORK_OSSPECIFIC_ERROR error = ARNETWORK_OSSPECIFIC_OK;

    /** check paratemters*/
    if ((manager == NULL) || (addr == NULL))
    {
        error = ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORK_OSSPECIFIC_OK)
    {
    	error = ARNETWORK_OSSPECIFIC_WifiNetwork_New(manager);
    }

    if (error == ARNETWORK_OSSPECIFIC_OK)
    {
        error = ARNETWORK_OSSPECIFIC_WifiNetwork_Connect (manager, addr, sendingPort);
    }

    if (error == ARNETWORK_OSSPECIFIC_OK)
    {
        error = ARNETWORK_OSSPECIFIC_WifiNetwork_Bind (manager, receivingPort, recvTimeoutSec);
    }

    return error;
}

void ARNETWORK_OSSPECIFIC_Manager_Delete (ARNETWORK_OSSPECIFIC_Manager_t **manager)
{
    /** -- Delete the Manager -- */

    /** local declarations */
    ARNETWORK_OSSPECIFIC_Manager_t *localManager = NULL;

    if (localManager)
    {
    	localManager = *manager;

    	ARNETWORK_OSSPECIFIC_WifiNetwork_Delete(localManager);

        if (localManager)
        {
            free (localManager);
            localManager = NULL;
        }

        *manager = NULL;
    }
}
