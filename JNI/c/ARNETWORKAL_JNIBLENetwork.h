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
 * @file  ARNETWORKAL_JNIBLENetwork.h
 * @brief private headers of BLE network manager allow to send over ble network.
 * @date 
 * @author 
 */

#ifndef _ARNETWORKAL_JNI_BLENETWORK_PRIVATE_H_
#define _ARNETWORKAL_JNI_BLENETWORK_PRIVATE_H_

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#define ARNETWORKAL_JNIBLENETWORK_MEDIA_MTU       (20)
#define ARNETWORKAL_JNIBLENETWORK_HEADER_SIZE     (2)
#define ARNETWORKAL_JNIBLENETWORK_MAX_BUFFER_SIZE (ARNETWORKAL_JNIBLENETWORK_MEDIA_MTU - ARNETWORKAL_JNIBLENETWORK_HEADER_SIZE)

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
 * @brief set the OnDisconnect Callback
 * @param manager pointer on the Manager
 * @param onDisconnectCallbak function called on disconnect
 * @param customData custom data to send to the onDisconnectCallback
 */
eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_SetOnDisconnectCallback(ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData);

/**
 * @brief Connect to a BLE device.
 * @param[in] manager address of the pointer on the Manager
 * @param[in] device address of device at which the data will be sent and received.
 * @param[in] timeoutSec timeout in seconds set on the socket to limit the time of blocking of the receiving.
 * @param[in] notificationIDArray list of the buffer ID to notify
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 */
eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec, jintArray notificationIDArray);

/**
 * @brief create the JNIBLENetwork
 * @warning This function allocate memory
 * @param[in] manager the networkAL_Manager
 * @param[in] jContext the context
 * @return error equal to ARNETWORKAL_OK if the connection if successful otherwise equal to negative value in eARNETWORKAL_ERROR.
 * @see ARNETWORKAL_JNIBLENetwork_Delete()
 */
eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_New (ARNETWORKAL_Manager_t *manager, jobject jContext);

/**
 * @brief Delete the JNIBLENetwork
 * @warning This function free memory
 * @param manager the networkAL_Manager
 * @see ARNETWORK_Manager_New()
 */
eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Delete (ARNETWORKAL_Manager_t *manager);

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Cancel (ARNETWORKAL_Manager_t *manager);

#endif /** _ARNETWORKAL_JNI_BLENETWORK_PRIVATE_H_ */
