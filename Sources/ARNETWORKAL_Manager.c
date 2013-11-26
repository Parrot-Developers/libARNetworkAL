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
    manager = malloc (sizeof(ARNETWORKAL_Manager_t));
    if (manager != NULL)
    {
        /** Initialize to default values */
        manager->pushFrame = (ARNETWORKAL_Manager_PushFrame_t)NULL;
        manager->popFrame = (ARNETWORKAL_Manager_PopFrame_t)NULL;
        manager->send = (ARNETWORKAL_Manager_Send_t)NULL;
        manager->receive = (ARNETWORKAL_Manager_Receive_t)NULL;
        manager->unlock = (ARNETWORKAL_Manager_Unlock_t)NULL;
        manager->receiverObject = (void *)NULL;
        manager->senderObject = (void *)NULL;
        manager->maxIds = ARNETWORKAL_MANAGER_DEFAULT_ID_MAX;
    }
    else
    {
        localError = ARNETWORKAL_ERROR_ALLOC;
    }

    /** delete the Manager if an error occurred */
    if (localError != ARNETWORKAL_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_MANAGER_TAG, "error: %d occurred \n", localError);
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
        manager->maxIds = ARNETWORKAL_MANAGER_WIFI_ID_MAX;
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_SignalWifiNetwork(ARNETWORKAL_Manager_t *manager)
{
    /** -- Signals the Wifi Network to stop blocking on sockets -- */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_WifiNetwork_Signal(manager);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseWifiNetwork (ARNETWORKAL_Manager_t *manager)
{
    /** -- Close the Wifi Network -- */

    /** local declarations */
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

eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitBLENetwork (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec)
{
    /** local declarations */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

#if defined(HAVE_COREBLUETOOTH_COREBLUETOOTH_H)
    /** -- Initialize the BLE Network -- */
    /** check parameters*/
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
        error = ARNETWORKAL_BLENetwork_Connect(manager, deviceManager, device, recvTimeoutSec);
    }

    if(error == ARNETWORKAL_OK)
    {
        manager->pushFrame = ARNETWORKAL_BLENetwork_PushFrame;
        manager->popFrame = ARNETWORKAL_BLENetwork_PopFrame;
        manager->send = ARNETWORKAL_BLENetwork_Send;
        manager->receive = ARNETWORKAL_BLENetwork_Receive;
        //manager->unlock = //TODO set
        manager->maxIds = ARNETWORKAL_MANAGER_BLE_ID_MAX;
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

    /** local declarations */
    ARNETWORKAL_Manager_t *localManager = NULL;

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
