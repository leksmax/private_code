
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <assert.h>

#include "sktr069.h"

//#define JNIREG_CLASS "com/skyworth/android/tr069/SkTr069Receiver"//指定要注册的类
#define JNIREG_CLASS  "com/skyworthdigital/tr069/TR069Params"

 int NetChange = 1;
jint sk_jni_tr069_init(JNIEnv *env, jobject obj)
{	
	int ret = 0;	
	if(NetChange)
	{
		NetChange = 0;
		sk_send_msg(NULL,77,START,NULL,0);
	}
		
//	LOGI("call jni func:%s:%d, ret:%d", __FUNCTION__, __LINE__, ret);	
	return ret;
}

/**
* Table of methods associated with a single class.
*/
static JNINativeMethod gMethods[] = {
	{"sk_tr069_in_init", "()I", (void*)sk_jni_tr069_init},	
};

/*
* Register several native methods for one class.
*/
static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz;
	clazz = (*env)->FindClass(env, className);
	if (clazz == NULL) {
		return JNI_FALSE;
	}
	if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
		return JNI_FALSE;
	}

	return JNI_TRUE;
}


/*
* Register native methods for all classes we know about.
*/
static int registerNatives(JNIEnv* env)
{
	if (!registerNativeMethods(env, JNIREG_CLASS, gMethods, 
                                 sizeof(gMethods) / sizeof(gMethods[0])))
		return JNI_FALSE;

	return JNI_TRUE;
}

/*
* Set some test stuff up.
*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		return -1;
	}
	assert(env != NULL);

	if (!registerNatives(env)) {//注册
		return -1;
	}
	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

	return result;
}

