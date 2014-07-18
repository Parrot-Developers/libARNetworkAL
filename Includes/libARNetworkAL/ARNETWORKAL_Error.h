/**
 * @file ARNETWORKAL_Error.h
 * @brief libARNetworkAL errors known.
 * @date 04/29/2013
 * @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_ERROR_H_
#define _ARNETWORKAL_ERROR_H_

/**
 * @brief libARNetworkAL errors known.
 */
typedef enum
{
    ARNETWORKAL_OK = 0,                              /**< No error */
    ARNETWORKAL_ERROR = -1000,                       /**< ARNetworkAL Generic error */
    ARNETWORKAL_ERROR_ALLOC,                         /**< Memory allocation error */
    ARNETWORKAL_ERROR_BAD_PARAMETER,                 /**< Bad parameters */
    ARNETWORKAL_ERROR_FIFO_INIT,                     /**< Fifo creation error (details set in errno) */

    ARNETWORKAL_ERROR_MANAGER = -2000,               /**< Manager generic error */
    ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED, /**< The current manager does not support this operation */

    ARNETWORKAL_ERROR_NETWORK = -3000,               /**< Network generic error */
    ARNETWORKAL_ERROR_NETWORK_TYPE,                  /**< Network type, not available for the platform error */

    ARNETWORKAL_ERROR_WIFI = -4000,                  /**< Wifi generic error */
    ARNETWORKAL_ERROR_WIFI_SOCKET_CREATION,          /**< Wifi socket error during creation */
    ARNETWORKAL_ERROR_WIFI_SOCKET_PERMISSION_DENIED, /**< Wifi socket permission denied */

    ARNETWORKAL_ERROR_BLE_CONNECTION = -5000,        /**< BLE connection generic error */
    ARNETWORKAL_ERROR_BLE_NOT_CONNECTED,             /**< BLE is not connected */
    ARNETWORKAL_ERROR_BLE_DISCONNECTION,             /**< BLE disconnection error */
    ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING,      /**< BLE network services discovering error */
    ARNETWORKAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING,  /**< BLE network characteristics discovering error */
    ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING,   /**< BLE network characteristic configuring error */
    ARNETWORKAL_ERROR_BLE_STACK,                        /**< BLE stack generic error */

} eARNETWORKAL_ERROR;

/**
 * @brief Gets the error string associated with an eARNETWORKAL_ERROR
 * @param error The error to describe
 * @return A static string describing the error
 *
 * @note User should NEVER try to modify a returned string
 */
char* ARNETWORKAL_Error_ToString (eARNETWORKAL_ERROR error);

#endif /* _ARNETWORKAL_ERROR_H_ */
