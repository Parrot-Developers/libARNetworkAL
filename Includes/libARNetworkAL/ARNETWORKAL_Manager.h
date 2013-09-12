/**
 * @file ARNETWORKAL_Manager.h
 * @brief network manager allow to send over network.
 * @date 04/29/2013
 * @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_MANAGER_H_
#define _ARNETWORKAL_MANAGER_H_

#include <libARNetworkAL/ARNETWORKAL_Error.h>
#include <libARNetworkAL/ARNETWORKAL_Frame.h>

#define ARNETWORKAL_MANAGER_DEFAULT_ID_MAX  256 /**< Default ID Max */
#define ARNETWORKAL_MANAGER_WIFI_ID_MAX ARNETWORKAL_MANAGER_DEFAULT_ID_MAX /**< ID Max for WifiNetwork */
#define ARNETWORKAL_MANAGER_BLE_ID_MAX  (ARNETWORKAL_MANAGER_DEFAULT_ID_MAX / 4) /**< ID Max for BLENetwork */

/**
 * @brief Network abstraction structure.
 * @see ARNETWORKAL_Manager_PushNextFrame_t
 * @see ARNETWORKAL_Manager_PopNextFrame_t
 * @see ARNETWORKAL_Manager_Sending_t
 * @see ARNETWORKAL_Manager_Receiving_t
 */
typedef struct ARNETWORKAL_Manager_s ARNETWORKAL_Manager_t;

/**
 * @brief BLE Manager (used to send/read data)
 * @note This is an application-provided object, OS Dependant
 */
typedef void* ARNETWORKAL_BLEDeviceManager_t;

/**
 * @brief BLE Device reference for the @ref ARNETWORKAL_BLEDeviceManager_t
 * @note This is an application-provided object, OS Dependant
 */
typedef void* ARNETWORKAL_BLEDevice_t;

/**
 * @brief Transfert functions return codes
 */
typedef enum
{
    ARNETWORKAL_MANAGER_RETURN_DEFAULT = 0, /**< Default return value (no error)*/
    ARNETWORKAL_MANAGER_RETURN_BUFFER_FULL, /**< Impossible to push a frame : network buffer is full */
    ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY, /**< Impossible to pop a frame, no frame in the buffer */
    ARNETWORKAL_MANAGER_RETURN_BAD_FRAME, /**< Impossible to pop a frame, frame is corrupted */
    ARNETWORKAL_MANAGER_RETURN_NO_DATA_AVAILABLE, /**< Impossible to read data from the network, no data available */
    ARNETWORKAL_MANAGER_RETURN_BAD_PARAMETERS, /**< Parameters given to the callback were not good */
    ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR, /**< Network error while reading or sending data */
} eARNETWORKAL_MANAGER_RETURN;

/**
 * @brief Queues data to be sent on the network
 * This function will queue an @ref ARNETWORKAL_Frame_t into the internal
 * buffers. Depending on the implementation, the send may also be triggered
 * by this call (like if a call to @ref ARNETWORKAL_Manager_Send_t was made)
 * @param manager The manager which should queue the data
 * @param frame The frame to be sent
 * @return A callback return code
 */
