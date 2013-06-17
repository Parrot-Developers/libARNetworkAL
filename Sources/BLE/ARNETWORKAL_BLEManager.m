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

@interface ARNETWORKAL_BLEManager (private)
- (void)ARNETWORKAL_BLEManager_Init;
@end

@implementation ARNETWORKAL_BLEManager
SYNTHESIZE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager, ARNETWORKAL_BLEManager_Init);

// Initializing function to synhtesize singleton
- (void)ARNETWORKAL_BLEManager_Init
{
    activePeripheral = nil;
    ARSAL_Sem_Init(&connectionSem, 0, 0);
    ARSAL_Sem_Init(&disconnectionSem, 0, 0);
}

- (BOOL)connectToPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager
{
    id <CBCentralManagerDelegate> previousDelegate = centralManager.delegate;
    [centralManager setDelegate:self];

    // If there is an active peripheral, disconnecting it
    if(activePeripheral != nil)
    {
        [self disconnectPeripheral:activePeripheral withCentralManager:centralManager];
        ARSAL_Sem_Wait(&disconnectionSem);
    }
    
    // Connection to the new peripheral
    activePeripheral = nil;
    [centralManager connectPeripheral:peripheral options:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBConnectPeripheralOptionNotifyOnDisconnectionKey, nil]];
    ARSAL_Sem_Wait(&connectionSem);
    activePeripheral = peripheral;
    activePeripheral.delegate = self;
    [centralManager setDelegate:previousDelegate];
    
    return (activePeripheral != nil);
}

- (BOOL)disconnectPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager
{
    id <CBCentralManagerDelegate> previousDelegate = centralManager.delegate;
    [centralManager setDelegate:self];
    
    if(activePeripheral != nil)
    {
        [centralManager cancelPeripheralConnection:activePeripheral];
        ARSAL_Sem_Wait(&disconnectionSem);
        activePeripheral.delegate = nil;
        activePeripheral = nil;    
    }
    
    [centralManager setDelegate:previousDelegate];

    return (activePeripheral == nil);
}

#pragma mark CBCentralManagerDelegate
- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    NSLog(@"%s:%d", __FUNCTION__, __LINE__);
    
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
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
    ARSAL_Sem_Post(&connectionSem);
}

- (void) centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *) error
{
    NSLog(@"%s:%d : %@", __FUNCTION__, __LINE__, peripheral);
    ARSAL_Sem_Post(&connectionSem);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
    NSLog(@"%s:%d : %@", __FUNCTION__, __LINE__, peripheral);
    ARSAL_Sem_Post(&disconnectionSem);
}

#pragma mark CBPeripheralDelegate
- (void)peripheralDidUpdateName:(CBPeripheral *)peripheral
{
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
}

@end
