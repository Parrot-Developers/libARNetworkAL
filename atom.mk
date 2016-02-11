LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libARNetworkAL
LOCAL_DESCRIPTION := ARSDK Network Control Library for Specific OS
LOCAL_CATEGORY_PATH := dragon/libs

LOCAL_MODULE_FILENAME := libarnetworkal.so

LOCAL_LIBRARIES := libARSAL
LOCAL_CONDITIONAL_LIBRARIES := \
	OPTIONAL:libmux \
	OPTIONAL:libpomp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/Includes \
	$(LOCAL_PATH)/Sources

LOCAL_CFLAGS := \
	-DHAVE_CONFIG_H

LOCAL_SRC_FILES := \
	Sources/ARNETWORKAL_Manager.c \
	Sources/Wifi/ARNETWORKAL_WifiNetwork.c \
	Sources/Mux/ARNETWORKAL_MuxNetwork.c \
	gen/Sources/ARNETWORKAL_Error.c

LOCAL_INSTALL_HEADERS := \
	Includes/libARNetworkAL/ARNetworkAL.h:usr/include/libARNetworkAL/ \
	Includes/libARNetworkAL/ARNETWORKAL_Error.h:usr/include/libARNetworkAL/ \
	Includes/libARNetworkAL/ARNETWORKAL_Frame.h:usr/include/libARNetworkAL/ \
	Includes/libARNetworkAL/ARNETWORKAL_Manager.h:usr/include/libARNetworkAL/

ifeq ("$(TARGET_OS)","darwin")
ifneq ("$(TARGET_OS_FLAVOUR)","native")
LOCAL_SRC_FILES += \
	Sources/BLE/ARNETWORKAL_BLENetwork.m
endif
endif

include $(BUILD_LIBRARY)
