package com.parrot.arsdk.arnetworkal;

import com.parrot.arsdk.arsal.ARNativeData;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.pm.PackageManager;

/**
 * Network manager allow to send and receive on network.
 */
public class ARNetworkALManager
{
    private static final String TAG = "ARNetworkALManager";

    private native long nativeNew();
    private native int nativeDelete(long jManager);
    private native int nativeInitWifiNetwork(long jManager, String jaddr, int sendingPort, int receivingPort, int recvTimeoutSec);
    private native int nativeSignalWifiNetwork(long jManager);
    private native int nativeCloseWifiNetwork(long jManager);
    
    /* private native int nativeInitBLENetwork(long jManager, ARNetworkALBLEManager jdeviceManager, BluetoothDevice jdevice, int recvTimeoutSec); */
    private native int nativeInitBLENetwork(long jManager, Object jdeviceManager, BluetoothDevice jdevice, int recvTimeoutSec, int[] notificationIDArray);
    private native int nativeCloseBLENetwork(long jManager);

    private long m_managerPtr;
    private boolean m_initOk;

    /**
     * Constructor
     */
    public ARNetworkALManager() {
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
    public void dispose() {
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
    public void finalize () throws Throwable {
        try {
            dispose ();
        } finally {
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
    public boolean isCorrectlyInitialized () {
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
     * Signal the wifi network to unblock all waiting functions
     */
    public ARNETWORKAL_ERROR_ENUM signalWifiNetwork()
    {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
        int intError = nativeSignalWifiNetwork(m_managerPtr);
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
            ARNetworkALBLEManager bleManager = new ARNetworkALBLEManager(context.getApplicationContext());
            
            /* init the ARNetworkALBLEManager */
            int intError = nativeInitBLENetwork(m_managerPtr, bleManager, device, recvTimeoutSec, notificationIDArray);
            error =  ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
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
}
