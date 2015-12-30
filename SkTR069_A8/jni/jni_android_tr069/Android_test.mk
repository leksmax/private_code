LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_SRC_FILES := $(call all-c-files-under, tr69_code)
#LOCAL_SRC_FILES += tr69_code/sk_param_porting.cpp sk_tr069_jni.cpp 
LOCAL_SRC_FILES = test.cpp sk_tr069_jni.cpp 

LOCAL_MODULE:= libsktr069

#LOCAL_EXPORT_LDLIBS :=
LOCAL_CFLAGS += -DHD_PLATFORM -DTR069_ANDROID
LOCAL_CFLAGS += -DSK_SH_CTC
#LOCAL_LIBS := -llog
#LOCAL_LDLIBS += -llog
#LOCAL_STATIC_LIBRARIES := libevent
#LOCAL_STATIC_LIBRARIES := sktr069servicemodel
#LOCAL_SHARED_LIBRARIES := liblog libdl libevent libutils libbinder libskcmdinterface
#LOCAL_STATIC_LIBRARIES :=  #sktr069servicemodel
#LOCAL_SHARED_LIBRARIES := liblog libdl  libutils  libevent

LOCAL_SHARED_LIBRARIES := liblog 


SK_ANDROID_FRMWK_INCLUDE := $(ANDROID_BUILD_TOP)/frameworks/base/include

#LOCAL_C_INCLUDES :=  $(JNI_H_INCLUDE) \
#	$(SK_ANDROID_FRMWK_INCLUDE)/binder \
#	$(SK_ANDROID_FRMWK_INCLUDE)/utils \
#	$(LOCAL_PATH)/tr69_code \
#	vendors/skyworth/libs \
#	vendors/skyworth/libs/skevent/include \
#	vendors/skyworth/libs/skevent/include/event2 \
#	vendors/skyworth/libs/sktr069server
	

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)

