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
#include <libARNetworkAL/ARNETWORKAL_Error.h>

#import <CoreBluetooth/CoreBluetooth.h>
#import "ARNETWORKAL_BLENetwork.h"
#import "ARNETWORKAL_BLEManager.h"

#define ARNETWORKAL_BLENETWORK_TAG                      "ARNETWORKAL_BLENetwork"
#define ARNETWORKAL_BLENETWORK_SENDING_BUFFER_SIZE      ARNETWORKAL_BLENETWORK_MAX_BUFFER_SIZE

#define ARNETWORKAL_BW_PROGRESS_EACH_SEC 1
#define ARNETWORKAL_BW_NB_ELEMS 10

#define ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID @"f"

/*****************************************
 *
 *             private header:
 *
 *****************************************/
@interface ARNETWORKAL_BLENetwork : NSObject <ARNetworkBLEManagerDelegate>

@property (nonatomic, strong) CBPeripheral *peripheral;
@property (nonatomic, strong) ARSAL_CentralManager *centralManager;
@property (nonatomic, strong) CBService *recvService;
@property (nonatomic, strong) CBService *sendService;
@property (nonatomic, strong) NSMutableArray *array;
@property (nonatomic) ARNETWORKAL_Manager_t *manager;

@property (nonatomic) uint32_t *bw_elementsUp;
@property (nonatomic) uint32_t *bw_elementsDown;
@property (nonatomic) int bw_index;
@property (nonatomic) uint32_t bw_currentUp;
@property (nonatomic) uint32_t bw_currentDown;
@property (nonatomic) ARSAL_Sem_t bw_sem;
@property (nonatomic) ARSAL_Sem_t bw_threadRunning;

@property (nonatomic) ARNETWORKAL_Manager_OnDisconnect_t onDisconnect;
@property (nonatomic) void* onDisconnectCustomData;

- (id)initWithManager:(ARNETWORKAL_Manager_t *)manager;
- (eARNETWORKAL_ERROR)connectWithBTManager:(ARSAL_CentralManager *)centralManager peripheral:(CBPeripheral *)peripheral andTimeout:(int)recvTimeoutSec;
- (eARNETWORKAL_ERROR)disconnect;
- (eARNETWORKAL_MANAGER_RETURN)pushFrame:(ARNETWORKAL_Frame_t *)frame;
- (eARNETWORKAL_MANAGER_RETURN)popFrame:(ARNETWORKAL_Frame_t *)frame;
- (eARNETWORKAL_MANAGER_RETURN)receive;

- (eARNETWORKAL_ERROR)getUpload:(uint32_t *)upload andDownloadBandwidth:(uint32_t *)download;
- (void)bw_thread;

- (void)onBLEDisconnect;

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
        _bw_elementsUp = malloc (ARNETWORKAL_BW_NB_ELEMS * sizeof (uint32_t));
        _bw_elementsDown = malloc (ARNETWORKAL_BW_NB_ELEMS * sizeof (uint32_t));
        ARSAL_Sem_Init (&_bw_threadRunning, 0, 1);
    }
    return self;
}

- (void)dealloc
{
    free (_bw_elementsUp);
    free (_bw_elementsDown);
    ARSAL_Sem_Destroy (&_bw_threadRunning);
}

