/**
 *  @file ARNETWORKAL_Error.h
 *  @brief libARNetworkAL errors known.
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_ERROR_H_
#define _ARNETWORKAL_ERROR_H_

/**
 *  @brief libARNetworkAL errors known.
 */
typedef enum
{
    ARNETWORKAL_OK = 0,                			/**< No error */
    ARNETWORKAL_ERROR = -1000,         			/**< Unknown generic error */
    ARNETWORKAL_ERROR_ALLOC,           			/**< Memory allocation error */
    ARNETWORKAL_ERROR_BAD_PARAMETER,   			/**< Bad parameters */

    ARNETWORKAL_ERROR_MANAGER = -2000, 			/**< Unknown ARNETWORKAL_Manager error */

    ARNETWORKAL_ERROR_SOCKET = -4000,  			/**< Unknown socket error */
    ARNETWORKAL_ERROR_SOCKET_CREATION,				/**< Socket error during creation */
    ARNETWORKAL_ERROR_SOCKET_PERMISSION_DENIED, 	/**< Permission denied on a socket */
} eARNETWORKAL_ERROR;

/**
 * @brief Gets the error string associated with an eARNETWORKAL_ERROR
 * @param error The error to describe
 * @return A static string describing the error
 *
 * @note User should NEVER try to modify a returned string
 */
char* ARNETWORKAL_Error_ToString (eARNETWORKAL_ERROR error);

#endif /* _ARNETWORKAL_ERROR_H_ */
