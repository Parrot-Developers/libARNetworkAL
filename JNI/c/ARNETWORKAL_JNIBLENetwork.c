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
 * @file  ARNETWORKAL_JNIBLENetwork.c
 * @brief private headers of BLE network manager allow to send over ble network.
 * @date
 * @author
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <jni.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include <libARNetworkAL/ARNETWORKAL_Manager.h>

#include "ARNETWORKAL_JNIManager.h"

#include "ARNETWORKAL_JNIBLENetwork.h"
//#include "../../Sources/BLE/ARNETWORKAL_BLENetwork.h"

/*****************************************
 *
 *             define :
 *
 *****************************************/

#define ARNETWORKAL_JNIBLENETWORK_TAG "JNIBLENetwork"

extern JavaVM *ARNETWORKAL_JNIManager_VM; /** reference to the java virtual machine */

static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_CONNECT;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_CANCEL;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_DISCONNECT;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_PUSH_FRAME;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_POP_FRAME;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_RECEIVE;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_UNLOCK;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_PUSH_FRAME;

static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_ID;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_DATA;
static jmethodID ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_RESULT;

/**
 * @brief JNI BLE Network
 */
typedef struct
{
    jobject jBLENetwork; /**< java BLENetwork manager */
    ARNETWORKAL_Manager_t *manager; /**< NetworkAL manager */
    jbyteArray currentFrameByteArray; /**< Byte Array storing the currentFrame */
    uint8_t* currentFrame; /**< currentFrame to store the data pop */
    ARNETWORKAL_Manager_OnDisconnect_t onDisconnect; /**< function called on disconnect */
    void *onDisconnectCustomData; /**< custom data sent to the onDisconnect */
} ARNETWORKAL_JNIBLENETWORK_Object_t;

/**
 * @brief Create a new ARNETWORKAL_JNIBLENETWORK_Object_t object.
 * @warning This function allocate memory
 * @post ARNETWORKAL_JNIBLENETWORK_ObjectDelete() must be called to delete the BLE network and free the memory allocated.
 * @param env java environement
 * @param[in] manager the networkAL Manager
 * @param[in] jContext the context
 * @param[out] error eARNETWORKAL_ERROR
 * @return the new ARNETWORKAL_JNIBLENETWORK
 * @see ARNETWORKAL_JNIBLENETWORK_ObjectDelete()
 */
ARNETWORKAL_JNIBLENETWORK_Object_t *ARNETWORKAL_JNIBLENetwork_ObjectNew (JNIEnv *env, ARNETWORKAL_Manager_t *manager, jobject jContext, eARNETWORKAL_ERROR *error);

/**
 * @brief Delete the ARNETWORKAL_JNIBLENETWORK_Object_t
 * @warning This function free memory
 * @param env java environement
 * @param jniBLENetwork JNIBLENETWORK to delete
 * @see ARNETWORKAL_JNIBLENetwork_ObjectNew()
 */
void ARNETWORKAL_JNIBLENetwork_ObjectDelete (JNIEnv *env, ARNETWORKAL_JNIBLENETWORK_Object_t **jniBLENetwork);

/**
 * @brief store in global reference the currentFrameByteArray and store the pointer on its data
 * @param env java environement
 * @param jniBLENetwork the JNIBLENETWORK
 * @param currentFrameByteArray the current drame to store
 */
void ARNETWORKAL_JNIBLENetwork_ObjectStoreCurrentFrame (JNIEnv *env, ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork, jbyteArray currentFrameByteArray);

/**
 * @brief delete the global reference of the currentFrameByteArray and the pointer on its data
 * @param env java environement
 * @param jniBLENetwork the JNIBLENETWORK
 */
void ARNETWORKAL_JNIBLENetwork_ObjectDeleteCurrentFrame (JNIEnv *env, ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork);

/*****************************************
 *
 *             implementation :
 *
 *****************************************/

