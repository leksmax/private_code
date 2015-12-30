#
# Copyright (C) 2011 iPanel Inc.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_MODULE_TAGS := eng

# This is the target being built.
LOCAL_MODULE:= libskyworthc

SK_ANDROID_FRMWK_INCLUDE := $(ANDROID_BUILD_TOP)/frameworks/base/include

LOCAL_C_INCLUDES := \
	$(SK_ANDROID_FRMWK_INCLUDE)/binder \
	$(SK_ANDROID_FRMWK_INCLUDE)/utils
	
# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libbinder
	
# All of the source files that we will compile.
LOCAL_SRC_FILES += \
	ISkyworthTr069.cpp \
	SkyworthTr069.cpp \
	ISkyworthTr069Service.cpp  \
	sktr069_client.cpp 


    
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

