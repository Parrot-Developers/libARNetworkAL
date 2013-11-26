/**
 * @file  ARNETWORKAL_JNIBLENetwork.h
 * @brief private headers of BLE network manager allow to send over ble network.
 * @date 
 * @author 
 */

#ifndef _ARNETWORKAL_JNI_BLENETWORK_PRIVATE_H_
#define _ARNETWORKAL_JNI_BLENETWORK_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

/**
 * @brief Callback defines to push next frame to send to Network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] frame frame to push
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_PushFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Callback defines to pop next frame received from Network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] frame frame to pop
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_PopFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Callback defines to send all frames to Network.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_Send(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Callback defines to receive frames from Network.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_Receive(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Callback defines to unlock all functions locked.
 * this function is call by ARNetwork to permit to join its threads.
 * @param manager The manager which should read from the network
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Unlock(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Connect to a BLE device.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] device address of device manager at which the data will be sent and received.
 * @param[in] device address of device at which the data will be sent and received.
 * @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the receiving.
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDeviceManager_t deviceManager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec);

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_New (ARNETWORKAL_Manager_t *manager);

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Delete (ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_JNI_BLENETWORK_PRIVATE_H_ */
