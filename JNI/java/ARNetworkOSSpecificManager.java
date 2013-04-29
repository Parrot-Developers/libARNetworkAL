package com.parrot.arsdk.arnetwork;

import android.util.Log;
import com.parrot.arsdk.arsal.ARNativeData;

/**
 * Network manager allow to send and receive on network.
**/
public abstract class ARNetworkOSSpecificManager 
{
    private static final String TAG = "ARNetworkOSSpecificManager";

    private native long nativeNew( int jerror );
    private native int nativeDelete( long jManager);

    private long m_manager;
    private boolean m_initOk;
    
    /**
     *  Constructor
    **/
    public ARNetworkOSSpecificManager() 
    {
        int error = eARNETWORK_OSSPECIFIC_ERROR.ARNETWORK_OK.ordinal();
        m_initOk = false;
        m_manager = nativeNew(error);
        
        Log.d (TAG, "Error:" + error );
        
        if( m_manager != 0 )
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
            nativeDelete(m_manager);
            m_manager = 0;
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
        return m_manager;
    }
    
    /**
     *  Get is the Manager is correctly initialized and if it is usable
     *  @return true is the Manager is usable
    **/
    public boolean isCorrectlyInitialized () 
    {
        return m_initOk;
    }
}
