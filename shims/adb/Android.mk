# Copyright 2005 The Android Open Source Project
#
# Android.mk for adb
#

LOCAL_PATH:= $(call my-dir)

include $(LOCAL_PATH)/adb/../platform_tools_tool_version.mk

adb_target_sanitize :=

ADB_COMMON_INCLUDES := \
    $(LOCAL_PATH) \
    system/core/adb \
    system/core/adb/daemon

ADB_COMMON_CFLAGS := \
    -Wall -Wextra -Werror \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
    -Wvla \
    -DADB_VERSION="\"$(tool_version)\"" \

ADB_COMMON_posix_CFLAGS := \
    -Wexit-time-destructors \
    -Wthread-safety \

ADB_COMMON_linux_CFLAGS := \
    $(ADB_COMMON_posix_CFLAGS) \

# libadb
# =========================================================

# Much of adb is duplicated in bootable/recovery/minadb and fastboot. Changes
# made to adb rarely get ported to the other two, so the trees have diverged a
# bit. We'd like to stop this because it is a maintenance nightmare, but the
# divergence makes this difficult to do all at once. For now, we will start
# small by moving common files into a static library. Hopefully some day we can
# get enough of adb in here that we no longer need minadb. https://b/17626262
LIBADB_SRC_FILES := \
    adb/adb.cpp \
    adb/adb_io.cpp \
    adb/adb_listeners.cpp \
    adb/adb_trace.cpp \
    adb/adb_utils.cpp \
    adb/fdevent.cpp \
    adb/sockets.cpp \
    adb/socket_spec.cpp \
    adb/sysdeps/errno.cpp \
    adb/transport.cpp \
    adb/transport_local.cpp \
    adb/transport_usb.cpp

LIBADB_CFLAGS := \
    $(ADB_COMMON_CFLAGS) \
    -fvisibility=hidden

LIBADB_linux_CFLAGS := \
    $(ADB_COMMON_linux_CFLAGS) \

LIBADB_linux_SRC_FILES := \
    adb/sysdeps_unix.cpp \
    adb/sysdeps/posix/network.cpp \
    adb/client/usb_dispatch.cpp \
    adb/client/usb_libusb.cpp \
    adb/client/usb_linux.cpp

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_MODULE := libadbd_usb_legacy
LOCAL_CFLAGS := $(LIBADB_CFLAGS) -DADB_HOST=0
LOCAL_SRC_FILES := daemon/usb.cpp

LOCAL_C_INCLUDES := \
    $(ADB_COMMON_INCLUDES)

LOCAL_CFLAGS += -DLEGACY_ADB_INTERFACE

LOCAL_SANITIZE := $(adb_target_sanitize)

# Even though we're building a static library (and thus there's no link step for
# this to take effect), this adds the includes to our path.
LOCAL_STATIC_LIBRARIES := libcrypto_utils libcrypto libbase

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_MODULE := libadbd_legacy
LOCAL_CFLAGS := $(LIBADB_CFLAGS) -DADB_HOST=0
LOCAL_SRC_FILES := \
    $(LIBADB_SRC_FILES) \
    adb/adbd_auth.cpp \
    adb/jdwp_service.cpp \
    adb/sysdeps/posix/network.cpp \

LOCAL_C_INCLUDES := \
    $(ADB_COMMON_INCLUDES)

LOCAL_SANITIZE := $(adb_target_sanitize)

# Even though we're building a static library (and thus there's no link step for
# this to take effect), this adds the includes to our path.
LOCAL_STATIC_LIBRARIES := libcrypto_utils libcrypto libqemu_pipe libbase

LOCAL_WHOLE_STATIC_LIBRARIES := libadbd_usb_legacy

include $(BUILD_STATIC_LIBRARY)

# adbd device daemon
# =========================================================

include $(CLEAR_VARS)

LOCAL_CLANG := true

LOCAL_SRC_FILES := \
    daemon/main.cpp \
    adb/daemon/mdns.cpp \
    adb/services.cpp \
    adb/file_sync_service.cpp \
    adb/framebuffer_service.cpp \
    adb/remount_service.cpp \
    adb/set_verity_enable_state_service.cpp \
    adb/shell_service.cpp \
    adb/shell_service_protocol.cpp \

LOCAL_C_INCLUDES := \
    $(ADB_COMMON_INCLUDES)

LOCAL_CFLAGS := \
    $(ADB_COMMON_CFLAGS) \
    $(ADB_COMMON_linux_CFLAGS) \
    -DADB_HOST=0 \
    -D_GNU_SOURCE \
    -Wno-deprecated-declarations \

LOCAL_CFLAGS += -DALLOW_ADBD_NO_AUTH=$(if $(filter userdebug eng,$(TARGET_BUILD_VARIANT)),1,0)

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DALLOW_ADBD_DISABLE_VERITY=1
LOCAL_CFLAGS += -DALLOW_ADBD_ROOT=1
endif

ifeq ($(TARGET_USES_LEGACY_ADB_INTERFACE),true)
LOCAL_CFLAGS += -DLEGACY_ADB_INTERFACE
endif

LOCAL_MODULE := adbd_legacy

LOCAL_VENDOR_MODULE := true

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_SANITIZE := $(adb_target_sanitize)
LOCAL_STRIP_MODULE := keep_symbols
LOCAL_STATIC_LIBRARIES := \
    libadbd_legacy \
    libavb_user \
    libbase \
    libqemu_pipe \
    libbootloader_message \
    libfs_mgr \
    libfec \
    libfec_rs \
    libselinux \
    liblog \
    libext4_utils \
    libsquashfs_utils \
    libcutils \
    libbase \
    libcrypto_utils \
    libcrypto \
    libminijail \
    libmdnssd \
    libdebuggerd_handler \

include $(BUILD_EXECUTABLE)
