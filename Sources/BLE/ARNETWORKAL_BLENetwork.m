/**
 * @file ARNETWORKAL_BLENetwork.m
 * @brief BLE network manager allow to send over ble network.
 * @date 06/11/2013
 * @author frederic.dhaeyer@parrot.com
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
#define ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE      20
#define ARNETWORKAL_BLENETWORK_RECEIVING_BUFFER_SIZE    1500

#define ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID @"f"

/*****************************************
 *
 *             private header:
 *
 *****************************************/
@interface ARNETWORKAL_BLENetwork : NSObject

@property (nonatomic, strong) CBPeripheral *peripheral;
@property (nonatomic, strong) CBCentralManager *centralManager;
@property (nonatomic, strong) CBService *recvService;
@property (nonatomic, strong) CBService *sendService;
@property (nonatomic, strong) NSMutableArray *array;
@property (nonatomic) ARNETWORKAL_Manager_t *manager;

- (id)initWithManager:(ARNETWORKAL_Manager_t *)manager;
- (eARNETWORKAL_ERROR)connectWithBTManager:(CBCentralManager *)centralManager peripheral:(CBPeripheral *)peripheral andTimeout:(int)recvTimeoutSec;
- (eARNETWORKAL_ERROR)disconnect;
- (eARNETWORKAL_MANAGER_RETURN)pushFrame:(ARNETWORKAL_Frame_t *)frame;
- (eARNETWORKAL_MANAGER_RETURN)popFrame:(ARNETWORKAL_Frame_t *)frame;
- (eARNETWORKAL_MANAGER_RETURN)receive;


@end

/*****************************************
 *
 *             implementation :
 *
 *****************************************/
@implementation ARNETWORKAL_BLENetwork

- (id)initWithManager:(ARNETWORKAL_Manager_t *)manager
{
    self = [super init];
    if (self)
    {
        _manager = manager;
        _array = [[NSMutableArray alloc] init];
    }
    return self;
}

- (eARNETWORKAL_ERROR)connectWithBTManager:(CBCentralManager *)centralManager peripheral:(CBPeripheral *)peripheral andTimeout:(int)recvTimeoutSec
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    CBService *senderService = nil;
    CBService *receiverService = nil;

    if(peripheral == nil)
    {
        result = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(result == ARNETWORKAL_OK)
    {
        if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) connectToPeripheral:peripheral withCentralManager:centralManager])
        {
            result = ARNETWORKAL_ERROR_BLE_CONNECTION;
        }
    }

    if(result == ARNETWORKAL_OK)
    {
        if([SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) discoverNetworkServices:nil])
        {
            for(int i = 0 ; (i < [[peripheral services] count]) && ((senderService == nil) || (receiverService == nil)) ; i++)
            {
                CBService *service = [[peripheral services] objectAtIndex:i];
                NSLog(@"Service : %@, %04x", [service.UUID representativeString], (unsigned int)service.UUID);
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

                            if((receiverService == nil) && (characteristic.properties & CBCharacteristicPropertyNotify) == CBCharacteristicPropertyNotify)
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

    if(result == ARNETWORKAL_OK)
    {
        NSLog(@"Sender service : %@", [senderService.UUID representativeString]);
        NSLog(@"Receiver service : %@", [receiverService.UUID representativeString]);

        _centralManager = centralManager;
        _peripheral = peripheral;
        _sendService = senderService;
        _recvService = receiverService;

        // Registered notification service for receiver.
        /*for(CBCharacteristic *characteristic in [receiverService characteristics])
        {
            if((characteristic.properties & CBCharacteristicPropertyNotify) == CBCharacteristicPropertyNotify)
            {
                [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setNotificationCharacteristic:characteristic];
            }
        }*/
        
        //temporary just Registered notification for the 4 characteristics need by the DELOS
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setNotificationCharacteristic:[[receiverService characteristics] objectAtIndex: 14]];
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setNotificationCharacteristic:[[receiverService characteristics] objectAtIndex: 15]];
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setNotificationCharacteristic:[[receiverService characteristics] objectAtIndex: 27]];
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setNotificationCharacteristic:[[receiverService characteristics] objectAtIndex: 28]];
        //temporary just Registered notification for the 4 characteristics need by the DELOS
    }

    return result;
}

- (eARNETWORKAL_ERROR)disconnect
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) disconnectPeripheral:_peripheral withCentralManager:_centralManager])
    {
        error = ARNETWORKAL_ERROR_BLE_DISCONNECTION;
    }
    return error;
}

