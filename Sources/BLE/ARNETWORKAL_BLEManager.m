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
    centralManager = [[CBCentralManager alloc] initWithDelegate:SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager) queue:nil];
    
    activePeripheral = nil;
    ARSAL_Mutex_Init(&connectionMutex);
    ARSAL_Cond_Init(&connectionCond);
}

- (BOOL)connectToPeripheral:(CBPeripheral *)peripheral
{
    [centralManager connectPeripheral:peripheral options:nil];
    ARSAL_Mutex_Lock(&connectionMutex);
    ARSAL_Cond_Wait(&connectionCond, &connectionMutex);
    ARSAL_Mutex_Unlock(&connectionMutex);
    
    return (activePeripheral != nil);
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
    activePeripheral = peripheral;
    activePeripheral.delegate = self;

    ARSAL_Mutex_Lock(&connectionMutex);
    ARSAL_Cond_Signal(&connectionCond);
    ARSAL_Mutex_Unlock(&connectionMutex);
}

- (void) centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(NSError *) error
{
    NSLog(@"%s:%d : %@", __FUNCTION__, __LINE__, peripheral);
    activePeripheral = nil;
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
    NSLog(@"%s:%d : %@", __FUNCTION__, __LINE__, peripheral);
    activePeripheral = nil;

    ARSAL_Mutex_Lock(&connectionMutex);
    ARSAL_Cond_Signal(&connectionCond);
    ARSAL_Mutex_Unlock(&connectionMutex);
}

#pragma mark CBPeripheralDelegate
- (void)peripheralDidUpdateName:(CBPeripheral *)peripheral
{
    NSLog(@"%s:%d -> %@", __FUNCTION__, __LINE__, peripheral);
}

@end
