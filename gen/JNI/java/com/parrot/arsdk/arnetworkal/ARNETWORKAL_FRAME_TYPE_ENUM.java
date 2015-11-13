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
 * Java copy of the eARNETWORKAL_FRAME_TYPE enum
 */
public enum ARNETWORKAL_FRAME_TYPE_ENUM {
   /** Dummy value for all unknown cases */
    eARNETWORKAL_FRAME_TYPE_UNKNOWN_ENUM_VALUE (Integer.MIN_VALUE, "Dummy value for all unknown cases"),
   /** Unknown type. Don't use */
    ARNETWORKAL_FRAME_TYPE_UNINITIALIZED (0, "Unknown type. Don't use"),
   /** Acknowledgment type. Internal use only */
    ARNETWORKAL_FRAME_TYPE_ACK (1, "Acknowledgment type. Internal use only"),
   /** Data type. Main type for data that does not require an acknowledge */
    ARNETWORKAL_FRAME_TYPE_DATA (2, "Data type. Main type for data that does not require an acknowledge"),
   /** Low latency data type. Should only be used when needed */
    ARNETWORKAL_FRAME_TYPE_DATA_LOW_LATENCY (3, "Low latency data type. Should only be used when needed"),
   /** Data that should have an acknowledge type. This type can have a long latency */
    ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK (4, "Data that should have an acknowledge type. This type can have a long latency"),
   /** Unused, iterator maximum value */
    ARNETWORKAL_FRAME_TYPE_MAX (5, "Unused, iterator maximum value");

    private final int value;
    private final String comment;
    static HashMap<Integer, ARNETWORKAL_FRAME_TYPE_ENUM> valuesList;

    ARNETWORKAL_FRAME_TYPE_ENUM (int value) {
        this.value = value;
        this.comment = null;
    }

    ARNETWORKAL_FRAME_TYPE_ENUM (int value, String comment) {
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
     * Gets the ARNETWORKAL_FRAME_TYPE_ENUM instance from a C enum value
     * @param value C value of the enum
     * @return The ARNETWORKAL_FRAME_TYPE_ENUM instance, or null if the C enum value was not valid
     */
    public static ARNETWORKAL_FRAME_TYPE_ENUM getFromValue (int value) {
        if (null == valuesList) {
            ARNETWORKAL_FRAME_TYPE_ENUM [] valuesArray = ARNETWORKAL_FRAME_TYPE_ENUM.values ();
            valuesList = new HashMap<Integer, ARNETWORKAL_FRAME_TYPE_ENUM> (valuesArray.length);
            for (ARNETWORKAL_FRAME_TYPE_ENUM entry : valuesArray) {
                valuesList.put (entry.getValue (), entry);
            }
        }
        ARNETWORKAL_FRAME_TYPE_ENUM retVal = valuesList.get (value);
        if (retVal == null) {
            retVal = eARNETWORKAL_FRAME_TYPE_UNKNOWN_ENUM_VALUE;
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
