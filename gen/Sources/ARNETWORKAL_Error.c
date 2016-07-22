/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/*
 * GENERATED FILE
 *  Do not modify this file, it will be erased during the next configure run 
 */

/**
 * @file ARNETWORKAL_Error.c
 * @brief ToString function for eARNETWORKAL_ERROR enum
 */

#include <libARNetworkAL/ARNETWORKAL_Error.h>

const char* ARNETWORKAL_Error_ToString (eARNETWORKAL_ERROR error)
{
    switch (error)
    {
    case ARNETWORKAL_OK:
        return "No error";
        break;
    case ARNETWORKAL_ERROR:
        return "ARNetworkAL Generic error";
        break;
    case ARNETWORKAL_ERROR_ALLOC:
        return "Memory allocation error";
        break;
    case ARNETWORKAL_ERROR_BAD_PARAMETER:
        return "Bad parameters";
        break;
    case ARNETWORKAL_ERROR_FIFO_INIT:
        return "Fifo creation error (details set in errno)";
        break;
    case ARNETWORKAL_ERROR_MAIN_THREAD:
        return "The function cannot be run in main thread";
        break;
    case ARNETWORKAL_ERROR_MANAGER:
        return "Manager generic error";
        break;
    case ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED:
        return "The current manager does not support this operation";
        break;
    case ARNETWORKAL_ERROR_NETWORK:
        return "Network generic error";
        break;
    case ARNETWORKAL_ERROR_NETWORK_TYPE:
        return "Network type, not available for the platform error";
        break;
    case ARNETWORKAL_ERROR_WIFI:
        return "Wifi generic error";
        break;
    case ARNETWORKAL_ERROR_WIFI_SOCKET_CREATION:
        return "Wifi socket error during creation";
        break;
    case ARNETWORKAL_ERROR_WIFI_SOCKET_PERMISSION_DENIED:
        return "Wifi socket permission denied";
        break;
    case ARNETWORKAL_ERROR_WIFI_SOCKET_GETOPT:
        return "wifi socket error on getopt";
        break;
    case ARNETWORKAL_ERROR_WIFI_SOCKET_SETOPT:
        return "wifi socket error on setopt";
        break;
    case ARNETWORKAL_ERROR_BLE_CONNECTION:
        return "BLE connection generic error";
        break;
    case ARNETWORKAL_ERROR_BLE_NOT_CONNECTED:
        return "BLE is not connected";
        break;
    case ARNETWORKAL_ERROR_BLE_DISCONNECTION:
        return "BLE disconnection error";
        break;
    case ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING:
        return "BLE network services discovering error";
        break;
    case ARNETWORKAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING:
        return "BLE network characteristics discovering error";
        break;
    case ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING:
        return "BLE network characteristic configuring error";
        break;
    case ARNETWORKAL_ERROR_BLE_STACK:
        return "BLE stack generic error";
        break;
    default:
        break;
    }
    return "Unknown value";
}
