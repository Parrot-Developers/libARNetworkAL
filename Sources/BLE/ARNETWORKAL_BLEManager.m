/**
 *  @file ARNETWORKAL_BLEManager.m
 *  @brief BLE network manager allow to send over ble network.
 *  @brief wifi network manager allow to send over wifi network.
 *  @date 06/11/2013
 *  @author frederic.dhaeyer@parrot.com
 */

/*****************************************
 *
 *             inport file :
 *
 *****************************************/
#import "ARNETWORKAL_BLEManager.h"

#define ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG (0)

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

@interface ARNETWORKAL_BLEManager (private)
- (void)ARNETWORKAL_BLEManager_Init;
@end

@implementation ARNETWORKAL_BLEManager
@synthesize discoverServicesError;
@synthesize discoverCharacteristicsError;
@synthesize activePeripheral;

SYNTHESIZE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager, ARNETWORKAL_BLEManager_Init);

// Initializing function to synhtesize singleton
- (void)ARNETWORKAL_BLEManager_Init
{
    self.activePeripheral = nil;
    self.discoverServicesError = nil;
    self.discoverCharacteristicsError = nil;
    
    ARSAL_Sem_Init(&connectionSem, 0, 0);
    ARSAL_Sem_Init(&disconnectionSem, 0, 0);
    ARSAL_Sem_Init(&discoverServicesSem, 0, 0);
    ARSAL_Sem_Init(&discoverCharacteristicsSem, 0, 0);
}

- (BOOL)discoverNetworkServices:(NSArray *)servicesUUIDs
{
    BOOL result = NO;
    // If there is an active peripheral, disconnecting it
    if(self.activePeripheral != nil)
    {
        self.discoverServicesError = nil;
        [self.activePeripheral discoverServices:nil];
        ARSAL_Sem_Wait(&discoverServicesSem);
        result = (self.discoverServicesError == nil);
        self.discoverServicesError = nil;
    }
    
    return result;
}

- (BOOL)discoverNetworkCharacteristics:(NSArray *)characteristicsUUIDs forService:(CBService *)service
{
    BOOL result = NO;
    // If there is an active peripheral, disconnecting it
    if(self.activePeripheral != nil)
    {
        NSLog(@"Service : %@", [service.UUID representativeString]);
        self.discoverCharacteristicsError = nil;
        [self.activePeripheral discoverCharacteristics:nil forService:service];
        ARSAL_Sem_Wait(&discoverCharacteristicsSem);
        result = (self.discoverCharacteristicsError == nil);
        self.discoverCharacteristicsError = nil;
        for(CBCharacteristic *characteristic in service.characteristics)
        {
            NSLog(@"Characteristic : %@", [characteristic.UUID representativeString]);
        }
    }
    
    return result;
}

- (BOOL)connectToPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager
{
    id <CBCentralManagerDelegate> previousDelegate = centralManager.delegate;
    [centralManager setDelegate:self];

    // If there is an active peripheral, disconnecting it
    if(self.activePeripheral != nil)
    {
        [self disconnectPeripheral:self.activePeripheral withCentralManager:centralManager];
        ARSAL_Sem_Wait(&disconnectionSem);
    }
    
    // Connection to the new peripheral
    self.activePeripheral = nil;
    [centralManager connectPeripheral:peripheral options:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBConnectPeripheralOptionNotifyOnDisconnectionKey, nil]];
    ARSAL_Sem_Wait(&connectionSem);
    self.activePeripheral = peripheral;
    self.activePeripheral.delegate = self;
    [centralManager setDelegate:previousDelegate];
    
    return (self.activePeripheral != nil);
}

- (BOOL)disconnectPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager
{
    id <CBCentralManagerDelegate> previousDelegate = centralManager.delegate;
    [centralManager setDelegate:self];
    
    if(self.activePeripheral != nil)
    {
        [centralManager cancelPeripheralConnection:activePeripheral];
        ARSAL_Sem_Wait(&disconnectionSem);
        self.activePeripheral.delegate = nil;
        self.activePeripheral = nil;
    }
    
    [centralManager setDelegate:previousDelegate];

    return (self.activePeripheral == nil);
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
    ARSAL_Sem_Post(&disconnectionSem);
}

#pragma mark CBPeripheralDelegate
- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@", __FUNCTION__, __LINE__, peripheral);
#endif
    self.discoverServicesError = error;
    ARSAL_Sem_Post(&discoverServicesSem);
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d - %@", __FUNCTION__, __LINE__, peripheral);
#endif
    self.discoverCharacteristicsError = error;
    ARSAL_Sem_Post(&discoverCharacteristicsSem);
}

- (void)peripheralDidUpdateName:(CBPeripheral *)peripheral
{
#if ARNETWORKAL_BLEMANAGER_ENABLE_DEBUG
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
#endif
}

@end
