/**
 *  @file  ARNETWORKAL_BLEManager.h
 *  @brief private headers of BLE manager allow to use Bluetooth low energy api's
 *  @date 06/11/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_BLEMANAGER_PRIVATE_H_
#define _ARNETWORKAL_BLEMANAGER_PRIVATE_H_

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#include <libARSAL/ARSAL_Sem.h>
#include "ARNETWORKAL_Singleton.h"

@interface ARNETWORKAL_BLEManager : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate>
{
    CBPeripheral *activePeripheral;
    ARSAL_Sem_t connectionSem;
    ARSAL_Sem_t disconnectionSem;
}

DECLARE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager);

- (BOOL)connectToPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager;
- (BOOL)disconnectPeripheral:(CBPeripheral *)peripheral withCentralManager:(CBCentralManager *)centralManager;

@end

#endif /** _ARNETWORKAL_BLEMANAGER_PRIVATE_H_ */

