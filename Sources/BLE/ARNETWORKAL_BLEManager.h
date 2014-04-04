/**
 * @file  ARNETWORKAL_BLEManager.h
 * @brief private headers of BLE manager allow to use Bluetooth low energy api's
 * @date 06/11/2013
 * @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_BLEMANAGER_PRIVATE_H_
#define _ARNETWORKAL_BLEMANAGER_PRIVATE_H_

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#include <libARSAL/ARSAL.h>
#include <libARSAL/ARSAL_CentralManager.h>
#include "ARNETWORKAL_Singleton.h"
#include <libARNetworkAL/ARNETWORKAL_Error.h>

@interface CBUUID (StringExtraction)
- (NSString *)representativeString;
@end

@protocol ARNetworkBLEManagerDelegate <NSObject>
@required
- (void)onBLEDisconnect;
@end

@interface ARNETWORKAL_BLEManager : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>
{
    ARSAL_Sem_t connectionSem;
    ARSAL_Sem_t disconnectionSem;
    ARSAL_Sem_t discoverServicesSem;
    ARSAL_Sem_t discoverCharacteristicsSem;
    
    ARSAL_Sem_t readCharacteristicsSem;
    ARSAL_Mutex_t readCharacteristicMutex;

    ARSAL_Sem_t configurationSem;
}

@property (nonatomic, assign) id <ARNetworkBLEManagerDelegate> delegate;
@property (nonatomic, retain) CBPeripheral *activePeripheral;
@property (nonatomic, retain) NSMutableArray *characteristicsNotifications;

DECLARE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager);

- (eARNETWORKAL_ERROR)connectToPeripheral:(CBPeripheral *)peripheral withCentralManager:(ARSAL_CentralManager *)centralManager;
- (void)disconnectPeripheral:(CBPeripheral *)peripheral withCentralManager:(ARSAL_CentralManager *)centralManager;
- (eARNETWORKAL_ERROR)discoverNetworkServices:(NSArray *)servicesUUIDs;
- (eARNETWORKAL_ERROR)discoverNetworkCharacteristics:(NSArray *)characteristicsUUIDs forService:(CBService *)service;
- (eARNETWORKAL_ERROR)setNotificationCharacteristic:(CBCharacteristic *)characteristic;
- (BOOL)writeData:(NSData *)data toCharacteristic:(CBCharacteristic *)characteristic;
- (BOOL)readData:(NSMutableArray *)mutableArray;
- (void)unlock;
- (void)reset;

@end

#endif /** _ARNETWORKAL_BLEMANAGER_PRIVATE_H_ */