typedef eARNETWORKAL_MANAGER_RETURN (*ARNETWORKAL_Manager_PushFrame_t) (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Sends all queued datas on the network
 * This function will trigger an actual network send of all the queued data. On
 * implementations where @ref ARNETWORKAL_Manager_PushFrame_t triggers a send,
 * this function is a no-op.
 * @param manager The manager which should send all its queued data
 * @return A callback return code
 */
typedef eARNETWORKAL_MANAGER_RETURN (*ARNETWORKAL_Manager_Send_t) (ARNETWORKAL_Manager_t *manager);

/**
 * @brief Read a bunch of frames from the network
 * This function will read a bunch of data from the network, so these datas can
 * be fetched using @ref ARNETWORKAL_Manager_PopFrame_t. On implementations
 * where @ref ARNETWORKAL_Manager_PopFrame_t does an actual network read, this
 * function only waits for data to be available, but does not do an actual
 * read. In any cases, this function should be used by the receiving thread to
 * wait for new network data.
 * @param manager The manager which should read from the network
 * @return A callback return code
 */
typedef eARNETWORKAL_MANAGER_RETURN (*ARNETWORKAL_Manager_Receive_t) (ARNETWORKAL_Manager_t *manager);


/**
 * @brief Fetches data from the last received network frame
 * This function will try to read an @ref ARNETWORKAL_Frame_t from the last
 * received network frame. Depending on the implementation, this function may
 * directly read from the network instead of an internal buffer.
 * @param manager The manager which should fetch the data
 * @param frame A frame pointer in which the fetched frame should be saved
 * @return A callback return code
 */
typedef eARNETWORKAL_MANAGER_RETURN (*ARNETWORKAL_Manager_PopFrame_t) (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);


/**
 * @brief ARNETWORKAL_Manager_t internal representation
 */
struct ARNETWORKAL_Manager_s
{
    ARNETWORKAL_Manager_PushFrame_t pushFrame; /**< Manager specific PushFrame function */
    ARNETWORKAL_Manager_PopFrame_t popFrame; /**< Manager specific PopFrame function */
    ARNETWORKAL_Manager_Send_t send; /**< Manager specific send function */
    ARNETWORKAL_Manager_Receive_t receive; /**< Manager specific receive function */
    void *senderObject; /**< Internal reference, do not use */
    void *receiverObject; /**< Internal reference, do not use */
    int maxIds; /**< Maximum supported buffer ID for ARNetwork */
};

/**
 * @brief Create a new Manager
 * @warning This function allocate memory
 * @post ARNETWORKAL_Manager_Delete() must be called to delete the manager and free the memory allocated.
 * @param[out] error pointer on the error output.
 * @return Pointer on the new Manager
 * @see ARNETWORKAL_Manager_Delete()
 */
ARNETWORKAL_Manager_t* ARNETWORKAL_Manager_New(eARNETWORKAL_ERROR *error);

/**
 * @brief Delete the Manager
 * @warning This function free memory
 * @param manager address of the pointer on the Manager
 * @see ARNETWORKAL_Manager_New()
 */
void ARNETWORKAL_Manager_Delete(ARNETWORKAL_Manager_t **manager);

/**
 * @brief initialize Wifi network.
 * @param manager pointer on the Manager
 * @param[in] addr IP address of connection at which the data will be sent.
 * @param[in] sendingPort port on which the data will be sent.
 * @param[in] receivingPort port on which the data will be received.
 * @param[in] recvTimeoutSec timeout in seconds set to limit the time of blocking of the Receive function.
 * @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitWifiNetwork(ARNETWORKAL_Manager_t *manager, const char *addr, int sendingPort, int receivingPort, int recvTimeoutSec);

/**
 * @brief force timeout on Wifi network
 * When this function is called, all the functions that are blocked on sockets are immediately unblocked.
 * A typical use case is when the ARNETWORK library is closing: The threads must be joined before calling
 * @ref ARNETWORKAL_Manager_CloseWifiNetwork, but the threads might be waiting for an ARNETWORKAL timeout.
 * @param manager pointer on the Manager
 * @return Any possible error code (see @ref eARNETWORKAL_ERROR)
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_SignalWifiNetwork(ARNETWORKAL_Manager_t *manager);

/**
 * @brief close Wifi network.
 * @param manager pointer on the Manager
 * @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseWifiNetwork(ARNETWORKAL_Manager_t *manager);

/**
 * @brief initialize BLE network.
 * @param manager pointer on the Manager
 * @param deviceManager the OS device manager which will be used for network functions
 * @param device the selected OS specific BLE device to connect to
 * @param recvTimeoutSec timeout in seconds set to limit the time of blocking of the Receive function.
 * @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_InitBLENetwork(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec);

/**
 * @brief close BLE network.
 * @param manager pointer on the Manager
 * @return error equal to ARNETWORKAL_OK if the initialization if successful otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_Manager_CloseBLENetwork(ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_MANAGER_H_ */