- (eARNETWORKAL_ERROR)connectWithBTManager:(ARSAL_CentralManager *)centralManager peripheral:(CBPeripheral *)peripheral andTimeout:(int)recvTimeoutSec
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    eARNETWORKAL_ERROR discoverCharacteristicsResult = ARNETWORKAL_OK;
    eARNETWORKAL_ERROR setNotifCharacteristicResult = ARNETWORKAL_OK;
    CBService *senderService = nil;
    CBService *receiverService = nil;

    if(peripheral == nil)
    {
        result = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(result == ARNETWORKAL_OK)
    {
        result = [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) connectToPeripheral:peripheral withCentralManager:centralManager];
    }
    
    if(result == ARNETWORKAL_OK)
    {
        result = [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) discoverNetworkServices:nil];
    }

    if(result == ARNETWORKAL_OK)
    {
        for(int i = 0 ; (i < [[peripheral services] count]) && ((senderService == nil) || (receiverService == nil)) && (result == ARNETWORKAL_OK) ; i++)
        {
            CBService *service = [[peripheral services] objectAtIndex:i];
            NSLog(@"Service : %@, %04x", [service.UUID representativeString], (unsigned int)service.UUID);
            if([[service.UUID representativeString] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID])
            {
                discoverCharacteristicsResult = [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) discoverNetworkCharacteristics:nil forService:service];
                switch (discoverCharacteristicsResult)
                {
                    case ARNETWORKAL_OK:
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
                        break;
                        
                    case ARNETWORKAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING:
                        /* This service is unknown by ARNetworkAL, ignore it */
                        break;
                        
                    case ARNETWORKAL_ERROR_BLE_NOT_CONNECTED:
                        /* the peripheral is disconnected */
                        result = ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;
                    
                    default:
                        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_BLENETWORK_TAG, "error %d unexpected : %s", discoverCharacteristicsResult, ARNETWORKAL_Error_ToString(discoverCharacteristicsResult));
                        result = ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;
                }
            }
            // NO ELSE
            // It's not a Parrot characteristics, ignore it
        }
    }
    
    if((result == ARNETWORKAL_OK) && ((senderService == nil) || (receiverService == nil)))
    {
        result = ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
    }

    if(result == ARNETWORKAL_OK)
    {
        NSLog(@"Sender service : %@", [senderService.UUID representativeString]);
        NSLog(@"Receiver service : %@", [receiverService.UUID representativeString]);

        _bw_index = 0;
        _bw_currentUp = 0;
        _bw_currentDown = 0;
        for (int i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
        {
            _bw_elementsUp[i] = 0;
            _bw_elementsDown[i] = 0;
        }
        ARSAL_Sem_Init (&_bw_sem, 0, 0);

        _centralManager = centralManager;
        _peripheral = peripheral;
        _sendService = senderService;
        _recvService = receiverService;
        
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setDelegate:self];

        // Registered notification service for receiver.
        for(CBCharacteristic *characteristic in [receiverService characteristics])
        {
            if((characteristic.properties & CBCharacteristicPropertyNotify) == CBCharacteristicPropertyNotify)
            {
                setNotifCharacteristicResult = [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setNotificationCharacteristic:characteristic];
                
                switch (setNotifCharacteristicResult)
                {
                    case ARNETWORKAL_OK:
                        /* notification successfully set */
                        /* do nothing */
                        break;
                        
                    case ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING:
                        /* This service is unknown by ARNetworkAL*/
                        /* do nothing */
                        break;
                        
                    case ARNETWORKAL_ERROR_BLE_NOT_CONNECTED:
                        /* the peripheral is disconnected */
                        result = ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;
                        
                    default:
                        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_BLENETWORK_TAG, "error %d unexpected : %s", setNotifCharacteristicResult, ARNETWORKAL_Error_ToString(setNotifCharacteristicResult));
                        break;
                }
            }
        }
    }
    
    return result;
}

