package com.parrot.arsdk.arnetwork;

import java.util.HashMap;

/**
 *  libARNetwork errors known.
**/

public enum eARNETWORK_OSSPECIFIC_ERROR
{
    ARNETWORK_OSSPECIFIC_OK (0), 						/**< no error */
    ARNETWORK_OSSPECIFIC_ERROR (-1000), 				/**< error unknown */ 
    ARNETWORK_OSSPECIFIC_ERROR_ALLOC (-999), 			/**< allocation error */
    ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER (-998), 	/**< parameter incorrect */ 
    ARNETWORK_OSSPECIFIC_ERROR_MANAGER (-2000), 		/**< manager error unknown */ 
    
    private final int m_val;
    static HashMap<Integer, eARNETWORK_OSSPECIFIC_ERROR> errList;
    
    /**
     *  Constructor
     *  @param value of the enumeration
    **/
    eARNETWORK_OSSPECIFIC_ERROR(int val)
    {
        m_val = val;
    }
    
    /**
     *  Get the value.
     *  @return value of the enumeration
    **/
    public int getValue()
    {
        return m_val;
    }
    
    /**
     *  Get error name from value.
     *  @param val value of the enumeration
     *  @return name of the enumeration equal to the value
    **/
    public static eARNETWORK_OSSPECIFIC_ERROR getErrorName(int val)
    {
        /** if the errList not exist, create it */
        if (null == errList)
        {
            eARNETWORK_OSSPECIFIC_ERROR[] errorList = eARNETWORK_OSSPECIFIC_ERROR.values();
            errList = new HashMap<Integer, eARNETWORK_OSSPECIFIC_ERROR> (errorList.length);
            for (eARNETWORK_OSSPECIFIC_ERROR err : errorList) 
            {
                errList.put(err.getValue(), err);
            }
        }
        return errList.get(val);
    }
}
