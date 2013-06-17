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
#define ARNETWORKAL_BLENETWORK_RECEIVING_BUFFER_SIZE	20

/*****************************************
 *
 *             private header:
 *
 *****************************************/
typedef struct _ARNETWORKAL_BLENetworkObject_
{
	ARNETWORKAL_BLEDeviceManager_t *deviceManager;
	ARNETWORKAL_BLEDevice_t *device;
	uint8_t *buffer;
	uint8_t *currentFrame;
	uint32_t size;
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
    	}
    	else
    	{
    		error = ARNETWORKAL_ERROR_ALLOC;
    	}
    }
    
    /** Allocate sender buffer */
    if(error == ARNETWORKAL_OK)
    {
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer = (uint8_t *)malloc(sizeof(uint8_t) * ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE);
    	if(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer != NULL)
    	{
    		((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size = 0;
    	    ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame = ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer;
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
     	}
    	else
    	{
    		error = ARNETWORKAL_ERROR_ALLOC;
    	}
    }
    
    /** Allocate receiver buffer */
    if(error == ARNETWORKAL_OK)
    {
    	((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer = (uint8_t *)malloc(sizeof(uint8_t) * ARNETWORKAL_BLENETWORK_RECEIVING_BUFFER_SIZE);
    	if(((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer != NULL)
    	{
    		((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size = 0;
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
            
            if(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer)
            {
                free (((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer);
                ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer = NULL;
            }
            
            free (manager->senderObject);
            manager->senderObject = NULL;
        }
        
        if(manager->receiverObject)
        {
            ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->device = NULL;
            ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->deviceManager = NULL;

            if(((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer)
            {
                free (((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer);
                ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer = NULL;
            }
            
            free (manager->receiverObject);
            manager->receiverObject = NULL;
        }
    }
    
    return error;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_pushNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    if((((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size + frame->size) > ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE)
    {
    	result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BUFFER_FULL;
    }
    
    if(result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        uint32_t droneEndianUInt32 = 0;
        
    	/** Add type */
    	memcpy (((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame, &frame->type, sizeof (uint8_t));
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame += sizeof (uint8_t);
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size += sizeof (uint8_t);
        
        /** Add frame type */
    	memcpy (((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame, &frame->id, sizeof (uint8_t));
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame += sizeof (uint8_t);
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size += sizeof (uint8_t);
        
    	/** Add frame sequence number */
    	droneEndianUInt32 = htodl (frame->seq);
    	memcpy (((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame, &droneEndianUInt32, sizeof (uint32_t));
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame += sizeof (uint32_t);
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size += sizeof (uint32_t);
        
    	/** Add frame size */
    	droneEndianUInt32 =  htodl (frame->size);
       	memcpy (((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame, &droneEndianUInt32, sizeof (uint32_t));
       	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame += sizeof (uint32_t);
        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size += sizeof (uint32_t);
        
        /** Add frame data */
        uint32_t dataSize = frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);
        
      	memcpy (((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame, frame->dataPtr, dataSize);
      	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame += dataSize;
        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size += dataSize;
    }
    
    return result;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_popNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    /** -- get a Frame of the receiving buffer -- */
    /** if the receiving buffer not contain enough data for the frame head*/
    if (((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame > ((((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer + ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size) - offsetof (ARNETWORKAL_Frame_t, dataPtr)))
    {
        if (((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame == (((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer + ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size))
        {
            result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BUFFER_EMPTY;
        }
        else
        {
            result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_FRAME;
        }
    }
    
    if (result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        /** Get the frame from the buffer */
        /** get type */
        memcpy (&(frame->type), ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame, sizeof (uint8_t));
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame += sizeof (uint8_t) ;
        
        /** get id */
        memcpy (&(frame->id), ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame, sizeof (uint8_t));
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame += sizeof (uint8_t);
        
        /** get seq */
        memcpy (&(frame->seq), ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame, sizeof (uint32_t));
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame += sizeof (uint32_t);
        /** convert the endianness */
        frame->seq = dtohl (frame->seq);
        
        /** get size */
        memcpy (&(frame->size), ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame, sizeof (uint32_t));
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame += sizeof(uint32_t);
        /** convert the endianness */
        frame->size = dtohl (frame->size);
        
        /** get data address */
        frame->dataPtr = ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame;
        
        /** if the receiving buffer not contain enough data for the full frame */
        if (((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame > ((((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer + ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size) - (frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr))))
        {
         	result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_FRAME;
        }
    }
    
    if (result == ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT)
    {
        /** offset the readingPointer on the next frame */
    	((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame = ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame + frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);
    }
    else
    {
        /** reset the reading pointer to the start of the buffer */
    	((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame = ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer;
    	((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size = 0;
        
        /** reset frame */
        frame->type = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED;
        frame->id = 0;
        frame->seq = 0;
        frame->size = 0;
        frame->dataPtr = NULL;
    }
    
    return result;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_sendingCallback(ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    if(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size != 0)
    {
        // TO DO
    	//ARSAL_Socket_Send(((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->socket, ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer, ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size, 0);
    	((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->size = 0;
	    ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->currentFrame = ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->buffer;
    }
    
    return result;
}

eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_receivingCallback(ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    
    /** -- receiving data present on the socket -- */
    /** local declarations */
    // TO DO
    int size = 0;
    //int size = ARSAL_Socket_Recv (((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->socket, ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer, ARNETWORKAL_BLENETWORK_RECEIVING_BUFFER_SIZE, 0);
    
	if (size > 0)
	{
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size = size;
	}
	else
	{
		result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_NO_DATA_AVAILABLE;
		((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->size = 0;
	}
    
	((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->currentFrame = ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->buffer;
    
    return result;
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec)
{
    eARNETWORKAL_MANAGER_CALLBACK_RETURN result = ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT;
    CBCentralManager *centralManager = (CBCentralManager *)deviceManager;
    CBPeripheral *peripheral = (CBPeripheral *)device;
    
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
        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->deviceManager = deviceManager;
        ((ARNETWORKAL_BLENetworkObject *)manager->senderObject)->device = device;
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->deviceManager = deviceManager;
        ((ARNETWORKAL_BLENetworkObject *)manager->receiverObject)->device = device;
    }
    
    return result;
}



