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

package com.parrot.arsdk.arnetworkal;

import java.util.HashMap;

/**
 * Java copy of the eARNETWORKAL_ERROR enum
 */
public enum ARNETWORKAL_ERROR_ENUM {
   /** Dummy value for all unknown cases */
    eARNETWORKAL_ERROR_UNKNOWN_ENUM_VALUE (Integer.MIN_VALUE, "Dummy value for all unknown cases"),
   /** No error */
    ARNETWORKAL_OK (0, "No error"),
   /** ARNetworkAL Generic error */
    ARNETWORKAL_ERROR (-1000, "ARNetworkAL Generic error"),
   /** Memory allocation error */
    ARNETWORKAL_ERROR_ALLOC (-999, "Memory allocation error"),
   /** Bad parameters */
    ARNETWORKAL_ERROR_BAD_PARAMETER (-998, "Bad parameters"),
   /** Fifo creation error (details set in errno) */
    ARNETWORKAL_ERROR_FIFO_INIT (-997, "Fifo creation error (details set in errno)"),
   /** The function cannot be run in main thread */
    ARNETWORKAL_ERROR_MAIN_THREAD (-996, "The function cannot be run in main thread"),
   /** Manager generic error */
    ARNETWORKAL_ERROR_MANAGER (-2000, "Manager generic error"),
   /** The current manager does not support this operation */
    ARNETWORKAL_ERROR_MANAGER_OPERATION_NOT_SUPPORTED (-1999, "The current manager does not support this operation"),
   /** Network generic error */
    ARNETWORKAL_ERROR_NETWORK (-3000, "Network generic error"),
   /** Network type, not available for the platform error */
    ARNETWORKAL_ERROR_NETWORK_TYPE (-2999, "Network type, not available for the platform error"),
   /** Wifi generic error */
    ARNETWORKAL_ERROR_WIFI (-4000, "Wifi generic error"),
   /** Wifi socket error during creation */
    ARNETWORKAL_ERROR_WIFI_SOCKET_CREATION (-3999, "Wifi socket error during creation"),
   /** Wifi socket permission denied */
    ARNETWORKAL_ERROR_WIFI_SOCKET_PERMISSION_DENIED (-3998, "Wifi socket permission denied"),
   /** wifi socket error on getopt */
    ARNETWORKAL_ERROR_WIFI_SOCKET_GETOPT (-3997, "wifi socket error on getopt"),
   /** wifi socket error on setopt */
    ARNETWORKAL_ERROR_WIFI_SOCKET_SETOPT (-3996, "wifi socket error on setopt"),
   /** BLE connection generic error */
    ARNETWORKAL_ERROR_BLE_CONNECTION (-5000, "BLE connection generic error"),
   /** BLE is not connected */
    ARNETWORKAL_ERROR_BLE_NOT_CONNECTED (-4999, "BLE is not connected"),
   /** BLE disconnection error */
    ARNETWORKAL_ERROR_BLE_DISCONNECTION (-4998, "BLE disconnection error"),
   /** BLE network services discovering error */
    ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING (-4997, "BLE network services discovering error"),
   /** BLE network characteristics discovering error */
    ARNETWORKAL_ERROR_BLE_CHARACTERISTICS_DISCOVERING (-4996, "BLE network characteristics discovering error"),
   /** BLE network characteristic configuring error */
    ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING (-4995, "BLE network characteristic configuring error"),
   /** BLE stack generic error */
    ARNETWORKAL_ERROR_BLE_STACK (-4994, "BLE stack generic error");

    private final int value;
    private final String comment;
    static HashMap<Integer, ARNETWORKAL_ERROR_ENUM> valuesList;

    ARNETWORKAL_ERROR_ENUM (int value) {
        this.value = value;
        this.comment = null;
    }

    ARNETWORKAL_ERROR_ENUM (int value, String comment) {
        this.value = value;
        this.comment = comment;
    }

    /**
     * Gets the int value of the enum
     * @return int value of the enum
     */
    public int getValue () {
        return value;
    }

    /**
     * Gets the ARNETWORKAL_ERROR_ENUM instance from a C enum value
     * @param value C value of the enum
     * @return The ARNETWORKAL_ERROR_ENUM instance, or null if the C enum value was not valid
     */
    public static ARNETWORKAL_ERROR_ENUM getFromValue (int value) {
        if (null == valuesList) {
            ARNETWORKAL_ERROR_ENUM [] valuesArray = ARNETWORKAL_ERROR_ENUM.values ();
            valuesList = new HashMap<Integer, ARNETWORKAL_ERROR_ENUM> (valuesArray.length);
            for (ARNETWORKAL_ERROR_ENUM entry : valuesArray) {
                valuesList.put (entry.getValue (), entry);
            }
        }
        ARNETWORKAL_ERROR_ENUM retVal = valuesList.get (value);
        if (retVal == null) {
            retVal = eARNETWORKAL_ERROR_UNKNOWN_ENUM_VALUE;
        }
        return retVal;    }

    /**
     * Returns the enum comment as a description string
     * @return The enum description
     */
    public String toString () {
        if (this.comment != null) {
            return this.comment;
        }
        return super.toString ();
    }
}
