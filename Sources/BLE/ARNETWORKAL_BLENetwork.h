/**
 * @file  ARNETWORKAL_BLENetwork.h
 * @brief private headers of BLE network manager allow to send over ble network.
 * @date 06/11/2013
 * @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_BLENETWORK_PRIVATE_H_
#define _ARNETWORKAL_BLENETWORK_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Error.h>

/**
 * @brief Create a new BLENetwork object.
 * @warning This function allocate memory
 * @post ARNETWORKAL_BLENetwork_Delete() must be called to delete the BLE network and free the memory allocated.
 * @param[in] manager address of the pointer on the Manager
 * @return eARNETWORKAL_ERROR
 * @see ARNETWORKAL_BLENetwork_Delete()
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_New (ARNETWORKAL_Manager_t *manager);

/**
 * @brief Delete the BLENetwork
 * @warning This function free memory
 * @param[in] manager address of the pointer on the Manager
 * @see ARNETWORKAL_BLENetwork_New()
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Delete (ARNETWORKAL_Manager_t *manager);

/**
 * @brief Gets the bandwith of the network
 * @param[in] manager pointer on the Manager
 * @param[out] pointer which will hold the upload bandwidth, in bytes per second (optionnal, can be NULL)
 * @param[out] pointer which will hold the download bandwidth, in bytes per second (optionnal, can be NULL)
 * @return error see ::eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_GetBandwidth (ARNETWORKAL_Manager_t *manager, uint32_t *uploadBw, uint32_t *downloadBw);

/**
 * @brief Thread entry point for the bandwidth measurement.
 * @param manager pointer on the Manager, casted as void *
 * @return always returns (void *)0
 */
void *ARNETWORKAL_BLENetwork_BandwidthThread (void *param);

/**
 * @brief Callback defines to push next frame to send to Network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] frame frame to push
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_PushFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Callback defines to pop next frame received from Network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] frame frame to pop
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_PopFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Callback defines to send all frames to Network.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_Send(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Callback defines to receive frames from Network.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_BLENetwork_Receive(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Callback defines to unlock all functions locked.
 * this function is call by ARNetwork to permit to join its threads.
 * @param manager The manager which should read from the network
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 **/
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Unlock(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Connect to a BLE device.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] device address of device manager at which the data will be sent and received.
 * @param[in] device address of device at which the data will be sent and received.
 * @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the receiving.
 * @param[in] notificationIDs list of the buffer ID to notify. If NULL all buffers of the receiver are notify.
 * @param[in] numberOfNotificationID size number buffer ID to notifiy.
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDevice_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec, int *notificationIDs, int numberOfNotificationID);

/**
 * @brief Cancel the Connect to a BLE device.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_OK if the cancel if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Cancel (ARNETWORKAL_Manager_t *manager);

/**
 * @brief set the OnDisconnect Callback
 * @param manager pointer on the Manager
 * @param onDisconnectCallbak function called on disconnect
 * @param customData custom data to send to the onDisconnectCallback
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_SetOnDisconnectCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData);

#endif /** _ARNETWORKAL_BLENETWORK_PRIVATE_H_ */
