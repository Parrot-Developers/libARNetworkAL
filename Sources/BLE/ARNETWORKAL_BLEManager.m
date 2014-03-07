/**
 * @file ARNETWORKAL_BLEManager.m
 * @brief BLE network manager allow to send over ble network.
 * @brief wifi network manager allow to send over wifi network.
 * @date 06/11/2013
 * @author frederic.dhaeyer@parrot.com
 */

/*****************************************
 *
 *             inport file :
 *
 *****************************************/
#import "ARNETWORKAL_BLEManager.h"
#include <libARNetworkAL/ARNETWORKAL_Error.h>

#define ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG (0)
#define ARNETWORKAL_BLEMANAGER_CONNECTION_TIMEOUT_SEC (5)

#pragma mark CBUUID (String Extraction extension)
@implementation CBUUID (StringExtraction)
- (NSString *)representativeString;
{
    NSData *data = [self data];

    NSUInteger bytesToConvert = [data length];
    const unsigned char *uuidBytes = [data bytes];
    NSMutableString *outputString = [NSMutableString stringWithCapacity:16];

    for (NSUInteger currentByteIndex = 0; currentByteIndex < bytesToConvert; currentByteIndex++)
    {
        switch (currentByteIndex)
        {
        case 3:
        case 5:
        case 7:
        case 9:[outputString appendFormat:@"%02x-", uuidBytes[currentByteIndex]]; break;
        default:[outputString appendFormat:@"%02x", uuidBytes[currentByteIndex]];
        }

    }

    return outputString;
}
@end

#pragma mark ARNETWORKAL_BLEManager implementation
@interface ARNETWORKAL_BLEManager ()

@property (nonatomic, assign) eARNETWORKAL_ERROR discoverServicesError;
@property (nonatomic, assign) eARNETWORKAL_ERROR discoverCharacteristicsError;
@property (nonatomic, assign) eARNETWORKAL_ERROR configurationCharacteristicError;

@property (nonatomic, assign) BOOL askDisconnection;
@property (nonatomic, assign) BOOL isDiscoveringServices;
@property (nonatomic, assign) BOOL isDiscoveringCharacteristics;
@property (nonatomic, assign) BOOL isConfiguringCharacteristics;

- (void)ARNETWORKAL_BLEManager_Init;
@end

@implementation ARNETWORKAL_BLEManager
@synthesize discoverServicesError;
@synthesize discoverCharacteristicsError;
@synthesize configurationCharacteristicError;
@synthesize activePeripheral;
@synthesize characteristicsNotifications;
@synthesize delegate = _delegate;

SYNTHESIZE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager, ARNETWORKAL_BLEManager_Init);

// Initializing function to synhtesize singleton
- (void)ARNETWORKAL_BLEManager_Init
{
    self.activePeripheral = nil;
    self.discoverServicesError = ARNETWORKAL_OK;
    self.discoverCharacteristicsError = ARNETWORKAL_OK;
    self.characteristicsNotifications = [NSMutableArray array];
    _askDisconnection = NO;
    _isDiscoveringServices = NO;
    _isDiscoveringCharacteristics = NO;
    _isConfiguringCharacteristics = NO;
    
    ARSAL_Sem_Init(&connectionSem, 0, 0);
    ARSAL_Sem_Init(&disconnectionSem, 0, 0);
    ARSAL_Sem_Init(&discoverServicesSem, 0, 0);
    ARSAL_Sem_Init(&discoverCharacteristicsSem, 0, 0);
    ARSAL_Sem_Init(&readCharacteristicsSem, 0, 0);
    ARSAL_Sem_Init(&configurationSem, 0, 0);

    ARSAL_Mutex_Init(&readCharacteristicMutex);
}

- (eARNETWORKAL_ERROR)discoverNetworkServices:(NSArray *)servicesUUIDs
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    
    @synchronized (self)
    {
        // If there is an active peripheral, disconnecting it
        if(self.activePeripheral != nil)
        {
            _isDiscoveringServices = YES;
            
            self.discoverServicesError = ARNETWORKAL_OK;
            [self.activePeripheral discoverServices:nil];
            ARSAL_Sem_Wait(&discoverServicesSem);
            result = self.discoverServicesError;
            self.discoverServicesError = ARNETWORKAL_OK;
            
            _isDiscoveringServices = NO;
        }
        else
        {
            result = ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
        }
    }
    
    return result;
}

