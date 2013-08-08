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
#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Mutex.h>
#include "ARNETWORKAL_Singleton.h"

@interface CBUUID (StringExtraction)
- (NSString *)representativeString;
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

@property (nonatomic, retain) NSError *discoverServicesError;
@property (nonatomic, retain) NSError *discoverCharacteristicsError;
@property (nonatomic, retain) NSError *configurationCharacteristicError;
@property (nonatomic, retain) CBPeripheral *activePeripheral;
@property (nonatomic, retain) NSMutableArray *characteristicsNotifications;

DECLARE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager);

- (BOOL)connectToPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager;
- (BOOL)disconnectPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager;
- (BOOL)discoverNetworkServices:(NSArray *)servicesUUIDs;
- (BOOL)discoverNetworkCharacteristics:(NSArray *)characteristicsUUIDs forService:(CBService *)service;
- (BOOL)setNotificationCharacteristic:(CBCharacteristic *)characteristic;
- (BOOL)writeData:(NSData *)data toCharacteristic:(CBCharacteristic *)characteristic;
- (BOOL)readData:(NSMutableArray *)mutableArray;

@end

#endif /** _ARNETWORKAL_BLEMANAGER_PRIVATE_H_ */