- (eARNETWORKAL_MANAGER_RETURN)pushFrame:(ARNETWORKAL_Frame_t *)frame
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;

    // first uint8_t is frame type and second uint8_t is sequence number
    if((frame->size - offsetof(ARNETWORKAL_Frame_t, dataPtr) + (2 * sizeof(uint8_t))) > ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE)
    {
        result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
    }

    if(result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        NSMutableData *data = [NSMutableData dataWithCapacity:0];

        /** Add frame type */
        [data appendBytes:&(frame->type) length:sizeof(uint8_t)];

        /** Add frame type */
        [data appendBytes:&(frame->seq) length:sizeof(uint8_t)];

        /** Add frame data */
        uint32_t dataSize = frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);
        [data appendBytes:frame->dataPtr length:dataSize];

        /** Get the good characteristic */
        CBCharacteristic *characteristicToSend = nil;

        characteristicToSend = [[_sendService characteristics] objectAtIndex:frame->id];

        if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) writeData:data toCharacteristic:characteristicToSend])
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    return result;
}

- (eARNETWORKAL_MANAGER_RETURN)popFrame:(ARNETWORKAL_Frame_t *)frame
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;
    CBCharacteristic *characteristic = nil;
    /** -- get a Frame of the receiving buffer -- */
    /** if the receiving buffer not contain enough data for the frame head*/
    if([_array count] == 0)
    {
        result = ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY;
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        characteristic = [_array objectAtIndex:0];
        if([[characteristic value] length] == 0)
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    if(result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        uint8_t *currentFrame = (uint8_t *)[[characteristic value] bytes];

        /** get id */
        int frameId = 0;
        if(sscanf([[[characteristic UUID] representativeString] cStringUsingEncoding:NSUTF8StringEncoding], "%04x", &frameId) != 1)
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }

        if(result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {

            /** Get the frame from the buffer */
            /** get id */
            uint8_t frameIdUInt8 = (uint8_t)frameId;
            memcpy(&(frame->id), &frameIdUInt8, sizeof(uint8_t));

            /** get type */
            memcpy(&(frame->type), currentFrame, sizeof(uint8_t));
            currentFrame += sizeof(uint8_t);

            /** get seq */
            memcpy(&(frame->seq), currentFrame, sizeof(uint8_t));
            currentFrame += sizeof(uint8_t);

            /** Get frame size */
            frame->size = [[characteristic value] length] - (2 * sizeof(uint8_t)) + offsetof(ARNETWORKAL_Frame_t, dataPtr);

            /** get data address */
            frame->dataPtr = currentFrame;
        }
    }

    if(result != ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY)
    {
        [_array removeObjectAtIndex:0];
    }

    if (result != ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /** reset frame */
        frame->type = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED;
        frame->id = 0;
        frame->seq = 0;
        frame->size = 0;
        frame->dataPtr = NULL;
    }

    return result;
}

- (eARNETWORKAL_MANAGER_RETURN)receive
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;

    if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) readData:_array])
    {
        result = ARNETWORKAL_MANAGER_RETURN_NO_DATA_AVAILABLE;
    }

    return result;
}


@end

/**
 * C Wrappers
 */

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_New (ARNETWORKAL_Manager_t *manager)
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /** Check parameters */
    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    /** Allocate object */
    if(error == ARNETWORKAL_OK)
    {
        ARNETWORKAL_BLENetwork *object = [[ARNETWORKAL_BLENetwork alloc] initWithManager:manager];
        if (object == nil)
        {
            error = ARNETWORKAL_ERROR_ALLOC;
        }
        else
        {
            manager->senderObject = (__bridge_retained void *)object;
            manager->receiverObject = (__bridge_retained void *)object;
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
        ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
        error = [network disconnect];
        CFRelease(manager->senderObject);
        CFRelease(manager->receiverObject);
        manager->senderObject = NULL;
        manager->receiverObject = NULL;
    }
    return error;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_PushFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
    return [network pushFrame:frame];
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_PopFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->receiverObject;
    return [network popFrame:frame];
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_Send(ARNETWORKAL_Manager_t *manager)
{
    return ARNETWORKAL_MANAGER_RETURN_DEFAULT;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_Receive(ARNETWORKAL_Manager_t *manager)
{
    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->receiverObject;
    return [network receive];
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec)
{
    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
    CBCentralManager *centralManager = (__bridge CBCentralManager *)deviceManager;
    CBPeripheral *peripheral = (__bridge CBPeripheral *)device;
    return [network connectWithBTManager:centralManager peripheral:peripheral andTimeout:recvTimeoutSec];
}
