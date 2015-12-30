LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
 
LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES :=  jpcap

LOCAL_SRC_FILES := $(call all-java-files-under, src) $(call all-renderscript-files-under, src)
 
LOCAL_PACKAGE_NAME := SkTR069
LOCAL_CERTIFICATE := platform

 
include $(BUILD_PACKAGE)

include $(CLEAR_VARS) 

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := jpcap:libs/jpcap.jar
  
include $(BUILD_MULTI_PREBUILT)
