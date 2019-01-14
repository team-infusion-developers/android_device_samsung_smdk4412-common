LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

LOCAL_SHARED_LIBRARIES := \
        libhardware \
        liblog \
        libcamera_metadata \
        libcutils \
        liblog \
        libutils

LOCAL_VENDOR_MODULE := true
LOCAL_MODULE := camera.smdk4x12

LOCAL_MODULE_TAGS := optional

#include $(BUILD_SHARED_LIBRARY)