- (eARNETWORKAL_ERROR)setNotificationCharacteristic:(CBCharacteristic *)characteristic
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    @synchronized (self)
    {
        // If there is an active peripheral, disconnecting it
        if(self.activePeripheral != nil)
        {
            _isConfiguringCharacteristics = YES;
            
            self.configurationCharacteristicError = ARNETWORKAL_OK;
            [self.activePeripheral setNotifyValue:YES forCharacteristic:characteristic];
            ARSAL_Sem_Wait(&configurationSem);
            result = self.configurationCharacteristicError;
            self.configurationCharacteristicError = ARNETWORKAL_OK;
            
            _isConfiguringCharacteristics = NO;
        }
        else
        {
            result = ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
        }
    }
    
    return result;
}

- (eARNETWORKAL_ERROR)discoverNetworkCharacteristics:(NSArray *)characteristicsUUIDs forService:(CBService *)service
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    @synchronized (self)
    {
        // If there is an active peripheral, disconnecting it
        if(self.activePeripheral != nil)
        {
            _isDiscoveringCharacteristics = YES;
            
            NSLog(@"Service : %@", [service.UUID representativeString]);
            self.discoverCharacteristicsError = ARNETWORKAL_OK;
            [self.activePeripheral discoverCharacteristics:nil forService:service];
            ARSAL_Sem_Wait(&discoverCharacteristicsSem);
            result = self.discoverCharacteristicsError;
            self.discoverCharacteristicsError = ARNETWORKAL_OK;
            
            _isDiscoveringCharacteristics = NO;
        }
        else
        {
            result = ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
        }
    }
    
    return result;
}

- (eARNETWORKAL_ERROR)connectToPeripheral:(CBPeripheral *)peripheral withCentralManager:(ARSAL_CentralManager *)centralManager
{
    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;
    const struct timespec connectionTimeout = {
        .tv_sec = ARNETWORKAL_BLEMANAGER_CONNECTION_TIMEOUT_SEC,
        .tv_nsec = 0,
    };
    
    @synchronized (self)
    {
        [centralManager addDelegate:self];

        // If there is an active peripheral, disconnecting it
        if(self.activePeripheral != nil)
        {
            [self disconnectPeripheral:self.activePeripheral withCentralManager:centralManager];
        }
        
        // Connection to the new peripheral
        [centralManager connectPeripheral:peripheral options:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBConnectPeripheralOptionNotifyOnDisconnectionKey, nil]];
        
        if(ARSAL_Sem_Timedwait(&connectionSem, &connectionTimeout) != 0)
        {
            /* disconnect timeout */
            [centralManager cancelPeripheralConnection:peripheral];
        }
        
        if (self.activePeripheral != nil)
        {
            self.activePeripheral.delegate = self;
        }
        else
        {
            /* Connection failed */
            result = ARNETWORKAL_ERROR_BLE_CONNECTION;
        }
    }
    
    return result;
}

- (BOOL)disconnectPeripheral:(CBPeripheral *)peripheral withCentralManager:(ARSAL_CentralManager *)centralManager
{
    @synchronized (self)
    {
        if (self.activePeripheral != nil)
        {
            _askDisconnection = YES;
            
            [centralManager cancelPeripheralConnection:activePeripheral];
            
            ARSAL_Sem_Wait(&disconnectionSem);

           [centralManager removeDelegate:self];
        }
    }

    return (self.activePeripheral == nil);
}

- (BOOL)writeData:(NSData *)data toCharacteristic:(CBCharacteristic *)characteristic
{
    BOOL result = NO;
    if((self.activePeripheral != nil) && (characteristic != nil) && (data != nil))
    {
        [self.activePeripheral writeValue:data forCharacteristic:characteristic type:CBCharacteristicWriteWithoutResponse];
        result = YES;
    }

    return result;
}

- (BOOL)readData:(NSMutableArray *)mutableArray
{
    BOOL result = NO;
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d", __FUNCTION__, __LINE__);
#endif
    ARSAL_Sem_Wait(&readCharacteristicsSem);

    if([self.characteristicsNotifications count] > 0)
    {
        ARSAL_Mutex_Lock(&readCharacteristicMutex);
        [mutableArray addObjectsFromArray:self.characteristicsNotifications];
        [self.characteristicsNotifications removeAllObjects];
        ARSAL_Mutex_Unlock(&readCharacteristicMutex);
        result = YES;
    }

    return result;
}

#pragma mark CBCentralManagerDelegate
- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d", __FUNCTION__, __LINE__);
#endif
    
    switch(central.state)
    {
    case CBCentralManagerStatePoweredOn:
        NSLog(@"CBCentralManagerStatePoweredOn");
        break;

    case CBCentralManagerStateResetting:
        NSLog(@"CBCentralManagerStateResetting");
        break;

    case CBCentralManagerStateUnsupported:
        NSLog(@"CBCentralManagerStateUnsupported");
        break;

    case CBCentralManagerStateUnauthorized:
        NSLog(@"CBCentralManagerStateUnauthorized");
        break;

    case CBCentralManagerStatePoweredOff:
        NSLog(@"CBCentralManagerStatePoweredOff");
        break;

    default:
    case CBCentralManagerStateUnknown:
        NSLog(@"CBCentralManagerStateUnknown");
        break;
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
#endif
    
    self.activePeripheral = peripheral;
    
    ARSAL_Sem_Post(&connectionSem);
}

