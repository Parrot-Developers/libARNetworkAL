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
 * @file ARNETWORKAL_JNIManager.c
 * @brief JNI between the ARNETWORK_Manager.h and ARNETWORKAL_Manager.java
 * @date 04/29/2013
 * @author frederic.dhaeyer@parrot.com
 **/

/*****************************************
 *
 *             include file :
 *
 ******************************************/

#include <jni.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include <libARNetworkAL/ARNETWORKAL_Error.h>
#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#include "ARNETWORKAL_JNIManager.h"

#include "ARNETWORKAL_JNIBLENetwork.h"

/*****************************************
 *
 *             private header:
 *
 ******************************************/

#define ARNETWORKAL_JNIMANAGER_TAG "ARNETWORKAL_JNIManager" /** tag used by the print of the file */

/*****************************************
 *
 *             implementation :
 *
 ******************************************/

JavaVM *ARNETWORKAL_JNIManager_VM = NULL; /** reference to the java virtual machine */

/**
 * @brief save the reference to the java virtual machine
 * @note this function is automatically called on the JNI startup
 * @param[in] VM reference to the java virtual machine
 * @param[in] reserved data reserved
 * @return JNI version
 **/
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *VM, void *reserved)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIMANAGER_TAG, "Library has been loaded");

    /** Saving the reference to the java virtual machine */
    ARNETWORKAL_JNIManager_VM = VM;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORKAL_JNIMANAGER_TAG, "JNI_OnLoad ARNETWORKAL_JNIManager_VM: %p ", ARNETWORKAL_JNIManager_VM);

    /** Return the JNI version */
    return JNI_VERSION_1_6;
}

/**
 * @brief get ARNETWORKAL_MANAGER_DEFAULT_ID_MAX
 * @return value of ARNETWORKAL_MANAGER_DEFAULT_ID_MAX
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeGetDefineDefaultIdMAX (JNIEnv *env, jclass class)
{
    return ARNETWORKAL_MANAGER_DEFAULT_ID_MAX;
}

/**
 * @brief get ARNETWORKAL_MANAGER_WIFI_ID_MAX
 * @return value of ARNETWORKAL_MANAGER_WIFI_ID_MAX
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeGetDefineWifiIdMAX (JNIEnv *env, jclass class)
{
    return ARNETWORKAL_MANAGER_WIFI_ID_MAX;
}

/**
 * @brief get ARNETWORKAL_MANAGER_BLE_ID_MAX
 * @return value of ARNETWORKAL_MANAGER_BLE_ID_MAX
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeGetDefineBleIdMAX (JNIEnv *env, jclass class)
{
    return ARNETWORKAL_MANAGER_BLE_ID_MAX;
}

/**
 * @brief Create a new manager
 * @warning This function allocate memory
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @return Pointer on the ARNETWORKAL_Manager_t.
 * @note This creator adds for all output, one other IOBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer arrays.
 * @warning The identifiers of the IoBuffer should not exceed the value 128.
 * @see Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeDelete()
 *
 **/
JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeNew(JNIEnv *env, jobject obj)
{
    /** -- Create a new manager -- */
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;
    /** local declarations */
    ARNETWORKAL_Manager_t *manager = ARNETWORKAL_Manager_New(&error);

    /** print error */
    if(error != ARNETWORKAL_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORKAL_JNIMANAGER_TAG, " error: %d occurred \n", error);
    }

    return (long) manager;
}

/**
 * @brief Delete the Manager
 * @warning This function free memory
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManager adress of the ARNETWORKAL_Manager_t
 * @see Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeNew()
 **/
JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeDelete(JNIEnv *env, jobject obj, jlong jManager)
{
    /** -- Delete the Manager -- */

    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManager;
    ARNETWORKAL_Manager_Delete(&manager);
}

/**
 * @brief initialize Wifi network for sending and receiving the data.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @param[in] jaddr address of connection at which the data will be sent.
 * @param[in] sendingPort port on which the data will be sent.
 * @param[in] recvPort port on which the data will be received.
 * @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function ARNETWORK_Receiver_Read().
 * @return error equal to ARNETWORKAL_OK if the init was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeInitWifiNetwork(JNIEnv *env, jobject obj, jlong jManagerPtr, jstring jaddr, jint sendingPort, jint recvPort, jint recvTimeoutSec)
{
    /** -- initialize UDP sockets of sending and receiving the data. -- */

    /** local declarations */
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    const char *nativeString = (*env)->GetStringUTFChars(env, jaddr, NULL);
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    error = ARNETWORKAL_Manager_InitWifiNetwork(manager, nativeString, sendingPort, recvPort, recvTimeoutSec);
    (*env)->ReleaseStringUTFChars( env, jaddr, nativeString );

    return error;
}

