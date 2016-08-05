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
package com.parrot.arsdk.arnetworkal;

import com.parrot.arsdk.arsal.ARNativeData;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.pm.PackageManager;

import com.parrot.arsdk.arsal.ARSAL_SOCKET_CLASS_SELECTOR_ENUM;
import com.parrot.arsdk.arsal.ARSALPrint;
import com.parrot.arsdk.arsal.ARSALBLEManager;
import com.parrot.mux.Mux;

/**
 * Network manager allow to send and receive on network.
 */
public class ARNetworkALManager
{
    private static final String TAG = "ARNetworkALManager";
    
    public static int ARNETWORKAL_MANAGER_DEFAULT_ID_MAX = 0;  /**< Default ID Max */
    public static int ARNETWORKAL_MANAGER_WIFI_ID_MAX = 0;  /**< ID Max for WifiNetwork */
    public static int ARNETWORKAL_MANAGER_BLE_ID_MAX = 0; /**< ID Max for BLENetwork */

    private static native int nativeGetDefineDefaultIdMAX ();
    private static native int nativeGetDefineWifiIdMAX ();
    private static native int nativeGetDefineBleIdMAX ();

    private native long nativeNew();
    private native int nativeDelete(long jManager);
    private native int nativeInitWifiNetwork(long jManager, String jaddr, int sendingPort, int receivingPort, int recvTimeoutSec);
    private native int nativeUnlock(long jManager);
    private native int nativeCloseWifiNetwork(long jManager);
    
    private native int nativeInitBLENetwork(long jManager, Object jContext, BluetoothDevice jdevice, int recvTimeoutSec, int[] notificationIDArray);
    private native int nativeCloseBLENetwork(long jManager);
    private native int nativeCancelBLENetwork(long jManager);

    private native int nativeInitMuxNetwork(long jManager, long jMux);
    private native int nativeCloseMuxNetwork(long jManager);

    private native int nativeSetSendBufferSize(long jManager, int bufferSize);
    private native int nativeSetRecvBufferSize(long jManager, int bufferSize);
    private native int nativeSetSendClassSelector(long jManager, int cs);
    private native int nativeSetRecvClassSelector(long jManager, int cs);

    private native int nativeEnableDataDump(long jManager, String jLogDir, String jName);
    private native int nativeDumpData(long jManager, byte tag, long nativeData, int datasize, int dumpsize);

    private long m_managerPtr;
    private boolean m_initOk;

    static
    {
        ARNETWORKAL_MANAGER_DEFAULT_ID_MAX = nativeGetDefineDefaultIdMAX ();
        ARNETWORKAL_MANAGER_WIFI_ID_MAX = nativeGetDefineWifiIdMAX ();
        ARNETWORKAL_MANAGER_BLE_ID_MAX = nativeGetDefineBleIdMAX ();
    }

    /**
     * Constructor
     */
    public ARNetworkALManager()
    {
        m_initOk = false;
        m_managerPtr = nativeNew();

        if( m_managerPtr != 0 )
        {
            m_initOk = true;
        }
    }

    /**
     * Dispose
     */
    public void dispose()
    {
        if(m_initOk == true)
        {
            nativeDelete(m_managerPtr);
            m_managerPtr = 0;
            m_initOk = false;
        }
    }

    /**
     * Destructor
     */
    public void finalize () throws Throwable
    {
        try
        {
            dispose ();
        }
        finally
        {
            super.finalize ();
        }
    }

    /**
     * Get the pointer C on the network manager
     * @return  Pointer C on the network manager
     */
    public long getManager ()
    {
        return m_managerPtr;
    }

    /**
     * Get is the Manager is correctly initialized and if it is usable
     * @return true is the Manager is usable
     */
    public boolean isCorrectlyInitialized ()
    {
        return m_initOk;
    }

    /**
     * Initialize Wifi network to send and receive data
     */
    public ARNETWORKAL_ERROR_ENUM initWifiNetwork(String addr, int sendingPort, int receivingPort, int recvTimeoutSec)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        
        if(addr != null)
        {
            int intError = nativeInitWifiNetwork(m_managerPtr, addr, sendingPort, receivingPort, recvTimeoutSec);
            error =  ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        }

