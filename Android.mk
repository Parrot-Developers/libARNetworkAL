LOCAL_PATH := $(call my-dir)

# JNI Wrapper
include $(CLEAR_VARS)

LOCAL_CFLAGS := -g
LOCAL_MODULE := libarnetworkal_android
LOCAL_SRC_FILES := JNI/c/ARNETWORKAL_JNIManager.c JNI/c/ARNETWORKAL_JNIBLENetwork.c
LOCAL_LDLIBS := -llog -lz
LOCAL_SHARED_LIBRARIES := \
	libARNetworkAL \
	libARSAL

include $(BUILD_SHARED_LIBRARY)
