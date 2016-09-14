/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 * @file  ARNETWORKAL_WifiNetwork.h
 * @brief private headers of Wifi network manager allow to send over wifi network.
 * @date 04/29/2013
 * @author frederic.dhaeyer@parrot.com
 */

#ifndef _ARNETWORKAL_WIFINETWORK_PRIVATE_H_
#define _ARNETWORKAL_WIFINETWORK_PRIVATE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#else
/* define a default common value */
#define IP_MAXPACKET 65535
#endif

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#define UDP_HEADER_SIZE 8

/** Maximum network buffer size.
 * It corresponds to the maximum amount of data you can put in a single ARNetwork packet over IP.
 */
#define ARNETWORKAL_WIFINETWORK_MAX_DATA_BUFFER_SIZE         (IP_MAXPACKET - UDP_HEADER_SIZE - offsetof(ARNETWORKAL_Frame_t, dataPtr))

/**
 * @brief Create a new WifiNetwork object.
 * @warning This function allocate memory
 * @post ARNETWORKAL_WifiNetwork_Delete() must be called to delete the wifi network and free the memory allocated.
 * @param[in] manager pointer on the Manager
 * @return eARNETWORKAL_ERROR
 * @see ARNETWORKAL_WifiNetwork_Delete()
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_New (ARNETWORKAL_Manager_t *manager);

/**
 * @brief Signal the Manager to stop all sockets blocking operations
 * @param[in] manager pointer on the Manager
 * @return eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Signal(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Gets the bandwith of the network
 * @param[in] manager pointer on the Manager
 * @param[out] pointer which will hold the upload bandwidth, in bytes per second (optionnal, can be NULL)
 * @param[out] pointer which will hold the download bandwidth, in bytes per second (optionnal, can be NULL)
 * @return error see ::eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetBandwidth (ARNETWORKAL_Manager_t *manager, uint32_t *uploadBw, uint32_t *downloadBw);

/**
 * @brief Thread entry point for the bandwidth measurement.
 * @param manager pointer on the Manager, casted as void *
 * @return always returns (void *)0
 */
void *ARNETWORKAL_WifiNetwork_BandwidthThread (void *param);


/**
 * @brief Delete the WifiNetwork
 * @warning This function free memory
 * @param[in] manager address of the pointer on the Manager
 * @see ARNETWORKAL_WifiNetwork_New()
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Delete (ARNETWORKAL_Manager_t *manager);

/**
 * @brief Connect the socket in UDP to a port of an address. the socket will be used to send over wifi network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] addr address of connection at which the data will be sent.
 * @param[in] port port on which the data will be sent.
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Connect (ARNETWORKAL_Manager_t *manager, const char *addr, int port);

/**
 * @brief Cancel the connect to wifi device.
 * @param[in] manager pointer on the Manager
 * @return error equal to ARNETWORKAL_OK if the cancel if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Cancel (ARNETWORKAL_Manager_t *manager);

/**
 * @brief Bind the Receiver' socket in UDP to a port. the socket will be used to receive the data.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] port port on which the data will be received.
 * @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the receiving.
 * @return error equal to ARNETWORKAL_OK if the Bind if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 **/
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_Bind (ARNETWORKAL_Manager_t *manager, unsigned short port, int timeoutSec);

/**
 * @brief Callback defines to push next frame to send to Network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] frame frame to push
 * @return error equal to ARNETWORKAL_MANAGER_CALLBACK_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_CALLBACK_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_PushFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Callback defines to pop next frame received from Network.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] frame frame to pop
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_PopFrame(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame);

/**
 * @brief Callback defines to send all frames to Network.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_Send(ARNETWORKAL_Manager_t *manager);

/**
 * @brief Callback defines to receive frames from Network.
 * @param[in] manager address of the pointer on the Manager
 * @return error equal to ARNETWORKAL_MANAGER_DEFAULT if the the next frame pushed on success otherwise error in eARNETWORKAL_MANAGER_RETURN.
 **/
eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_WifiNetwork_Receive(ARNETWORKAL_Manager_t *manager);

/**
 * @brief set the OnDisconnect Callback
 * @param manager pointer on the Manager
 * @param onDisconnectCallbak function called on disconnect
 * @param customData custom data to send to the onDisconnectCallback
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetOnDisconnectCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData);

/**
 * @brief Sets the size of the send buffer
 * @param manager pointer on the Manager
 * @param bufferSize requested size for the buffer
 * @return ARNETWORKAL_OK if the buffer size was set, otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetSendBufferSize(ARNETWORKAL_Manager_t *manager, int bufferSize);

/**
 * @brief Sets the size of the receive buffer
 * @param manager pointer on the Manager
 * @param bufferSize requested size for the buffer
 * @return ARNETWORKAL_OK if the buffer size was set, otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetRecvBufferSize(ARNETWORKAL_Manager_t *manager, int bufferSize);

/**
 * @brief Gets the size of the send buffer
 * @param manager pointer on the Manager
 * @param bufferSize Pointer which will hold the size of the buffer
 * @return see ::eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetSendBufferSize(ARNETWORKAL_Manager_t *manager, int *bufferSize);

/**
 * @brief Gets the size of the receive buffer
 * @param manager pointer on the Manager
 * @param bufferSize Pointer which will hold the size of the buffer
 * @return see ::eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetRecvBufferSize(ARNETWORKAL_Manager_t *manager, int *bufferSize);

/**
 * @brief Sets the class selector for the send socket
 * @param manager pointer on the Manager
 * @param classSelector class selector
 * @return ARNETWORKAL_OK if the class selector was set, otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetSendClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR classSelector);

/**
 * @brief Sets the class selector for the receive socket
 * @param manager pointer on the Manager
 * @param classSelector class selector
 * @return ARNETWORKAL_OK if the class selector was set, otherwise see eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_SetRecvClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR classSelector);

/**
 * @brief Gets the class selector for the send socket
 * @param manager pointer on the Manager
 * @param classSelector pointer on the class selector
 * @return see ::eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetSendClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR *classSelector);

/**
 * @brief Gets the class selector for the receive socket
 * @param manager pointer on the Manager
 * @param classSelector pointer on the class selector
 * @return see ::eARNETWORKAL_ERROR
 */
eARNETWORKAL_ERROR ARNETWORKAL_WifiNetwork_GetRecvClassSelector(ARNETWORKAL_Manager_t *manager, eARSAL_SOCKET_CLASS_SELECTOR *classSelector);

#endif /** _ARNETWORKAL_WIFINETWORK_PRIVATE_H_ */
