LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware liblog libcamera_client libutils libcutils

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera.exynos4

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
