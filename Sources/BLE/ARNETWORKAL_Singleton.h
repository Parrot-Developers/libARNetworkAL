/**
 *  @file  ARNETWORKAL_BLENetwork.h
 *  @brief private headers of BLE network manager allow to send over ble network.
 *  @date 06/11/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_SINGLETON_PRIVATE_H_
#define _ARNETWORKAL_SINGLETON_PRIVATE_H_

#define SYNTHESIZE_SINGLETON_FOR_CLASS(classname)       \
+ (classname *)shared##classname                        \
{                                                       \
static classname *shared##classname = nil;              \
static dispatch_once_t onceToken;                       \
dispatch_once(&onceToken, ^{                            \
shared##classname = [[classname alloc] init];           \
});                                                     \
return shared##classname;                               \
}

#define DECLARE_SINGLETON_FOR_CLASS(classname)          \
+ (classname *)shared##classname;

#define SINGLETON_FOR_CLASS(classname)                  \
[classname shared##classname]

#endif /** _ARNETWORKAL_SINGLETON_PRIVATE_H_ */
