LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := \
    external/selinux/libselinux/include \
    system/core/base/include \
    system/core/init

LOCAL_CFLAGS := -Wall -DANDROID_TARGET=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SRC_FILES := init_smdk4412-common.cpp
LOCAL_MODULE := libinit_smdk4412-common

include $(BUILD_STATIC_LIBRARY)
