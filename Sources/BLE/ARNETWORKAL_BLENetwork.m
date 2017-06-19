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
#include <libARSAL/ARSAL_Singleton.h>
#include <libARSAL/ARSAL_Error.h>
#include <libARNetworkAL/ARNETWORKAL_Error.h>

#import <CoreBluetooth/CoreBluetooth.h>
#import "ARNETWORKAL_BLENetwork.h"
#import <libARSAL/ARSAL_BLEManager.h>

#define ARNETWORKAL_BLENETWORK_TAG                      "ARNETWORKAL_BLENetwork"

#define ARNETWORKAL_BW_PROGRESS_EACH_SEC 1
#define ARNETWORKAL_BW_NB_ELEMS 10


#define kARNETWORKAL_BLENetwork_NotificationRecv    @"ARNETWORKAL_BLENetwork_NotificationRecv"
#define ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID @"f"
#define ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_21 @"fd23"
#define ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_51 @"fd53"


/*****************************************
 *
 *             private header:
 *
 *****************************************/

@interface ARNETWORKAL_BLENetwork : NSObject <ARSALBLEManagerDelegate>

@property (nonatomic, strong) CBPeripheral *peripheral;
@property (nonatomic, strong) ARSAL_CentralManager *centralManager;
@property (nonatomic, strong) CBService *recvService;
@property (nonatomic, strong) CBService *sendService;
@property (nonatomic, strong) NSMutableArray *recvArray;
@property (nonatomic, strong) NSMutableArray *recvNotificationCharacteristicArray;
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
- (eARNETWORKAL_ERROR)connectWithBTManager:(ARSAL_CentralManager *)centralManager peripheral:(CBPeripheral *)peripheral andTimeout:(int)recvTimeoutSec andNotificationIDArray:(NSArray *) notificationIDArray;
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
        _recvArray = [[NSMutableArray alloc] init];
        _recvNotificationCharacteristicArray = [[NSMutableArray alloc] init];
        _bw_elementsUp = malloc (ARNETWORKAL_BW_NB_ELEMS * sizeof (uint32_t));
        _bw_elementsDown = malloc (ARNETWORKAL_BW_NB_ELEMS * sizeof (uint32_t));
        ARSAL_Sem_Init (&_bw_threadRunning, 0, 0);
        ARSAL_Sem_Init (&_bw_sem, 0, 0);
    }
    return self;
}

- (void)dealloc
{
    free (_bw_elementsUp);
    free (_bw_elementsDown);
    ARSAL_Sem_Destroy (&_bw_sem);
    ARSAL_Sem_Destroy (&_bw_threadRunning);
}

