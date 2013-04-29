/**
 *  @file ARNETWORK_OSSPECIFIC_Manager.h
 *  @brief private headers of network manager allow to send over network.
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _NETWORK_OSSPECIFIC_MANAGER_PRIVATE_H_
#define _NETWORK_OSSPECIFIC_MANAGER_PRIVATE_H_

#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Manager.h>

/**
 *  @brief status return by the callback.
 */
typedef enum
{
    ARNETWORK_OSSPECIFIC_MANAGER_CALLBACK_RETURN_DEFAULT = 0, /**< default value must be returned when the status callback differ of ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT */
} eARNETWORK_OSSPECIFIC_MANAGER_CALLBACK_RETURN;

/**
 *  @brief callback use when data is ready to push
 *  @warning the callback can't call the ARNETWORK_OSSPECIFIC's functions
 *  @return
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
 */
typedef eARNETWORK_OSSPECIFIC_MANAGER_CALLBACK_RETURN (*ARNETWORK_OSSPECIFIC_Manager_PushNextFrame_Callback_t) (ARNETWORK_OSSPECIFIC_Manager_t *manager);

/**
 *  @brief callback use when the  data is ready to pop
 *  @warning the callback can't call the ARNETWORK_OSSPECIFIC's functions
 *  @return
 *  @see
 */
typedef eARNETWORK_OSSPECIFIC_MANAGER_CALLBACK_RETURN (*ARNETWORK_OSSPECIFIC_Manager_PopNextFrame_Callback_t) (ARNETWORK_OSSPECIFIC_Manager_t *manager);

/**
 *  @brief callback use when data is ready to send over network
 *  @warning the callback can't call the ARNETWORK_OSSPECIFIC's functions
 *  @return
 *  @see
 */
typedef eARNETWORK_OSSPECIFIC_MANAGER_CALLBACK_RETURN (*ARNETWORK_OSSPECIFIC_Manager_Sending_Callback_t) (ARNETWORK_OSSPECIFIC_Manager_t *manager);

/**
 *  @brief callback use when data is ready to receive over network
 *  @warning the callback can't call the ARNETWORK_OSSPECIFIC's functions
 *  @return
 *  @see
 */
typedef eARNETWORK_OSSPECIFIC_MANAGER_CALLBACK_RETURN (*ARNETWORK_OSSPECIFIC_Manager_Receiving_Callback_t) (ARNETWORK_OSSPECIFIC_Manager_t *manager);

/**
 *  @brief network manager allow to .
**/
struct ARNETWORK_OSSPECIFIC_Manager_t
{
	eARNETWORK_OSSPECIFIC_MANAGER_NETWORK_TYPE type;
	void *senderObject;
	void *receiverObject;
	ARNETWORK_OSSPECIFIC_Manager_PushNextFrame_Callback_t pushNextFrameCallback;
	ARNETWORK_OSSPECIFIC_Manager_PopNextFrame_Callback_t popNextFrameCallback;
	ARNETWORK_OSSPECIFIC_Manager_Sending_Callback_t sendingCallback;
	ARNETWORK_OSSPECIFIC_Manager_Receiving_Callback_t receivingCallback;
};

#endif /** _NETWORK_OSSPECIFIC_MANAGER_PRIVATE_H_ */

