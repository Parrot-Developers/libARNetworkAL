package com.parrot.arsdk.arnetworkal;

import java.util.HashMap;

/**
 *  libARNetwork errors known.
**/

public enum eARNETWORKAL_ERROR
{
    ARNETWORKAL_OK (0), 						/**< no error */
    ARNETWORKAL_ERROR (-1000), 				/**< error unknown */ 
    ARNETWORKAL_ERROR_ALLOC (-999), 			/**< allocation error */
    ARNETWORKAL_ERROR_BAD_PARAMETER (-998), 	/**< parameter incorrect */ 
    ARNETWORKAL_ERROR_MANAGER (-2000); 		/**< manager error unknown */ 
    
    private final int m_val;
    static HashMap<Integer, eARNETWORKAL_ERROR> errList;
    
    /**
     *  Constructor
     *  @param value of the enumeration
    **/
    eARNETWORKAL_ERROR(int val)
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
    public static eARNETWORKAL_ERROR getErrorName(int val)
    {
        /** if the errList not exist, create it */
        if (null == errList)
        {
            eARNETWORKAL_ERROR[] errorList = eARNETWORKAL_ERROR.values();
            errList = new HashMap<Integer, eARNETWORKAL_ERROR> (errorList.length);
            for (eARNETWORKAL_ERROR err : errorList) 
            {
                errList.put(err.getValue(), err);
            }
        }
        return errList.get(val);
    }
}
