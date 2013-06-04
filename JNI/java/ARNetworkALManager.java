package com.parrot.arsdk.arnetworkal;

import android.util.Log;
import com.parrot.arsdk.arsal.ARNativeData;

/**
 * Network manager allow to send and receive on network.
**/
public class ARNetworkALManager 
{
    private static final String TAG = "ARNetworkALManager";

    private native long nativeNew();
    private native int nativeDelete(long jManager);
    private native int nativeInitWiFiNetwork(long jManager, String jaddr, int sendingPort, int receivingPort, int recvTimeoutSec);

    private long m_managerPtr;
    private boolean m_initOk;
    
    /**
     *  Constructor
    **/
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
     *  Dispose
     *  @post after this function the object must be not used more
    **/
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
     *  Destructor
    **/
    public void finalize () throws Throwable
    {
        try {
            dispose ();
        } finally {
            super.finalize ();
        }
    }
    
     /**
     *  Get the pointer C on the network manager 
     *  @return  Pointer C on the network manager 
    **/
    public long getManager () 
    {
        return m_managerPtr;
    }
    
    /**
     *  Get is the Manager is correctly initialized and if it is usable
     *  @return true is the Manager is usable
    **/
    public boolean isCorrectlyInitialized () 
    {
        return m_initOk;
    }
    
    /**
     *  Initialize Wifi network to send and receive data
    **/
    public eARNETWORKAL_ERROR initWiFiNetwork(String addr, int sendingPort, int receivingPort, int recvTimeoutSec) 
    {
    	eARNETWORKAL_ERROR error = eARNETWORKAL_ERROR.ARNETWORKAL_ERROR;
    	
    	if(addr != null)
        {
            int intError = nativeInitWiFiNetwork(m_managerPtr, addr, sendingPort, receivingPort, recvTimeoutSec);
            error =  eARNETWORKAL_ERROR.getErrorName(intError); 
        }
    	
    	return error;
    }
}