- (eARNETWORKAL_ERROR)connectWithBTManager:(ARSAL_CentralManager *)centralManager peripheral:(CBPeripheral *)peripheral andTimeout:(int)recvTimeoutSec andNotificationIDArray:(NSArray *) notificationIDArray
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    eARSAL_ERROR discoverCharacteristicsResult = ARSAL_OK;
    eARSAL_ERROR setNotifCharacteristicResult = ARSAL_OK;
    eARSAL_ERROR resultSAL = ARSAL_OK;
    CBService *senderService = nil;
    CBService *receiverService = nil;

    if(peripheral == nil)
    {
        result = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if([NSThread isMainThread])
    {
        result = ARNETWORKAL_ERROR_MAIN_THREAD;
    }

    if(result == ARNETWORKAL_OK)
    {
        [_recvNotificationCharacteristicArray removeAllObjects];
    }

    if(result == ARNETWORKAL_OK)
    {
        resultSAL = [SINGLETON_FOR_CLASS(ARSAL_BLEManager) connectToPeripheral:peripheral withCentralManager:centralManager];
        if (resultSAL != ARSAL_OK)
        {
            result = ARNETWORKAL_ERROR_BLE_CONNECTION;
        }
    }

    //discover all services
    if(result == ARNETWORKAL_OK)
    {
        resultSAL = [SINGLETON_FOR_CLASS(ARSAL_BLEManager) discoverNetworkServices:nil];
        if (resultSAL != ARSAL_OK)
        {
            result = ARNETWORKAL_ERROR_BLE_DISCONNECTION;
        }
    }

    //discover all characteristics related to known services
    for(NSUInteger i = 0 ; (i < [[peripheral services] count]) && (result == ARNETWORKAL_OK) ; i++)
    {
        CBService *service = [[peripheral services] objectAtIndex:i];
        //NSLog(@"Service : %@, %04x", [service.UUID shortUUID], (unsigned int)service.UUID);
        if([[service.UUID shortUUID] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID])
        {
            resultSAL = [SINGLETON_FOR_CLASS(ARSAL_BLEManager) discoverNetworkCharacteristics:nil forService:service];
            if (resultSAL != ARSAL_OK)
            {
                result = ARNETWORKAL_ERROR_BLE_DISCONNECTION;
            }
        }
    }

    if(result == ARNETWORKAL_OK)
    {
        for(NSUInteger i = 0 ; (i < [[peripheral services] count]) && ((senderService == nil) || (receiverService == nil)) && (result == ARNETWORKAL_OK) ; i++)
        {
            CBService *service = [[peripheral services] objectAtIndex:i];
            NSLog(@"Service : %@, %@, %04x", [service.UUID representativeString], [service.UUID shortUUID], (unsigned int)service.UUID);
            if([[service.UUID shortUUID] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID])
            {
                //done before
                //discoverCharacteristicsResult = [SINGLETON_FOR_CLASS(ARSAL_BLEManager) discoverNetworkCharacteristics:nil forService:service];
                discoverCharacteristicsResult = ARSAL_OK;
                switch (discoverCharacteristicsResult)
                {
                    case ARSAL_OK:
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

                    case ARSAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING:
                        /* This service is unknown by ARNetworkAL, ignore it */
                        result = ARNETWORKAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING;
                        break;

                    case ARSAL_ERROR_BLE_NOT_CONNECTED:
                        /* the peripheral is disconnected */
                        result = ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;

                    default:
                        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_BLENETWORK_TAG, "error %d unexpected : %s", discoverCharacteristicsResult, ARSAL_Error_ToString(discoverCharacteristicsResult));
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
        NSLog(@"Sender service : %@", [senderService.UUID shortUUID]);
        NSLog(@"Receiver service : %@", [receiverService.UUID shortUUID]);

        _bw_index = 0;
        _bw_currentUp = 0;
        _bw_currentDown = 0;
        for (int i = 0; i < ARNETWORKAL_BW_NB_ELEMS; i++)
        {
            _bw_elementsUp[i] = 0;
            _bw_elementsDown[i] = 0;
        }
        ARSAL_Sem_Post(&_bw_threadRunning);

        _centralManager = centralManager;
        _peripheral = peripheral;
        _sendService = senderService;
        _recvService = receiverService;

        [SINGLETON_FOR_CLASS(ARSAL_BLEManager) setDelegate:self];

        NSArray *notificationCharateristics = nil;
        if (notificationIDArray != nil)
        {
            NSMutableArray *notificationCharateristicsTmp = [[NSMutableArray alloc] init];
            /* Add the charateristics to be notified */
            for(NSNumber *notificationID in notificationIDArray)
            {
                [notificationCharateristicsTmp addObject: [[receiverService characteristics] objectAtIndex:[notificationID intValue]]];
            }

            notificationCharateristics = notificationCharateristicsTmp;
        }
        else
        {
            notificationCharateristics = [receiverService characteristics];
        }
        // Registered notification service for receiver.
        for(CBCharacteristic *characteristic in notificationCharateristics)
        {
            if((characteristic.properties & CBCharacteristicPropertyNotify) == CBCharacteristicPropertyNotify)
            {
                setNotifCharacteristicResult = [SINGLETON_FOR_CLASS(ARSAL_BLEManager) setNotificationCharacteristic:characteristic];

                switch (setNotifCharacteristicResult)
                {
                    case ARSAL_OK:
                        /* notification successfully set */
                        [_recvNotificationCharacteristicArray addObject:characteristic];
                        break;

                    case ARSAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING:
                        /* This service is unknown by ARNetworkAL*/
                        /* do nothing */
                        result = ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING;
                        break;

                    case ARSAL_ERROR_BLE_NOT_CONNECTED:
                        /* the peripheral is disconnected */
                        result = ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;

                    default:
                        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_BLENETWORK_TAG, "error %d unexpected : %s", setNotifCharacteristicResult, ARSAL_Error_ToString(setNotifCharacteristicResult));
                        result = ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;
                }
            }
        }

        [SINGLETON_FOR_CLASS(ARSAL_BLEManager) registerNotificationCharacteristics:_recvNotificationCharacteristicArray toKey:kARNETWORKAL_BLENetwork_NotificationRecv];
    }

    for(NSUInteger i = 0 ; (i < [[peripheral services] count]) && (result == ARNETWORKAL_OK) ; i++)
    {
        CBService *service = [[peripheral services] objectAtIndex:i];
        //NSLog(@"Service : %@, %04x", [service.UUID shortUUID], (unsigned int)service.UUID);
        if([[service.UUID shortUUID] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID])
        {
            for(NSUInteger j = 0 ; (j < [[service characteristics] count]) && (result == ARNETWORKAL_OK) ; j++)
            {
                CBCharacteristic *characteristic = [[service characteristics] objectAtIndex:j];
                //NSLog(@"Characteristic : %@, %04x", [characteristic.UUID shortUUID], (unsigned int)characteristic.UUID);
                if(((characteristic.properties & CBCharacteristicPropertyNotify) == CBCharacteristicPropertyNotify)
                    && ([[characteristic.UUID shortUUID] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_21]
                    || [[characteristic.UUID shortUUID] hasPrefix:ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_51]))
                {
                    resultSAL = [SINGLETON_FOR_CLASS(ARSAL_BLEManager) setNotificationCharacteristic:characteristic];
                    NSLog(@"==REGISTERED Characteristic : %@, %04x", [characteristic.UUID shortUUID], (unsigned int)characteristic.UUID);
                    if (resultSAL != ARSAL_OK)
                    {
                        result = ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING;
                    }
                }
            }
        }
    }

    return result;
}

- (eARNETWORKAL_ERROR)disconnect
{
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    @synchronized (self)
    {
        if (_peripheral != nil)
        {
            if ([NSThread isMainThread])
            {
                error = ARNETWORKAL_ERROR_MAIN_THREAD;
            }

            if (error == ARNETWORKAL_OK)
            {
                [SINGLETON_FOR_CLASS(ARSAL_BLEManager) setDelegate:nil];

                ARSAL_Sem_Post (&_bw_sem);
                ARSAL_Sem_Wait(&_bw_threadRunning);

                [SINGLETON_FOR_CLASS(ARSAL_BLEManager) disconnectPeripheral:_peripheral withCentralManager:_centralManager];
                _peripheral = nil;
            }
        }
    }
    return error;
}

- (eARNETWORKAL_MANAGER_RETURN)pushFrame:(ARNETWORKAL_Frame_t *)frame
{
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;

    // first uint8_t is frame type and second uint8_t is sequence number
    if((frame->size - offsetof(ARNETWORKAL_Frame_t, dataPtr) + ARNETWORKAL_BLENETWORK_HEADER_SIZE ) > ARNETWORKAL_BLENETWORK_MEDIA_MTU)
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

        if(![SINGLETON_FOR_CLASS(ARSAL_BLEManager) writeData:data toCharacteristic:characteristicToSend])
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
    ARSALBLEManagerNotificationData *notificationData = nil;
    /** -- get a Frame of the receiving buffer -- */
    /** if the receiving buffer not contain enough data for the frame head*/
    if([_recvArray count] == 0)
    {
        result = ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY;
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        notificationData = [_recvArray objectAtIndex:0];
        if([[notificationData value] length] == 0)
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    if(result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        uint8_t *currentFrame = (uint8_t *)[[notificationData value] bytes];

        /** get id */
        int frameId = 0;
        //if(sscanf([[[[notificationData characteristic] UUID] representativeString] cStringUsingEncoding:NSUTF8StringEncoding], "%04x", &frameId) != 1)
        if(sscanf([[[[notificationData characteristic] UUID] shortUUID] cStringUsingEncoding:NSUTF8StringEncoding], "%04x", &frameId) != 1)
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
            frame->size = [[notificationData value] length] - (2 * sizeof(uint8_t)) + offsetof(ARNETWORKAL_Frame_t, dataPtr);

            /** get data address */
            frame->dataPtr = currentFrame;

            _bw_currentDown += [[notificationData value] length];
        }
    }

    if(result != ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY)
    {
        [_recvArray removeObjectAtIndex:0];
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

    //if(![SINGLETON_FOR_CLASS(ARSAL_BLEManager) readData:_array])

    if([SINGLETON_FOR_CLASS(ARSAL_BLEManager) readNotificationData:_recvArray maxCount:INT_MAX timeout:nil toKey:kARNETWORKAL_BLENetwork_NotificationRecv] != ARSAL_OK)
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
        [SINGLETON_FOR_CLASS(ARSAL_BLEManager) reset];
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

        if (network != nil)
        {
            network.onDisconnect = NULL;
            network.onDisconnectCustomData = NULL;
            error = [network disconnect];

            //check error
            if (manager->senderObject != nil)
            {
                CFRelease(manager->senderObject);
                manager->senderObject = nil;
            }

            if (manager->receiverObject != nil)
            {
                CFRelease(manager->receiverObject);
                manager->receiverObject = nil;
            }

            /* reset the BLEManager for a new use */
            [SINGLETON_FOR_CLASS(ARSAL_BLEManager) reset];
        }
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
    [SINGLETON_FOR_CLASS(ARSAL_BLEManager) unlock];
    return ARNETWORKAL_OK;
}

eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec, int *notificationIDs, int numberOfNotificationID)
{
    ARNETWORKAL_BLENetwork *network = (__bridge ARNETWORKAL_BLENetwork *)manager->senderObject;
    ARSAL_CentralManager *centralManager = (__bridge ARSAL_CentralManager *)deviceManager;
    CBPeripheral *peripheral = (__bridge CBPeripheral *)device;

    NSMutableArray *bleNotificationIDs = nil;
    if(notificationIDs != NULL)
    {
        bleNotificationIDs = [[NSMutableArray alloc]init];
        for(int i = 0 ; i < numberOfNotificationID ; i++)
        {
            [bleNotificationIDs addObject: [NSNumber numberWithInt: notificationIDs[i]]];
        }
    }

    return [network connectWithBTManager:centralManager peripheral:peripheral andTimeout:recvTimeoutSec andNotificationIDArray: bleNotificationIDs];
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