/**
 * @brief unlocks all blocking functions.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @return error equal to ARNETWORKAL_OK if the signal was successful otherwise see eARNETWORKAL_ERROR.
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeUnlock(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_Unlock (manager);
    return error;
}

/**
 * @brief Closes Wifi network for sending and receiving the data.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @return error equal to ARNETWORKAL_OK if the close was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeCloseWifiNetwork(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    error = ARNETWORKAL_Manager_CloseWifiNetwork(manager);

    return error;
}


/**
 * @brief initialize BLE network for sending and receiving the data.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @param[in] jdeviceManager BLE manager.
 * @param[in] device BLE device.
 * @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function ARNETWORK_Receiver_Read().
 * @param[in] notificationIDArray list of the buffer ID to notify
 * @return error equal to ARNETWORKAL_OK if the init was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeInitBLENetwork(JNIEnv *env, jobject obj, jlong jManagerPtr, jobject jContext, jobject jdevice, jint recvTimeoutSec, jintArray notificationIDArray)
{
    /* -- initialize BLE of sending and receiving the data. -- */

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIMANAGER_TAG, " nativeInitBLENetwork");

    /* local declarations */
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /** -- Initialize the BLE Network -- */
    /** check parameters*/
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_JNIBLENetwork_New (manager, jContext);
    }

    if (error == ARNETWORKAL_OK)
    {
        error = ARNETWORKAL_JNIBLENetwork_Connect(manager, jdevice, recvTimeoutSec, notificationIDArray);
    }

    if(error == ARNETWORKAL_OK)
    {
        manager->pushFrame = ARNETWORKAL_JNIBLENetwork_PushFrame;
        manager->popFrame = ARNETWORKAL_JNIBLENetwork_PopFrame;
        manager->send = ARNETWORKAL_JNIBLENetwork_Send;
        manager->receive = ARNETWORKAL_JNIBLENetwork_Receive;
        manager->unlock = ARNETWORKAL_JNIBLENetwork_Unlock;
        manager->maxIds = ARNETWORKAL_MANAGER_BLE_ID_MAX;
        manager->maxBufferSize = ARNETWORKAL_JNIBLENETWORK_MAX_BUFFER_SIZE;
        manager->setOnDisconnectCallback = ARNETWORKAL_JNIBLENetwork_SetOnDisconnectCallback;
    }

    return error;
}

/**
 * @brief Cancel BLE network
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @return error equal to ARNETWORKAL_OK if the close was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeCancelBLENetwork(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIMANAGER_TAG, " nativeCancelBLENetwork");

    if(manager)
    {
        error = ARNETWORKAL_JNIBLENetwork_Cancel(manager);
    }
    else
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    return error;
}

/**
 * @brief Closes BLE network for sending and receiving the data.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @return error equal to ARNETWORKAL_OK if the close was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeCloseBLENetwork(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIMANAGER_TAG, " nativeCloseBLENetwork");

    if(manager)
    {
        error = ARNETWORKAL_JNIBLENetwork_Delete(manager);
    }

    return error;
}

/**
 * @brief initialize Mux network for sending and receiving the data.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @param[in] jMuxCtxPtr mux context.
 * @return error equal to ARNETWORKAL_OK if the init was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeInitMuxNetwork(JNIEnv *env, jobject obj, jlong jManagerPtr, jlong jMuxCtxPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    struct mux_ctx *mux_ctx = (struct mux_ctx*) (intptr_t) jMuxCtxPtr;

    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIMANAGER_TAG, " nativeInitMuxNetwork");

    if(manager && mux_ctx)
    {
        error = ARNETWORKAL_Manager_InitMuxNetwork(manager, mux_ctx);
    }

    return error;
}


/**
 * @brief Closes Mux network for sending and receiving the data.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @return error equal to ARNETWORKAL_OK if the close was successful otherwise see eARNETWORKAL_ERROR.
 **/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeCloseMuxNetwork(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    error = ARNETWORKAL_Manager_CloseMuxNetwork(manager);

    return error;
}


/**
 * @brief Sets the send bufer size.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManager address of the ARNETWORKAL_Manager_t
 * @param size The size to set
 * @return error equal to ARNETWORKAL_OK if the set was successful otherwise see eARNETWORKAL_ERROR.
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeSetSendBufferSize(JNIEnv *env, jobject obj, jlong jManager, jint size)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManager;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_SetSendBufferSize(manager, (int)size);
    return error;
}

/**
 * @brief Sets the recv bufer size.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManager address of the ARNETWORKAL_Manager_t
 * @param size The size to set
 * @return error equal to ARNETWORKAL_OK if the set was successful otherwise see eARNETWORKAL_ERROR.
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeSetRecvBufferSize(JNIEnv *env, jobject obj, jlong jManager, jint size)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManager;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_SetRecvBufferSize(manager, (int)size);
    return error;
}

/**
 * @brief Sets the send class selector.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManager address of the ARNETWORKAL_Manager_t
 * @param cs The class selector to set
 * @return error equal to ARNETWORKAL_OK if the set was successful otherwise see eARNETWORKAL_ERROR.
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeSetSendClassSelector(JNIEnv *env, jobject obj, jlong jManager, jint cs)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManager;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_SetSendClassSelector(manager, (int)cs);
    return error;
}

/**
 * @brief Sets the recv class selector.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManager address of the ARNETWORKAL_Manager_t
 * @param cs The class selector to set
 * @return error equal to ARNETWORKAL_OK if the set was successful otherwise see eARNETWORKAL_ERROR.
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeSetRecvClassSelector(JNIEnv *env, jobject obj, jlong jManager, jint cs)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManager;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_SetRecvClassSelector(manager, (int)cs);
    return error;
}

JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeEnableDataDump(JNIEnv *env, jobject obj, jlong jManager, jstring jLogDir, jstring jName)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManager;
    const char *nativeLogDir = (*env)->GetStringUTFChars(env, jLogDir, NULL);
    const char *nativeName = (*env)->GetStringUTFChars(env, jName, NULL);
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_EnableDataDump(manager, nativeLogDir, nativeName);
    (*env)->ReleaseStringUTFChars( env, jLogDir, nativeLogDir );
    (*env)->ReleaseStringUTFChars( env, jName, nativeName );
    return error;
}

JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeDumpData(JNIEnv *env, jobject obj, jlong jManager, jbyte tag, jlong nativeData, jint datasize, jint dumpsize)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *)(intptr_t) jManager;
    uint8_t *data = (uint8_t *)(intptr_t)nativeData;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_DumpData(manager, tag, data, datasize, dumpsize, NULL);
    return error;
}
