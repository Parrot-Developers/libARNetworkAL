/**
 *  @file ARNETWORKAL_BLENetwork.m
 *  @brief BLE network manager allow to send over ble network.
 *  @date 06/11/2013
 *  @author frederic.dhaeyer@parrot.com
 */

/*****************************************
 *
 *             inport file :
 *
 *****************************************/
#include <libARSAL/ARSAL_Endianness.h>
#include "ARNETWORKAL_Singleton.h"

#import <CoreBluetooth/CoreBluetooth.h>
#import "ARNETWORKAL_BLENetwork.h"
#import "ARNETWORKAL_BLEManager.h"

#define ARNETWORKAL_BLENETWORK_TAG                      "ARNETWORKAL_BLENetwork"
#define ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE		20
#define ARNETWORKAL_BLENETWORK_RECEIVING_BUFFER_SIZE	20 + (sizeof(ARNETWORKAL_Frame_t) - 1)

#define ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID @"f"

/*****************************************
 *
 *             private header:
 *
 *****************************************/
typedef struct _ARNETWORKAL_BLENetworkObject_
{
	ARNETWORKAL_BLEDeviceManager_t *deviceManager;
	ARNETWORKAL_BLEDevice_t *device;
	CBService *service;
} ARNETWORKAL_BLENetworkObject;

/*****************************************
 *
 *             implementation :
 *
 *****************************************/
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_New (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    
    /** Check parameters */
    if(manager == NULL)
    {
    	error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    
    /** Allocate sender object */
    if(error == ARNETWORKAL_OK)
    {
    	manager->senderObject = malloc(sizeof(ARNETWORKAL_BLENetworkObject));
    	if(manager->senderObject != NULL)
    	{
    		((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->deviceManager = NULL;
    		((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->device = NULL;
    		((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->service = nil;
    	}
    	else
    	{
    		error = ARNETWORKAL_ERROR_ALLOC;
    	}
    }
    
    /** Allocate receiver object */
    if(error == ARNETWORKAL_OK)
    {
    	manager->receiverObject = malloc(sizeof(ARNETWORKAL_BLENetworkObject));
    	if(manager->receiverObject != NULL)
    	{
    		((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->deviceManager = NULL;
    		((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->device = NULL;
    		((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->service = nil;
     	}
    	else
    	{
    		error = ARNETWORKAL_ERROR_ALLOC;
    	}
    }
    
    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Delete (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    
    if(manager == NULL)
    {
    	error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    
    if(error == ARNETWORKAL_OK)
    {
        CBCentralManager *centralManager = (CBCentralManager *)(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->deviceManager);
        CBPeripheral *peripheral = (CBPeripheral *)(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->device);
        
        if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) disconnectPeripheral:peripheral withCentralManager:centralManager])
        {
            error = ARNETWORKAL_ERROR_BLE_DISCONNECTION;
        }
    }
    
    if(error == ARNETWORKAL_OK)
    {
        if (manager->senderObject)
        {
            ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->device = NULL;
            ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->deviceManager = NULL;
            ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->service = nil;

            free (manager->senderObject);
            manager->senderObject = NULL;
        }
        
        if(manager->receiverObject)
        {
            ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->device = NULL;
            ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->deviceManager = NULL;
            ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->service = nil;

            free (manager->receiverObject);
            manager->receiverObject = NULL;
        }
    }
    
    return error;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_pushNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    if((frame->size - offsetof(ARNETWORKAL_Frame_t, dataPtr) + sizeof(uint8_t)) > ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE) // uint8_t is frame type
    {
    	result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_FRAME;
    }
    
    if(result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        NSMutableData *data = [NSMutableData dataWithCapacity:0];
        
        /** Add frame type */
        [data appendBytes:&frame->type length:sizeof(uint8_t)];

        /** Add frame data */
        uint32_t dataSize = frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);
        [data appendBytes:frame->dataPtr length:dataSize];
        
        /** Get the good characteristic */
        CBCharacteristic *characteristicToSend = [[(CBService *)(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->service) characteristics] objectAtIndex:frame->id];
        NSLog(@"characteristic : %@, frame id : %d", [characteristicToSend.UUID representativeString], frame->id);
        
        if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) writeData:data toCharacteristic:characteristicToSend])
        {
            result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_FRAME;
        }
    }
    
    return result;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_popNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_NO_DATA_AVAILABLE;
    
    /** reset frame */
    frame->type = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED;
    frame->id = 0;
    frame->seq = 0;
    frame->size = 0;
    frame->dataPtr = NULL;
    
    return result;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_sendingCallback(ARNETWORKAL_Manager_t *manager)
{
    return ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_receivingCallback(ARNETWORKAL_Manager_t *manager)
{
    return ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    CBCentralManager *centralManager = (CBCentralManager *)deviceManager;
    CBPeripheral *peripheral = (CBPeripheral *)device;
    CBService *senderService = nil;
    CBService *receiverService = nil;
    
    if(peripheral == nil)
    {
        result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_PARAMETERS;
    }
    
    if(result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) connectToPeripheral:peripheral withCentralManager:centralManager])
        {
            result = ARNETWORKAL_ERROR_BLE_CONNECTION;
        }
    }
    
    if(result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        if([SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) discoverNetworkServices:nil])
        {
            for(int i = 0 ; (i < [[peripheral services] count]) && ((senderService == nil) || (receiverService == nil)) ; i++)
            {
                CBService *service = [[peripheral services] objectAtIndex:i];
                NSLog(@"Service : %@, %04x", [service.UUID representativeString], service.UUID);
                if([[service.UUID representativeString] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID])
                {
                    if([SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) discoverNetworkCharacteristics:nil forService:service])
                    {
                        if([[service characteristics] count] > 0)
                        {
                            CBCharacteristic *characteristic = [[service characteristics] objectAtIndex:0];
                            if((senderService == nil) && ((characteristic.properties & CBCharacteristicPropertyWriteWithoutResponse) == CBCharacteristicPropertyWriteWithoutResponse))
                            {
                                senderService = service;
                            }

                            if((receiverService == nil) && (characteristic.properties & CBCharacteristicPropertyRead) == CBCharacteristicPropertyRead)
                            {
                                receiverService = service;
                            }
                        }
                    }
                    // NO ELSE
                    // This service is unknown by ARNetworkAL, ignore it
                }
                // NO ELSE
                // It's not a Parrot characteristics, ignore it
            }
        }

        if((senderService == nil) || (receiverService == nil))
        {
            result = ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
        }
    }

    if(result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        NSLog(@"Sender service : %@", [senderService.UUID representativeString]);
        NSLog(@"Receiver service : %@", [receiverService.UUID representativeString]);

        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->deviceManager = deviceManager;
        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->device = device;
        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->service = senderService;
        
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->deviceManager = deviceManager;
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->device = device;
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->service = receiverService;
    }
    
    return result;
}



