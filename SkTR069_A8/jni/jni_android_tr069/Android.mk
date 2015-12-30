LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-c-files-under, tr69_code)
LOCAL_SRC_FILES += tr69_code/sk_param_porting.cpp sk_tr069_jni.cpp 
#LOCAL_SRC_FILES +=tr69_code/sk_param_porting.cpp test.cpp

LOCAL_MODULE:= libsktr069

#LOCAL_EXPORT_LDLIBS :=
LOCAL_CFLAGS += -DHD_PLATFORM -DTR069_ANDROID
#LOCAL_LIBS := -llog
#LOCAL_LDLIBS += -llog
#LOCAL_STATIC_LIBRARIES := libevent
#LOCAL_STATIC_LIBRARIES := sktr069servicemodel
#LOCAL_SHARED_LIBRARIES := liblog libdl libevent libutils libbinder libskcmdinterface
#LOCAL_STATIC_LIBRARIES :=  #sktr069servicemodel
LOCAL_SHARED_LIBRARIES := liblog libdl  libutils libcutils libevent libskyworthc

SK_ANDROID_FRMWK_INCLUDE := $(ANDROID_BUILD_TOP)/frameworks/base/include

LOCAL_C_INCLUDES := \
	$(SK_ANDROID_FRMWK_INCLUDE)/binder \
	$(SK_ANDROID_FRMWK_INCLUDE)/utils \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/event2 \
	$(LOCAL_PATH)/tr69_code
	

LOCAL_MODULE_TAGS := eng
LOCAL_PRELINK_MODULE := false
LOCAL_CERTIFICATE := platform

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)

