/**
 *  @file ARNETWORK_OSSPECIFIC_JNIManager.c
 *  @brief JNI between the ARNETWORK_Manager.h and ARNETWORK_OSSPECIFIC_Manager.java
 *  @date 04/29/2013
 *  @author frederic.dhaeyer@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <jni.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Error.h>
#include <libARNetwork_OSSpecific/ARNETWORK_OSSPECIFIC_Manager.h>

/**
 *  @brief data sent to the callback
**/
typedef struct
{
    jobject jManager; 	/**< manager sent the data */
    jobject jARData; 	/**< java native data*/
}ARNETWORK_OSSPECIFIC_JNIManager_CallbackData_t;

/*****************************************
 * 
 *             private header:
 *
******************************************/

#define ARNETWORK_OSSPECIFIC_JNIMANGER_TAG "ARNETWORK_OSSPECIFIC_JNIManager" /** tag used by the print of the file */

/**
 *  @brief call back use when the data are sent or have a timeout 
 *  @param[in] IOBufferID identifier of the IoBuffer is calling back
 *  @param[in] dataPtr pointer on the data
 *  @param[in] customData pointer on a custom data
 *  @param[in] status indicating the reason of the callback. eARNETWORK_MANAGER_CALLBACK_STATUS type
 *  @return eARNETWORK_MANAGER_CALLBACK_RETURN
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
**/
//eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_OSSPECIFIC_JNIManger_Callback_t(int IOBufferID,  uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

/*****************************************
 * 
 *             implementation :
 *
******************************************/


JavaVM* gARNETWORK_OSSPECIFIC_JNIManager_VM = NULL; /** reference to the java virtual machine */

/**
 *  @brief save the reference to the java virtual machine
 *  @note this function is automatically call on the JNI startup
 *  @param[in] VM reference to the java virtual machine
 *  @param[in] reserved data reserved
 *  @return JNI version
**/
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *VM, void *reserved)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_OSSPECIFIC_JNIMANGER_TAG, "Library has been loaded");
    
    /** Saving the reference to the java virtual machine */
    gARNETWORK_OSSPECIFIC_JNIManager_VM = VM;
    
    /** Return the JNI version */
    return JNI_VERSION_1_6;
}

/**
 *  @brief Create a new manager
 *  @warning This function allocate memory
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @return Pointer on the ARNETWORK_OSSPECIFIC_Manager_t.
 *  @note This creator adds for all output, one other IOBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer arrays.
 *  @warning The identifiers of the IoBuffer should not exceed the value 128.
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkOSSpecificManager_nativeDelete()
 * 
**/
JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkOSSpecificManager_nativeNew(JNIEnv *env, jobject obj)
{
    /** -- Create a new manager -- */
    
    /** local declarations */
    ARNETWORK_OSSPECIFIC_Manager_t *manager = NULL;
    eARNETWORK_OSSPECIFIC_ERROR error = ARNETWORK_OSSPECIFIC_OK;
            
    manager = ARNETWORK_Manager_New(&error);
    
    /** print error */
    if(error != ARNETWORK_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_JNIMANGER_TAG, " error: %d occurred \n", error);
    }
    
    return (long) manager;
}

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManager adress of the ARNETWORK_OSSPECIFIC_Manager_t
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkOSSpecificManager_nativeNew()
**/
JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkOSSpecificManager_nativeDelete(JNIEnv *env, jobject obj, jlong jManager)
{
    /** -- Delete the Manager -- */
    
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManager;
    ARNETWORK_Manager_Delete(&managerPtr);
}

/**
 *  @brief initialize UDP sockets of sending and receiving the data.
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManager adress of the ARNETWORK_OSSPECIFIC_Manager_t
 *  @param[in] jaddr address of connection at which the data will be sent.
 *  @param[in] sendingPort port on which the data will be sent.
 *  @param[in] recvPort port on which the data will be received.
 *  @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function ARNETWORK_Receiver_Read().
 *  @return error equal to ARNETWORK_OK if the Bind if successful otherwise see eARNETWORK_ERROR.
**/
//JNIEXPORT jint JNICALL
//Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeSocketsInit(JNIEnv *env, jobject obj, jlong jManager, jstring jaddr, jint sendingPort, jint recvPort, jint recvTimeoutSec)
//{
//    /** -- initialize UDP sockets of sending and receiving the data. -- */
//
//    /** local declarations */
//    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManager;
//    const char *nativeString = (*env)->GetStringUTFChars(env, jaddr, 0);
//    eARNETWORK_ERROR error = ARNETWORK_OK;
//
//    error = ARNETWORK_Manager_SocketsInit(managerPtr, nativeString, sendingPort, recvPort, recvTimeoutSec);
//    (*env)->ReleaseStringUTFChars( env, jaddr, nativeString );
//
//    return error;
//}
