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
#include "ARNETWORKAL_Singleton.h"

@interface ARNETWORKAL_BLEManager : NSObject
DECLARE_SINGLETON_FOR_CLASS(ARNETWORKAL_BLEManager);


@end

#endif /** _ARNETWORKAL_BLEMANAGER_PRIVATE_H_ */

