/**
 *  @file  ARNETWORKAL_WifiNetwork.h
 *  @brief private headers of Wifi network manager allow to send over wifi network.
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_WIFINETWORK_PRIVATE_H_
#define _ARNETWORKAL_WIFINETWORK_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

/**
 *  @brief Create a new WifiNetwork object.
 *  @warning This function allocate memory
 *  @post ARNETWORKAL_WifiNetwork_Delete() must be called to delete the wifi network and free the memory allocated.
 *  @param[in] manager address of the pointer on the Manager
 *  @return eARNETWORKAL_ERROR
 *  @see ARNETWORKAL_WifiNetwork_Delete()
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_New (ARNETWORKAL_Manager_t *manager);

/**
 *  @brief Delete the WifiNetwork
 *  @warning This function free memory
 *  @param[in] manager address of the pointer on the Manager
 *  @see ARNETWORKAL_WifiNetwork_New()
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Delete (ARNETWORKAL_Manager_t *manager);

/**
 *  @brief Connect the socket in UDP to a port of an address. the socket will be used to send over wifi network.
 *  @param[in] manager address of the pointer on the Manager
 *  @param[in] addr address of connection at which the data will be sent.
 *  @param[in] port port on which the data will be sent.
 *  @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Connect (ARNETWORKAL_Manager_t *manager, const char *addr, int port);

/**
 *  @brief Bind the Receiver' socket in UDP to a port. the socket will be used to receive the data.
 *  @param[in] manager address of the pointer on the Manager
 *  @param[in] port port on which the data will be received.
 *  @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the receiving.
 *  @return error equal to ARNETWORKAL_OK if the Bind if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
**/
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Bind (ARNETWORKAL_Manager_t *manager, unsigned short port, int timeoutSec);

/**
 *  @brief Callback defines to push next frame to send to Network.
 *  @param[in] manager address of the pointer on the Manager
 *	@param[in] frame frame to push
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_WifiNetwork_pushNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 *  @brief Callback defines to pop next frame received from Network.
 *  @param[in] manager address of the pointer on the Manager
 *	@param[in] frame frame to pop
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_WifiNetwork_popNextFrameCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 *  @brief Callback defines to send all frames to Network.
 *  @param[in] manager address of the pointer on the Manager
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_WifiNetwork_sendingCallback(ARNETWORKAL_Manager_t *manager);

/**
 *  @brief Callback defines to receive frames from Network.
 *  @param[in] manager address of the pointer on the Manager
 *  @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_CALLBACK_RETURN ARNETWORKAL_WifiNetwork_receivingCallback(ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_WIFINETWORK_PRIVATE_H_ */