- (eARNETWORKAL_ERROR)disconnect
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    
    ARSAL_Sem_Post (&_bw_sem);
    ARSAL_Sem_Wait(&_bw_threadRunning);
    ARSAL_Sem_Destroy (&_bw_sem);

    if(![SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) disconnectPeripheral:_peripheral withCentralManager:_centralManager])
    {
        error = ARNETWORKAL_ERROR_BLE_DISCONNECTION;
    }
    
    [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) setDelegate:nil];
    
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

        /** Add frame seq number */
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
        else
        {
            _bw_currentUp += data.length;
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

            _bw_currentDown += [[characteristic value] length];
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

- (eARNETWORKAL_ERROR)getUpload:(uint32_t *)upload andDownloadBandwidth:(uint32_t *)download
{
    int i;

    uint32_t up = 0, down = 0;
    for (i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
    {
        up += _bw_elementsUp[i];
        down += _bw_elementsDown[i];
    }
    up /= (ARNETWORKAL_BW_NB_ELEMS * ARNETWORKAL_BW_PROGRESS_EACH_SEC);
    down /= (ARNETWORKAL_BW_NB_ELEMS * ARNETWORKAL_BW_PROGRESS_EACH_SEC);
    if (upload != NULL)
    {
        *upload = up;
    }
    if (download != NULL)
    {
        *download = down;
    }
    return ARNETWORKAL_OK;
}

- (void)bw_thread
{
    
    if (ARSAL_Sem_Trywait(&_bw_threadRunning) == 0)
    {
        const struct timespec timeout = {
            .tv_sec = ARNETWORKAL_BW_PROGRESS_EACH_SEC,
            .tv_nsec = 0,
        };

        int waitRes = ARSAL_Sem_Timedwait (&_bw_sem, &timeout);
        int loopCondition = (waitRes == -1) && (errno == ETIMEDOUT);
        while (loopCondition)
        {
            _bw_index++;
            _bw_index %= ARNETWORKAL_BW_NB_ELEMS;
            _bw_elementsUp[_bw_index] = _bw_currentUp;
            _bw_elementsDown[_bw_index] = _bw_currentDown;
            _bw_currentUp = 0;
            _bw_currentDown = 0;

            // Update loop condition
            waitRes = ARSAL_Sem_Timedwait (&_bw_sem, &timeout);
            loopCondition = (waitRes == -1) && (errno == ETIMEDOUT);
        }
        
        ARSAL_Sem_Post(&_bw_threadRunning);
    }
    /* No Else: the thread is already running or stopped*/
}


- (void)onBLEDisconnect
{
    NSLog(@"%s:%d", __FUNCTION__, __LINE__);
    
    if(self.onDisconnect != NULL)
    {
        self.onDisconnect(_manager, self.onDisconnectCustomData);
    }
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

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Cancel (ARNETWORKAL_Manager_t *manager)
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
        
        /* reset the BLEManager for a new use */
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) reset];
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
        
        /* reset the BLEManager for a new use */
        [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) reset];
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

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Unlock(ARNETWORKAL_Manager_t *manager)
{
    /* -- BLE unlock all functions locked -- */
    [SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) unlock];
    return ARNETWORKAL_OK;
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec)
{
    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
    ARSAL_CentralManager *centralManager = (__bridge ARSAL_CentralManager *)deviceManager;
    CBPeripheral *peripheral = (__bridge CBPeripheral *)device;
    return [network connectWithBTManager:centralManager peripheral:peripheral andTimeout:recvTimeoutSec];
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_GetBandwidth (ARNETWORKAL_Manager_t *manager, uint32_t *uploadBw, uint32_t *downloadBw)
{
    if (manager == NULL)
    {
        return ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
    return [network getUpload:uploadBw andDownloadBandwidth:downloadBw];
}

void *ARNETWORKAL_BLENetwork_BandwidthThread (void *param)
{
    if (param != NULL)
    {
        ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *)param;
        ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
        [network bw_thread];
    }
    return (void *)0;
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_SetOnDisconnectCallback (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData)
{
    /* -- set the OnDisconnect Callback -- */
    
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    ARNETWORKAL_BLENetwork *network = nil;
    
    if ((manager == NULL) || (onDisconnectCallback == NULL))
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }
    /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */
    
    if (error == ARNETWORKAL_OK)
    {
        network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
        if (network == nil)
        {
            error = ARNETWORKAL_ERROR_BAD_PARAMETER;
        }
        /* No Else: the checking parameters sets error to ARNETWORKAL_ERROR_BAD_PARAMETER and stop the processing */
    }
    /* No else: skipped by an error */ 
    
    if (error == ARNETWORKAL_OK)
    {
        network.onDisconnect = onDisconnectCallback;
        network.onDisconnectCustomData = customData;
    }
    /* No else: skipped by an error */ 
    
    return error;
}