ARNETWORKAL_JNIBLENETWORK_Object_t *ARNETWORKAL_JNIBLENetwork_ObjectNew (JNIEnv *env, ARNETWORKAL_Manager_t *manager, jobject jContext, eARNETWORKAL_ERROR *error)
{
    /* -- Create a new sender -- */

    /* local declarations */
    ARNETWORKAL_JNIBLENETWORK_Object_t* jniBLENetwork =  NULL;
    eARNETWORKAL_ERROR localError = ARNETWORKAL_OK;

    jclass jBLENetworkCls = NULL;
    jobject jBLENetwork = NULL;

    /* Create the jniBLENetwork */
    jniBLENetwork =  malloc (sizeof (ARNETWORKAL_JNIBLENETWORK_Object_t));

    if (jniBLENetwork == NULL)
    {
        localError = ARNETWORKAL_ERROR_ALLOC;
    }

    /* create the java BLENetwork */
    if (localError == ARNETWORKAL_OK)
    {
        /* get jBLENetwork */
        jBLENetworkCls = (*env)->FindClass(env, "com/parrot/arsdk/arnetworkal/ARNetworkALBLENetwork");
        jmethodID jBLENnetworkMethodConstructor = (*env)->GetMethodID(env, jBLENetworkCls, "<init>", "(JLandroid/content/Context;)V");

        /* create jBLENetwork */
        jBLENetwork = (*env)->NewObject(env, jBLENetworkCls, jBLENnetworkMethodConstructor, (jlong)(intptr_t)jniBLENetwork, jContext);

        /* free jBLENetworkCls */
        (*env)->DeleteLocalRef (env, jBLENetworkCls );

        if (jBLENetwork == NULL)
        {
            localError = ARNETWORKAL_ERROR_ALLOC;
        }
        else
        {
            //store the globalref of the jBLENetwork
            jniBLENetwork->jBLENetwork = (void *) (*env)->NewGlobalRef(env, jBLENetwork);
        }
    }

    /* init the currentFrame use to store the data pop */
    if (localError == ARNETWORKAL_OK)
    {
        jniBLENetwork->currentFrame = NULL;
        jniBLENetwork->manager = manager;
        jniBLENetwork->currentFrameByteArray = NULL;
        jniBLENetwork->onDisconnect = NULL;
        jniBLENetwork->onDisconnectCustomData = NULL;
    }

    /* delete the jniBLENetwork if an error occurred */
    if (localError != ARNETWORKAL_OK)
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORKAL_JNIBLENETWORK_TAG, "error: %s", ARNETWORKAL_Error_ToString (localError));
        ARNETWORKAL_JNIBLENetwork_ObjectDelete (env, &jniBLENetwork);
    }

    /* error return */
    if (error != NULL)
    {
        *error = localError;
    }

    /* cleanup */
    (*env)->DeleteLocalRef (env, jBLENetwork);

    return jniBLENetwork;
}

void ARNETWORKAL_JNIBLENetwork_ObjectDelete (JNIEnv *env, ARNETWORKAL_JNIBLENETWORK_Object_t **jniBLENetwork)
{
    /* -- Delete the jniBLENetwork -- */

    if (jniBLENetwork != NULL)
    {
        if (*jniBLENetwork != NULL)
        {
            /* delete jBLENetwork */
            (*env)->CallVoidMethod(env, (*jniBLENetwork)->jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_DISCONNECT);
            (*env)->DeleteGlobalRef (env, (*jniBLENetwork)->jBLENetwork);
            (*jniBLENetwork)->jBLENetwork = NULL;

            /* free currentFrame*/;
            ARNETWORKAL_JNIBLENetwork_ObjectDeleteCurrentFrame (env, *jniBLENetwork);

            /* free jniBLENetwork */
            free (*jniBLENetwork);
            *jniBLENetwork = NULL;
        }
    }

}

void ARNETWORKAL_JNIBLENetwork_ObjectStoreCurrentFrame (JNIEnv *env, ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork, jbyteArray currentFrameByteArray)
{
    /* -- Store the current frame pop -- */

    /*release the previous currentFrame*/
    ARNETWORKAL_JNIBLENetwork_ObjectDeleteCurrentFrame (env, jniBLENetwork);

    /*store the new currentFrame*/
    jniBLENetwork->currentFrameByteArray = (*env)->NewGlobalRef(env, currentFrameByteArray);
    jniBLENetwork->currentFrame = (uint8_t *)(*env)->GetByteArrayElements(env, jniBLENetwork->currentFrameByteArray, NULL);
}

