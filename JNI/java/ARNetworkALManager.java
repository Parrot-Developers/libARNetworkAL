package com.parrot.arsdk.arnetworkal;

import com.parrot.arsdk.arsal.ARNativeData;

/**
 * Network manager allow to send and receive on network.
 */
public class ARNetworkALManager
{
    private static final String TAG = "ARNetworkALManager";

    private native long nativeNew();
    private native int nativeDelete(long jManager);
    private native int nativeInitWiFiNetwork(long jManager, String jaddr, int sendingPort, int receivingPort, int recvTimeoutSec);

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
    public ARNETWORKAL_ERROR_ENUM initWiFiNetwork(String addr, int sendingPort, int receivingPort, int recvTimeoutSec) {
        ARNETWORKAL_ERROR_ENUM error = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;

        if(addr != null)
        {
            int intError = nativeInitWiFiNetwork(m_managerPtr, addr, sendingPort, receivingPort, recvTimeoutSec);
            error =  ARNETWORKAL_ERROR_ENUM.getFromValue(intError);
        }

        return error;
    }
}
