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
 *  @brief Create a new WifiNetwork object.
 *  @warning This function allocate memory
 *  @post ARNETWORK_OSSPECIFIC_WifiNetwork_Delete() must be called to delete the wifi network and free the memory allocated.
 *  @param[in] manager address of the pointer on the Manager
 *  @return eARNETWORK_OSSPECIFIC_ERROR
 *  @see ARNETWORK_OSSPECIFIC_WifiNetwork_Delete()
 */
eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_New (ARNETWORK_OSSPECIFIC_Manager_t *manager);

/**
 *  @brief Delete the WifiNetwork
 *  @warning This function free memory
 *  @param[in] manager address of the pointer on the Manager
 *  @see ARNETWORK_OSSPECIFIC_WifiNetwork_New()
 */
eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_Delete (ARNETWORK_OSSPECIFIC_Manager_t *manager);

/**
 *  @brief Connect the socket in UDP to a port of an address. the socket will be used to send over wifi network.
 *  @param[in] manager address of the pointer on the Manager
 *  @param[in] addr address of connection at which the data will be sent.
 *  @param[in] port port on which the data will be sent.
 *  @return error equal to ARNETWORK_OSSPECIFIC_OK if the connection if successful otherwise equal to negative value in eARNETWORK_OSSPECIFIC_ERROR.
 */
eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_Connect (ARNETWORK_OSSPECIFIC_Manager_t *manager, const char *addr, int port);

/**
 *  @brief Bind the Receiver' socket in UDP to a port. the socket will be used to receive the data.
 *  @param[in] manager address of the pointer on the Manager
 *  @param[in] port port on which the data will be received.
 *  @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the receiving.
 *  @return error equal to ARNETWORK_OSSPECIFIC_OK if the Bind if successful otherwise equal to negative value in eARNETWORK_OSSPECIFIC_ERROR.
**/
eARNETWORK_OSSPECIFIC_ERROR ARNETWORK_OSSPECIFIC_WifiNetwork_Bind (ARNETWORK_OSSPECIFIC_Manager_t *manager, unsigned short port, int timeoutSec);

#endif /** _NETWORK_OSSPECIFIC_MANAGER_PRIVATE_H_ */