void ARNETWORKAL_JNIBLENetwork_ObjectDeleteCurrentFrame (JNIEnv *env, ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork)
{
    /* -- Release the current frame pop -- */

    if (jniBLENetwork->currentFrameByteArray != NULL)
    {
        if (jniBLENetwork->currentFrame != NULL)
        {
            (*env)->ReleaseByteArrayElements (env, jniBLENetwork->currentFrameByteArray, (jbyte *)jniBLENetwork->currentFrame, 0);
            jniBLENetwork->currentFrame = NULL;
        }

        (*env)->DeleteGlobalRef (env, jniBLENetwork->currentFrameByteArray);
        jniBLENetwork->currentFrameByteArray = NULL;
    }
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALBLENetwork_nativeGetMediaMTU(JNIEnv *env, jobject obj)
{
    /* get ARNETWORKAL_JNIBLENETWORK_MEDIA_MTU*/

    return ARNETWORKAL_JNIBLENETWORK_MEDIA_MTU;
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALBLENetwork_nativeGetHeaderSize(JNIEnv *env, jobject obj)
{
    /* get ARNETWORKAL_JNIBLENETWORK_HEADER_SIZE*/

    return ARNETWORKAL_JNIBLENETWORK_HEADER_SIZE;
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALBLENetwork_nativeJNIInit(JNIEnv *env, jobject obj)
{
    /* -- initialize the JNI part -- */
    /* load the references of java methods */

    jclass jBLENetworkCls = (*env)->FindClass(env, "com/parrot/arsdk/arnetworkal/ARNetworkALBLENetwork");

    ARNETWORKAL_JNIBLENETWORK_METHOD_CONNECT = (*env)->GetMethodID(env, jBLENetworkCls, "connect", "(Landroid/bluetooth/BluetoothDevice;[I)I");

    ARNETWORKAL_JNIBLENETWORK_METHOD_CANCEL = (*env)->GetMethodID(env, jBLENetworkCls, "cancel", "()V");
    ARNETWORKAL_JNIBLENETWORK_METHOD_DISCONNECT = (*env)->GetMethodID(env, jBLENetworkCls, "disconnect", "()V");
    ARNETWORKAL_JNIBLENETWORK_METHOD_PUSH_FRAME = (*env)->GetMethodID(env, jBLENetworkCls, "pushFrame", "(IIII[B)I");
    ARNETWORKAL_JNIBLENETWORK_METHOD_POP_FRAME = (*env)->GetMethodID(env, jBLENetworkCls, "popFrame", "()Lcom/parrot/arsdk/arnetworkal/ARNetworkALBLENetwork$DataPop;");
    ARNETWORKAL_JNIBLENETWORK_METHOD_RECEIVE = (*env)->GetMethodID(env, jBLENetworkCls, "receive", "()I");
    ARNETWORKAL_JNIBLENETWORK_METHOD_UNLOCK = (*env)->GetMethodID(env, jBLENetworkCls, "unlock", "()V");

    jclass jDataPopCls = (*env)->FindClass(env, "com/parrot/arsdk/arnetworkal/ARNetworkALBLENetwork$DataPop");

    ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_ID = (*env)->GetMethodID(env, jDataPopCls, "getId", "()I");
    ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_DATA = (*env)->GetMethodID(env, jDataPopCls, "getData", "()[B");
    ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_RESULT = (*env)->GetMethodID(env, jDataPopCls, "getResult", "()I");

    /* cleanup */
    (*env)->DeleteLocalRef (env, jBLENetworkCls);
    (*env)->DeleteLocalRef (env, jDataPopCls);
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALBLENetwork_nativeJNIOnDisconect(JNIEnv *env, jobject obj, jlong jniARNetworkALBLENetwork)
{
    /* -- onDisconect callback -- */

    /* local declarations */
    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) jniARNetworkALBLENetwork;

    if ((jniBLENetwork != NULL) && (jniBLENetwork->onDisconnect != NULL))
    {
        jniBLENetwork->onDisconnect (jniBLENetwork->manager, jniBLENetwork->onDisconnectCustomData);
    }
}

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_New (ARNETWORKAL_Manager_t *manager, jobject jContext)
{
    /* -- create a new BLEnetwork -- */

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = NULL;

    /* Check parameters */
    if (manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if (error == ARNETWORKAL_OK)
    {
        /* get the environment */
        if (ARNETWORKAL_JNIManager_VM != NULL)
        {
            getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
        }
        /* if no environment then attach the thread to the virtual machine */
        if (getEnvResult == JNI_EDETACHED)
        {
            ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
            (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
        }
        /* check the environment  */
        if (env == NULL)
        {
            error = ARNETWORKAL_ERROR;
        }
    }

    /* create the jniBLENetwork */
    if (error == ARNETWORKAL_OK)
    {
        jniBLENetwork = ARNETWORKAL_JNIBLENetwork_ObjectNew (env, manager, jContext, &error);
    }

    if (error == ARNETWORKAL_OK)
    {
        manager->senderObject = (void *)jniBLENetwork;
        manager->receiverObject = (void *)jniBLENetwork;
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Delete (ARNETWORKAL_Manager_t *manager)
{
    /* -- delete the BLEnetwork -- */

    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORKAL_JNIBLENETWORK_TAG, " BLENetwork_Delete ");

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /* Check parameters */
    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        /* get the environment */
        if (ARNETWORKAL_JNIManager_VM != NULL)
        {
            getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
        }
        /* if no environment then attach the thread to the virtual machine */
        if (getEnvResult == JNI_EDETACHED)
        {
            ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
            (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
        }
        /* check the environment  */
        if (env == NULL)
        {
            error = ARNETWORKAL_ERROR;
        }
    }

    if(error == ARNETWORKAL_OK)
    {
        /* delete jniBLENetwork */
        ARNETWORKAL_JNIBLENetwork_ObjectDelete (env, (ARNETWORKAL_JNIBLENETWORK_Object_t **) &manager->senderObject);
        manager->receiverObject = NULL;
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return error;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_PushFrame (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    /* -- push a BLE a frame -- */

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, " BLENetwork_PushFrame ");

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;

    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->senderObject;
    jobject jBLENetwork = jniBLENetwork->jBLENetwork;

    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;

    int dataSize = 0;
    jbyteArray dataByteArray = NULL;

    /* get the environment */
    if (ARNETWORKAL_JNIManager_VM != NULL)
    {
        getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
    }
    /* if no environment then attach the thread to the virtual machine */
    if (getEnvResult == JNI_EDETACHED)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
        (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
    }
    /* check the environment  */
    if (env == NULL)
    {
        result = ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /* create dataByteArray */
        dataSize = frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr);
        dataByteArray = (*env)->NewByteArray(env, dataSize);
        (*env)->SetByteArrayRegion(env, dataByteArray, 0, dataSize, (jbyte*)frame->dataPtr);

        /* check frame->size */
        /* first uint8_t is frame type and second uint8_t is sequence number */
        if ((frame->size - offsetof (ARNETWORKAL_Frame_t, dataPtr) + ARNETWORKAL_JNIBLENETWORK_HEADER_SIZE) > ARNETWORKAL_JNIBLENETWORK_MEDIA_MTU)
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /* java BLE push */
        result = (*env)->CallIntMethod(env, jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_PUSH_FRAME, (jint) frame->type, (jint) frame->id, (jint) frame->seq, (jint) dataSize, dataByteArray);
    }

    /* cleanup*/
    /* free dataByteArray */
    (*env)->DeleteLocalRef (env, dataByteArray );


    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return result;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_PopFrame (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Frame_t *frame)
{
    /* -- pop a BLE a frame -- */

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, " BLENetwork_PopFrame ");

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;

    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->receiverObject;
    jobject jBLENetwork = jniBLENetwork->jBLENetwork;

    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;
    jobject jDataPop = NULL;
    jbyteArray currentFrameByteArray = NULL;
    uint8_t *currentFrame = NULL;
    int frameId = 0;

    /* Get the environment */
    if (ARNETWORKAL_JNIManager_VM != NULL)
    {
        getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
    }
    /* if no environment then attach the thread to the virtual machine */
    if (getEnvResult == JNI_EDETACHED)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
        (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
    }
    /* check the environment  */
    if (env == NULL)
    {
        result = ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /* Java BLE pop */
        jDataPop = (*env)->CallObjectMethod (env, jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_POP_FRAME);

        if (jDataPop != NULL)
        {
            result = (*env)->CallIntMethod (env, jDataPop, ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_RESULT);
            frameId = (*env)->CallIntMethod (env, jDataPop, ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_ID);
            currentFrameByteArray = (*env)->CallObjectMethod (env, jDataPop, ARNETWORKAL_JNIBLENETWORK_METHOD_DATA_POP_GET_DATA);

            /* delete jDataPop*/
            (*env)->DeleteLocalRef (env, jDataPop);
        }
        else
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }

        if (currentFrameByteArray != NULL)
        {
            /*store the new currentFrame*/
            ARNETWORKAL_JNIBLENetwork_ObjectStoreCurrentFrame (env, jniBLENetwork, currentFrameByteArray);

            currentFrame = jniBLENetwork->currentFrame;
        }
        else
        {
            result = ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
        }
    }

    if (result == ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /* Get the frame from the buffer */
        /* Get id */
        uint8_t frameIdUInt8 = (uint8_t)frameId;
        memcpy (&(frame->id), &frameIdUInt8, sizeof(uint8_t));

        /* Get type */
        memcpy (&(frame->type), currentFrame, sizeof(uint8_t));
        currentFrame += sizeof(uint8_t);

        /* Get seq */
        memcpy (&(frame->seq), currentFrame, sizeof(uint8_t));
        currentFrame += sizeof(uint8_t);

        /* Get frame size */
        frame->size = (*env)->GetArrayLength(env, jniBLENetwork->currentFrameByteArray) - (2 * sizeof(uint8_t)) + offsetof(ARNETWORKAL_Frame_t, dataPtr);

        /* Get data address */
        frame->dataPtr = currentFrame;
    }

    if (result != ARNETWORKAL_MANAGER_RETURN_DEFAULT)
    {
        /** reset frame */
        frame->type = ARNETWORKAL_FRAME_TYPE_UNINITIALIZED;
        frame->id = 0;
        frame->seq = 0;
        frame->size = 0;
        frame->dataPtr = NULL;
    }

    /*cleaup*/
    (*env)->DeleteLocalRef (env, currentFrameByteArray);

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return result;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_Send(ARNETWORKAL_Manager_t *manager)
{
    /* -- BLE send -- */

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, " BLENetwork_Send ");

    return ARNETWORKAL_MANAGER_RETURN_DEFAULT;
}

eARNETWORKAL_MANAGER_RETURN ARNETWORKAL_JNIBLENetwork_Receive(ARNETWORKAL_Manager_t *manager)
{
    /* -- BLE Receive -- */

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;
    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->receiverObject;
    jobject jBLENetwork = jniBLENetwork->jBLENetwork;
    eARNETWORKAL_MANAGER_RETURN result = ARNETWORKAL_MANAGER_RETURN_DEFAULT;

    /* get the environment */
    if (ARNETWORKAL_JNIManager_VM != NULL)
    {
        getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
    }
    /* if no environment then attach the thread to the virtual machine */
    if (getEnvResult == JNI_EDETACHED)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
        (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
    }
    /* check the environment  */
    if (env != NULL)
    {
        /* java BLE receive */
        result = (*env)->CallIntMethod(env, jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_RECEIVE);
    }
    else
    {
        result = ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return result;
}

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Unlock(ARNETWORKAL_Manager_t *manager)
{
    /* -- BLE unlock all functions locked -- */

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, " BLENetwork_Unlock ");

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;
    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->receiverObject;
    jobject jBLENetwork = jniBLENetwork->jBLENetwork;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /* get the environment */
    if (ARNETWORKAL_JNIManager_VM != NULL)
    {
        getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
    }
    /* if no environment then attach the thread to the virtual machine */
    if (getEnvResult == JNI_EDETACHED)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
        (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
    }
    /* check the environment  */
    if (env != NULL)
    {
        /* java BLE unlock */
        (*env)->CallVoidMethod(env, jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_UNLOCK);
    }
    else
    {
        error = ARNETWORKAL_ERROR;
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_SetOnDisconnectCallback (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_Manager_OnDisconnect_t onDisconnectCallback, void *customData)
{
    /* -- set the OnDisconnect Callback -- */

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;
    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->receiverObject;
    jobject jBLENetwork = jniBLENetwork->jBLENetwork;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /* get the environment */
    if (ARNETWORKAL_JNIManager_VM != NULL)
    {
        getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
    }
    /* if no environment then attach the thread to the virtual machine */
    if (getEnvResult == JNI_EDETACHED)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
        (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
    }
    /* check the environment  */
    if (env != NULL)
    {
        /* java BLE Set OnDisconnectCallback */
        jniBLENetwork->onDisconnect = onDisconnectCallback;
        jniBLENetwork->onDisconnectCustomData = customData;
    }
    else
    {
        error = ARNETWORKAL_ERROR;
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return error;
}

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Connect (ARNETWORKAL_Manager_t *manager, ARNETWORKAL_BLEDevice_t device, int recvTimeoutSec, jintArray notificationIDArray)
{
    /* -- connect the BLE network -- */

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;

    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->senderObject;
    jobject jBLENetwork = jniBLENetwork->jBLENetwork;
    jobject peripheral = (jobject)device;

    eARNETWORKAL_ERROR result = ARNETWORKAL_OK;

    /* get the environment */
    if (ARNETWORKAL_JNIManager_VM != NULL)
    {
        getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
    }
    /* if no environment then attach the thread to the virtual machine */
    if (getEnvResult == JNI_EDETACHED)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
        (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
    }

    if (env != NULL)
    {
        /* java BLE connect */
        result = (*env)->CallIntMethod(env, jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_CONNECT, peripheral, notificationIDArray);
    }
    else
    {
        result = ARNETWORKAL_ERROR;
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return result;
}

eARNETWORKAL_ERROR ARNETWORKAL_JNIBLENetwork_Cancel (ARNETWORKAL_Manager_t *manager)
{
    /* -- cancel the BLEnetwork connection -- */

    ARSAL_PRINT(ARSAL_PRINT_WARNING, ARNETWORKAL_JNIBLENETWORK_TAG, " BLENetwork_Cancel ");

    /* local declarations */
    JNIEnv *env = NULL;
    jint getEnvResult = JNI_OK;
    ARNETWORKAL_JNIBLENETWORK_Object_t *jniBLENetwork = NULL;
    jobject jBLENetwork = NULL;
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    /* Check parameters */
    if(manager == NULL)
    {
        error = ARNETWORKAL_ERROR_BAD_PARAMETER;
    }

    if(error == ARNETWORKAL_OK)
    {
        /* get the environment */
        if (ARNETWORKAL_JNIManager_VM != NULL)
        {
            getEnvResult = (*ARNETWORKAL_JNIManager_VM)->GetEnv(ARNETWORKAL_JNIManager_VM, (void **) &env, JNI_VERSION_1_6);
        }
        /* if no environment then attach the thread to the virtual machine */
        if (getEnvResult == JNI_EDETACHED)
        {
            ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORKAL_JNIBLENETWORK_TAG, "attach the thread to the virtual machine ...");
            (*ARNETWORKAL_JNIManager_VM)->AttachCurrentThread(ARNETWORKAL_JNIManager_VM, &env, NULL);
        }
        /* check the environment  */
        if (env == NULL)
        {
            error = ARNETWORKAL_ERROR;
        }
    }

    if(error == ARNETWORKAL_OK)
    {
        /* cancel jniBLENetwork connection */

        jniBLENetwork = (ARNETWORKAL_JNIBLENETWORK_Object_t *) manager->senderObject;
        jobject jBLENetwork = jniBLENetwork->jBLENetwork;
        (*env)->CallVoidMethod(env, jBLENetwork, ARNETWORKAL_JNIBLENETWORK_METHOD_CANCEL);
    }

    /* if the thread has been attached then detach the thread from the virtual machine */
    if ((getEnvResult == JNI_EDETACHED) && (env != NULL))
    {
        (*ARNETWORKAL_JNIManager_VM)->DetachCurrentThread(ARNETWORKAL_JNIManager_VM);
    }

    return error;
}