        return error;
    }

    /**
     * Unlock the manager
     */
    public ARNETWORKAL_ERROR_ENUM unlock()
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeUnlock(m_managerPtr);
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Closes Wifi network
     */
    public ARNETWORKAL_ERROR_ENUM closeWifiNetwork()
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeCloseWifiNetwork(m_managerPtr);
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Sets the send buffer size
     */
    public ARNETWORKAL_ERROR_ENUM setSendBufferSize(int bufferSize)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeSetSendBufferSize(m_managerPtr, bufferSize);
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Sets the recv buffer size
     */
    public ARNETWORKAL_ERROR_ENUM setRecvBufferSize(int bufferSize)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeSetRecvBufferSize(m_managerPtr, bufferSize);
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Sets the send class selector
     */
    public ARNETWORKAL_ERROR_ENUM setSendClassSelector(ARSAL_SOCKET_CLASS_SELECTOR_ENUM cs)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeSetSendClassSelector(m_managerPtr, cs.getValue());
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Sets the recv class selector
     */
    public ARNETWORKAL_ERROR_ENUM setRecvClassSelector(ARSAL_SOCKET_CLASS_SELECTOR_ENUM cs)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeSetRecvClassSelector(m_managerPtr, cs.getValue());
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Enable data dump
     */
    public ARNETWORKAL_ERROR_ENUM enableDataDump(String logDir, String name)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeEnableDataDump(m_managerPtr, logDir, name);
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Dump some data
     */
    public ARNETWORKAL_ERROR_ENUM dumpData(ARNativeData data, byte tag)
    {
        int size = data.getDataSize();
        return dumpData(data, size, tag);
    }

    /**
     * Dump some data, with the given maximum size
     */
    public ARNETWORKAL_ERROR_ENUM dumpData(ARNativeData data, int maxSize, byte tag)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int totalsize = data.getDataSize();
        int dumpsize = maxSize < totalsize ? maxSize : totalsize;
        int intError = nativeDumpData(m_managerPtr, tag, data.getData(), totalsize, dumpsize);
        error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    /**
     * Initialize BLE network to send and receive data
     */
    public ARNETWORKAL_ERROR_ENUM initBLENetwork(Context context, BluetoothDevice device, int recvTimeoutSec, int[] notificationIDArray)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        
        /* check parameters */
        if (context == null)
        {
            error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BAD_PARAMETER;
        }
        
        if (error == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            /* check if the BLE is available*/
            if (context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE) != true)
            {
                error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_NETWORK_TYPE;
            }
        }
        
        if (error == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            /* create deviceManager */
            //ARSALBLEManager bleManager = ARSALBLEManager.getInstance(context.getApplicationContext());
            
            /* init the ARNetworkALBLEManager */
            int intError = nativeInitBLENetwork(m_managerPtr, context.getApplicationContext(), device, recvTimeoutSec, notificationIDArray);
            error =  ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        }
        
        return error;
    }
    
    /**
     * cancel BLE network connection
     */
    public ARNETWORKAL_ERROR_ENUM cancelBLENetwork()
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        
        if(m_initOk == false)
        {
            error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        }
        
        if (error == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            /* close the ARNetworkALBLEManager */
            int intError = nativeCancelBLENetwork(m_managerPtr);
            error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        }
        
        return error;
    }
    
    /**
     * Closes BLE network
     */
    public ARNETWORKAL_ERROR_ENUM closeBLENetwork(Context context)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        
        /* check parameters */
        if (context == null)
        {
            error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BAD_PARAMETER;
        }
        
        if (error == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            /* check if the BLE is available*/
            if (context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE) != true)
            {
                error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_NETWORK_TYPE;
            }
        }
        
        if (error == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            /* close the ARNetworkALBLEManager */
            int intError = nativeCloseBLENetwork(m_managerPtr);
            error = ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        }
        
        return error;
    }

    /**
     * Initialise Mux network
     */
    public ARNETWORKAL_ERROR_ENUM initMuxNetwork(Mux mux)
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        if(mux != null)
        {
            Mux.Ref muxref = mux.newMuxRef();
            int intError = nativeInitMuxNetwork(m_managerPtr, muxref.getCPtr());
            muxref.release();
            error =  ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        }
        return error;
    }

    /**
     * Close Mux network
     */
    public ARNETWORKAL_ERROR_ENUM closeMuxNetwork()
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeCloseMuxNetwork(m_managerPtr);
        error =  ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        return error;
    }

    public String getCId()
    {
        return String.format("0x%08x", m_managerPtr);
    }
}
