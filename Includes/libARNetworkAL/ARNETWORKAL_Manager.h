/**
 *  @file ARNETWORKAL_Manager.h
 *  @brief network manager allow to send over network.
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_MANAGER_H_
#define _ARNETWORKAL_MANAGER_H_

#include <libARNetworkAL/ARNETWORKAL_Error.h>
#include <libARNetworkAL/ARNETWORKAL_Frame.h>

/**
 *  @brief network manager allow to send to network and receive from a network.
 */
typedef struct ARNETWORKAL_Manager_t ARNETWORKAL_Manager_t;

/**
 *  @brief BLE debice allow to send to BLE network and receive from a BLE network.
 */
typedef void* ARNETWORKAL_BLEDevice_t;

/**
 *  @brief status return by the callback.
 */
typedef enum
{
    ARNETWORKAL_MANAGER_CALLBACK_RETURN_DEFAULT = 0, /**< default value must be returned when the status callback is OK*/
    ARNETWORKAL_MANAGER_CALLBACK_RETURN_BUFFER_FULL,
    ARNETWORKAL_MANAGER_CALLBACK_RETURN_BUFFER_EMPTY,
    ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_FRAME,
    ARNETWORKAL_MANAGER_CALLBACK_RETURN_NO_DATA_AVAILABLE,
    ARNETWORKAL_MANAGER_CALLBACK_RETURN_BAD_PARAMETERS,
} eARNETWORKAL_MANAGER_CALLBACK_RETURN;

/**
 *  @brief callback use when data is ready to push
 *  @warning the callback can't call the ARNETWORKAL's functions
 *  @return
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
 */
typedef eARNETWORKAL_MANAGER_CALLBACK_RETURN (*ARNETWORKAL_Manager_PushNextFrame_Callback_t) (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 *  @brief callback use when the  data is ready to pop
 *  @warning the callback can't call the ARNETWORKAL's functions
 *  @return
 *  @see
 */
typedef eARNETWORKAL_MANAGER_CALLBACK_RETURN (*ARNETWORKAL_Manager_PopNextFrame_Callback_t) (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 *  @brief callback use when data is ready to send over network
 *  @warning the callback can't call the ARNETWORKAL's functions
 *  @return
 *  @see
 */
typedef eARNETWORKAL_MANAGER_CALLBACK_RETURN (*ARNETWORKAL_Manager_Sending_Callback_t) (ARNETWORKAL_Manager_t *manager);

/**
 *  @brief callback use when data is ready to receive over network
 *  @warning the callback can't call the ARNETWORKAL's functions
 *  @return
 *  @see
 */
typedef eARNETWORKAL_MANAGER_CALLBACK_RETURN (*ARNETWORKAL_Manager_Receiving_Callback_t) (ARNETWORKAL_Manager_t *manager);

struct ARNETWORKAL_Manager_t
{
	ARNETWORKAL_Manager_PushNextFrame_Callback_t pushNextFrameCallback;
	ARNETWORKAL_Manager_PopNextFrame_Callback_t popNextFrameCallback;
	ARNETWORKAL_Manager_Sending_Callback_t sendingCallback;
	ARNETWORKAL_Manager_Receiving_Callback_t receivingCallback;
	void *senderObject;
	void *receiverObject;
};

/**
 *  @brief Create a new Manager
 *  @warning This function allocate memory
 *  @post ARNETWORKAL_Manager_Delete() must be called to delete the manager and free the memory allocated.
 *  @param[out] error pointer on the error output.
 *  @return Pointer on the new Manager
 *  @see ARNETWORKAL_Manager_Delete()
 */
ARNETWORKAL_Manager_t* ARNETWORKAL_Manager_New(eARNETWORKAL_ERROR *error);

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param manager address of the pointer on the Manager
 *  @see ARNETWORKAL_Manager_New()
 */
void ARNETWORKAL_Manager_Delete(ARNETWORKAL_Manager_t **manager);

/**
 *  @brief initialize Wifi network.
 *  @param manager pointer on the Manager
 *  @param[in] addr address of connection at which the data will be sent.
 *  @param[in] sendingPort port on which the data will be sent.
 *  @param[in] receivingPort port on which the data will be received.
 *  @param[in] recvTimeoutSec timeout in seconds set to limit the time of blocking of the Receiving function.
 *  @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitWiFiNetwork(ARNETWORKAL_Manager_t *manager, const char *addr, int sendingPort, int receivingPort, int recvTimeoutSec);

/**
 *  @brief close Wifi network.
 *  @param manager pointer on the Manager
 *  @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseWiFiNetwork(ARNETWORKAL_Manager_t *manager);

/**
 *  @brief initialize BLE network.
 *  @param manager pointer on the Manager
 *  @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitBLENetwork(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec);

/**
 *  @brief close BLE network.
 *  @param manager pointer on the Manager
 *  @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseBLENetwork(ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_MANAGER_H_ */
