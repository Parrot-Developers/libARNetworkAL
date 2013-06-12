/**
 *  @file  ARNETWORKAL_BLENetwork.h
 *  @brief private headers of BLE network manager allow to send over ble network.
 *  @date 06/11/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_BLENETWORK_PRIVATE_H_
#define _ARNETWORKAL_BLENETWORK_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

/**
 *  @brief Create a new BLENetwork object.
 *  @warning This function allocate memory
 *  @post ARNETWORKAL_BLENetwork_Delete() must be called to delete the BLE network and free the memory allocated.
 *  @param[in] manager address of the pointer on the Manager
 *  @return eARNETWORKAL_ERROR
 *  @see ARNETWORKAL_BLENetwork_Delete()
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_New (ARNETWORKAL_Manager_t *manager);

/**
 *  @brief Delete the BLENetwork
 *  @warning This function free memory
 *  @param[in] manager address of the pointer on the Manager
 *  @see ARNETWORKAL_BLENetwork_New()
 */
eARNETWORKAL_ERROR ARNETWORKAL_BLENetwork_Delete (ARNETWORKAL_Manager_t *manager);

/**
 *  @brief Callback defines to push next frame to send to Network.
 *  @param[in] manager address of the pointer on the Manager
 *	@param[in] frame frame to push
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_pushNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 *  @brief Callback defines to pop next frame received from Network.
 *  @param[in] manager address of the pointer on the Manager
 *	@param[in] frame frame to pop
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_popNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 *  @brief Callback defines to send all frames to Network.
 *  @param[in] manager address of the pointer on the Manager
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_sendingCallback(ARNETWORKAL_Manager_t *manager);

/**
 *  @brief Callback defines to receive frames from Network.
 *  @param[in] manager address of the pointer on the Manager
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_BLENetwork_receivingCallback(ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_BLENETWORK_PRIVATE_H_ */