- (void) centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *) error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d : %@", __FUNCTION__, __LINE__, peripheral);
#endif

    ARSAL_Sem_Post(&connectionSem);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d : %@", __FUNCTION__, __LINE__, peripheral);
#endif
    
    if((self.activePeripheral != nil) && (self.activePeripheral == peripheral))
    {
        self.activePeripheral.delegate = nil;
        self.activePeripheral = nil;
        
        ARSAL_Sem_Post(&disconnectionSem);
        
        
        /* if activePeripheral is discovering services */
        if(_isDiscoveringServices)
        {
            self.discoverServicesError = ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
            ARSAL_Sem_Post(&discoverServicesSem);
        }
        
        /* if activePeripheral is discovering Characteristics */
        if(_isDiscoveringCharacteristics)
        {
            self.discoverCharacteristicsError = ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
            ARSAL_Sem_Post(&discoverCharacteristicsSem);
        }
        
        /* if activePeripheral is configuring Characteristics */
        if(_isConfiguringCharacteristics)
        {
            self.configurationCharacteristicError = ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
            ARSAL_Sem_Post(&configurationSem);
        }
        
        /* Notify delegate */
        if(!_askDisconnection)
        {
            if ((_delegate != nil) && ([_delegate respondsToSelector:@selector(onBLEDisconnect)]))
            {
                [_delegate onBLEDisconnect];
            }
        }
        _askDisconnection = NO;
    }
}

#pragma mark CBPeripheralDelegate
- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@", __FUNCTION__, __LINE__, peripheral);
#endif
    
    if (error != nil)
    {
        self.discoverServicesError = ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
    }
    
    ARSAL_Sem_Post(&discoverServicesSem);
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@", __FUNCTION__, __LINE__, peripheral);
#endif
    
    if (error != nil)
    {
        self.discoverCharacteristicsError = ARNETWORKAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING;
    }
    
    ARSAL_Sem_Post(&discoverCharacteristicsSem);
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@ : %@", __FUNCTION__, __LINE__, peripheral, [error localizedDescription]);
#endif
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@ : %@", __FUNCTION__, __LINE__, [characteristic.UUID representativeString], [error localizedDescription]);
#endif

    ARSAL_Mutex_Lock(&readCharacteristicMutex);
    [self.characteristicsNotifications addObject:characteristic];
    ARSAL_Mutex_Unlock(&readCharacteristicMutex);

    ARSAL_Sem_Post(&readCharacteristicsSem);
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@ : %@", __FUNCTION__, __LINE__, [characteristic.UUID representativeString], [error localizedDescription]);
#endif
    
    if (error != nil)
    {
        self.configurationCharacteristicError = ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING;
    }
    
    ARSAL_Sem_Post(&configurationSem);
}

- (void)peripheralDidUpdateName:(CBPeripheral *)peripheral
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
#endif
}

- (void)unlock
{
    /* post all Semaphore to unlock the all the functions */
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
#endif
    
    ARSAL_Sem_Post(&connectionSem);
    ARSAL_Sem_Post(&discoverServicesSem);
    ARSAL_Sem_Post(&discoverCharacteristicsSem);
    ARSAL_Sem_Post(&readCharacteristicsSem);
    ARSAL_Sem_Post(&configurationSem);
    
    /* disconnectionSem is not post because:
     * if the connection is fail, disconnect is not call.
     * if the connection is successful, the BLE callback is always called.
     * the disconnect function is called after the join of the network threads.
     */
}

- (void)reset
{
    @synchronized (self)
    {
        /* reset all Semaphores */
    #if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
        NSLog(@"%s:%d", __FUNCTION__, __LINE__);
    #endif
        
        while (ARSAL_Sem_Trywait(&connectionSem) == 0)
        {
            /* Do nothing*/
        }
        
        while (ARSAL_Sem_Trywait(&discoverServicesSem) == 0)
        {
            /* Do nothing*/
        }
        
        while (ARSAL_Sem_Trywait(&discoverCharacteristicsSem) == 0)
        {
            /* Do nothing*/
        }
        
        while (ARSAL_Sem_Trywait(&disconnectionSem) == 0)
        {
            /* Do nothing*/
        }
        
        while (ARSAL_Sem_Trywait(&configurationSem) == 0)
        {
            /* Do nothing*/
        }
        
        while (ARSAL_Sem_Trywait(&readCharacteristicsSem) == 0)
        {
            /* Do nothing*/
        }
    }
}

@end
