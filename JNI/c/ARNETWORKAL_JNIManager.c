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

    /** Return the JNI version */
    return JNI_VERSION_1_6;
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
    const char *nativeString = (*env)->GetStringUTFChars(env, jaddr, 0);
    eARNETWORKAL_ERROR error = ARNETWORKAL_OK;

    error = ARNETWORKAL_Manager_InitWifiNetwork(manager, nativeString, sendingPort, recvPort, recvTimeoutSec);
    (*env)->ReleaseStringUTFChars( env, jaddr, nativeString );

    return error;
}

/**
 * @brief force timeout on Wifi network
 * When this function is called, all the functions that are blocked on sockets are immediately unblocked.
 * A typical use case is when the ARNETWORK library is closing: The threads must be joined before calling
 * @ref Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeCloseWifiNetwork, but the threads might
 * be waiting for an ARNETWORKAL timeout.
 * @param env reference to the java environment
 * @param obj reference to the object calling this function
 * @param jManagerPtr address of the ARNETWORKAL_Manager_t
 * @param manager pointer on the Manager
 * @return error equal to ARNETWORKAL_OK if the signal was successful otherwise see eARNETWORKAL_ERROR.
 */
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetworkal_ARNetworkALManager_nativeSignalWifiNetwork(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    ARNETWORKAL_Manager_t *manager = (ARNETWORKAL_Manager_t *) (intptr_t) jManagerPtr;
    eARNETWORKAL_ERROR error = ARNETWORKAL_Manager_SignalWifiNetwork (manager);
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
