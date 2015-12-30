#ifndef _SK_DEBUG_H_
#define _SK_DEBUG_H_
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SK_ITV_LOG_OPEN
#define SK_ITV_LOG_OPEN 1
#endif

#if SK_ITV_LOG_OPEN

#ifndef SK_LOG_MODULE_TAG
#define SK_LOG_MODULE_TAG "SKYIPTV"
#endif
#include <android/log.h>

// verbose
#define SK_VERBOSE(...) __android_log_print(ANDROID_LOG_VERBOSE, SK_LOG_MODULE_TAG, __VA_ARGS__)
// debug
#define SK_DBG(...) __android_log_print(ANDROID_LOG_DEBUG, SK_LOG_MODULE_TAG, __VA_ARGS__)
// informational
#define SK_INFO(...) __android_log_print(ANDROID_LOG_INFO, SK_LOG_MODULE_TAG, __VA_ARGS__)
// warning
#define SK_WARN(...) __android_log_print(ANDROID_LOG_WARN, SK_LOG_MODULE_TAG, __VA_ARGS__)
// err
#define SK_ERR(...) __android_log_print(ANDROID_LOG_ERROR, SK_LOG_MODULE_TAG, __VA_ARGS__)

#else

#define SK_VERBOSE(...)
#define SK_DBG(...)
#define SK_INFO(...)
#define SK_WARN(...)
#define SK_ERR(...)

#endif
#ifdef __cplusplus
}
#endif

#endif