#ifndef EVCPE_HEADER_H_
#define EVCPE_HEADER_H_
#include <jni.h>
#include <android/log.h>


//#define TR069_ANDROID


#ifdef TR069_ANDROID
#define  LOG_TAG    "JNILOG"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#else

#define  LOGI       printf



#endif




#endif
