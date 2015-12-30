#ifndef EVCPE_HEADER_H_
#define EVCPE_HEADER_H_

#include "sk_tr069.h"
#define TR069_ANDROID
#define HD_PLATFORM

#ifdef TR069_ANDROID
#include <jni.h>
#include <android/log.h>

#define LOG_TAG    "JNILOG"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO , LOG_TAG , __VA_ARGS__)
#define  LOGD(...)   
#define SK_LOG_PATH  "/data"           //日志文件保存目录

#else

#define LOGI       printf
#define SK_LOG_PATH  "/data"                     //日志文件保存目录liangzhen 

#endif

#endif//EVCPE_HEADER_H_