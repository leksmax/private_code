#
# Copyright (C) 2011 iPanel Inc.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng

# This is the target being built.
LOCAL_MODULE:= skyworth.tr069

# Target install path.
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin

# No specia compiler flags.
LOCAL_CFLAGS += -D_cplusplus

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
    liblog \
    libhardware \
    libdl \
    libskyworthc \
	libbinder
	
# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
	SkyworthTr069Service.cpp \
	main_server.cpp\
	net_diagnosis.c

# No static libraries.
LOCAL_STATIC_LIBRARIES := \


# Don't prelink this library.  For more efficient code, you may want
# to add this library to the prelink map and set this to true.
LOCAL_PRELINK_MODULE := false

include $(BUILD_EXECUTABLE)








