/**
 *  @file ARNETWORK_OSSPECIFIC_Error.h
 *  @brief libARNetwork_OSSpecific errors known.
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORK_OSSPECIFIC_ERROR_H_
#define _ARNETWORK_OSSPECIFIC_ERROR_H_

/**
 *  @brief libARNetwork_OSSpecific errors known.
 */
typedef enum
{
    ARNETWORK_OSSPECIFIC_OK = 0,                			/**< No error */
    ARNETWORK_OSSPECIFIC_ERROR = -1000,         			/**< Unknown generic error */
    ARNETWORK_OSSPECIFIC_ERROR_ALLOC,           			/**< Memory allocation error */
    ARNETWORK_OSSPECIFIC_ERROR_BAD_PARAMETER,   			/**< Bad parameters */

    ARNETWORK_OSSPECIFIC_ERROR_MANAGER = -2000, 			/**< Unknown ARNETWORK_OSSPECIFIC_Manager error */

    ARNETWORK_OSSPECIFIC_ERROR_SOCKET = -4000,  			/**< Unknown socket error */
    ARNETWORK_OSSPECIFIC_ERROR_SOCKET_CREATION,				/**< Socket error during creation */
    ARNETWORK_OSSPECIFIC_ERROR_SOCKET_PERMISSION_DENIED, 	/**< Permission denied on a socket */
} eARNETWORK_OSSPECIFIC_ERROR;

/**
 * @brief Gets the error string associated with an eARNETWORK_OSSPECIFIC_ERROR
 * @param error The error to describe
 * @return A static string describing the error
 *
 * @note User should NEVER try to modify a returned string
 */
char* ARNETWORK_OSSPECIFIC_Error_ToString (eARNETWORK_OSSPECIFIC_ERROR error);

#endif /* _ARNETWORK_OSSPECIFIC_ERROR_H_ */
