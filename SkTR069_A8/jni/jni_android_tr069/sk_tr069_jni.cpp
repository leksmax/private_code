#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include "sk_params.h"

#define SK_LOG_MODULE_TAG "TR069-JNI"
#include "sk_debug.h"
#include "sk_tr069.h"

#define SK_TR069_JNI_REGISTER_CLASSNAME "com/skyworthdigital/tr069/TR069Params"


JavaVM* mVM = NULL;
int m_EnvInThread = 0;
static jclass mClazz = 0;
static jobject mJObj = 0;
static jmethodID get_param_method = 0;
static jmethodID set_param_method = 0;
static jmethodID send_factory_reset_method = 0;
static jmethodID send_upgrade_method = 0;
static char mConfigPath[512] = {0};

extern int test_entry();

static JNIEnv* _get_env()
{
	JNIEnv* env;
	
	if((mVM)->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
	{
		if(mVM->AttachCurrentThread(&env, NULL) != JNI_OK)
		{
			SK_ERR("vm get env error.");
			return NULL;
		}

		m_EnvInThread = 1;
	}
	
    return env;
}

static void _release_env()
{
	if(m_EnvInThread == 1)
	{
		if(mVM->DetachCurrentThread() != JNI_OK)
		{
			SK_ERR("[%s]: vm DetachCurrentThread() failed", __FUNCTION__);
		}
	}

	m_EnvInThread = 0;
}

static void _jni_init(JNIEnv *env, jobject obj)
{
	mClazz = env->FindClass(SK_TR069_JNI_REGISTER_CLASSNAME);
	mJObj = env->NewWeakGlobalRef(obj);
	if(mClazz == 0)
	{
		SK_DBG("can not find class [%s]!", SK_TR069_JNI_REGISTER_CLASSNAME);
	}
	else
	{
		get_param_method = env->GetMethodID(mClazz, "getParam", "(Ljava/lang/String;)Ljava/lang/String;");
		set_param_method = env->GetMethodID(mClazz, "setParam", "(Ljava/lang/String;Ljava/lang/String;)V");
		send_factory_reset_method = env->GetMethodID(mClazz, "sendFactoryReset", "()V");
		send_upgrade_method = env->GetMethodID(mClazz, "sendUpgrade", "(Ljava/lang/String;)V");
	}
}

static void _jni_test(JNIEnv *env, jobject obj)
{
	sk_tr069_start();
}


static void nativeStop(JNIEnv *env, jobject obj)
{
	sk_tr069_shutdown();
}

static void _jni_pingresylt(JNIEnv *env, jobject obj,jstring jpath)
{
	const char* path = NULL;
	path = env->GetStringUTFChars(jpath, 0);
	//sk_tr069_get_pingresult(path);
	env->ReleaseStringUTFChars(jpath, path);
}


static void _jni_traceroute_resylt(JNIEnv *env, jobject obj,jstring jpath)
{
	const char* path = NULL;
	path = env->GetStringUTFChars(jpath, 0);
	//sk_tr069_get_tracerouteresult(path);
	env->ReleaseStringUTFChars(jpath, path);
}

static void _jni_set_config_path(JNIEnv *env, jobject obj, jstring jpath)
{
	const char* path = NULL;
	int config_size = sizeof(mConfigPath);

	path = env->GetStringUTFChars(jpath, 0);
	memset(mConfigPath, 0, config_size);
	snprintf(mConfigPath, config_size, "%str069_model.xml", path);
	env->ReleaseStringUTFChars(jpath, path);
}

void _nativeBandWidthTestResponse()
{
	sk_band_width_test_response_start();
}

static JNINativeMethod methods[] = {
	{"nativeInit", "()V", (void*)_jni_init},
	{"nativeTest", "()V", (void*)_jni_test},
	{"setConfigPath", "(Ljava/lang/String;)V", (void*)_jni_set_config_path},
	{"nativeping", "(Ljava/lang/String;)V", (void*)_jni_pingresylt},
	{"nativeTraceroute", "(Ljava/lang/String;)V", (void*)_jni_traceroute_resylt},
	{"nativeStop", "()V", (void*)nativeStop},
	{"nativeBandWidthTestResponse", "()V", (void*)_nativeBandWidthTestResponse},
};

static int _register_native_methods(JNIEnv* env, const char* className, const JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL)
        return -1;

    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0)
        return -1;

    return 0;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = JNI_ERR;

	mVM = vm;

	if(vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
	{
		return result;
	}

	if(JNI_OK != _register_native_methods(env, SK_TR069_JNI_REGISTER_CLASSNAME, methods, sizeof(methods) / sizeof(methods[0])))
	{
		return result;
	}

	return JNI_VERSION_1_4;
}

int sk_api_params_set(const char *name, const char *value)
{
	int ret = -1;
   	SK_DBG("[sk_tr069_jni] sk_api_params_get begin name:%s,value:%s\n",name,value);
	if(0 != set_param_method)
	{
		JNIEnv* env = _get_env();

		if(env == NULL)
        {
		    SK_ERR("get env fail!!:%s:%d", __FUNCTION__, __LINE__);
            return -1;
        }

		jobject real = env->NewLocalRef(mJObj);
		if(real == NULL)
		{
			SK_ERR("object is null!!:%s:%d", __FUNCTION__, __LINE__);
			_release_env();
			return -1;
		}
		
		jstring j_name = env->NewStringUTF(name);
		jstring j_value = env->NewStringUTF(value);

		env->CallVoidMethod(real, set_param_method, j_name, j_value);

		if(env->ExceptionCheck() != 0)
		{
			SK_DBG("*** Uncaught exception returned from java call!!!");
			env->ExceptionDescribe();
		}

		env->DeleteLocalRef(j_name);
		env->DeleteLocalRef(j_value);
		env->DeleteLocalRef(real);
		_release_env();
		ret = 0;
	}
	
	return ret;
}

int sk_api_params_get(const char *name, char *value, int size)
{
	int ret = -1;
	SK_DBG("[sk_tr069_jni] sk_api_params_get begin name:%s\n",name);
	if(0 != get_param_method)
	{
		const char* val = NULL;
		JNIEnv* env = _get_env();
		
		if(env == NULL)
        {
		    SK_ERR("get env fail!!:%s:%d", __FUNCTION__, __LINE__);
            return -1;
        }

		jobject real = env->NewLocalRef(mJObj);

		if(real == NULL)
		{
			SK_ERR("object is null!!:%s:%d", __FUNCTION__, __LINE__);
			_release_env();
			return -1;
		}
		
		jstring j_name = env->NewStringUTF(name);

		jstring j_value = (jstring)env->CallObjectMethod(real, get_param_method, j_name);

		if(env->ExceptionCheck() != 0)
		{
			SK_DBG("*** Uncaught exception returned from java call!!!");
			env->ExceptionDescribe();
		}

		val = env->GetStringUTFChars(j_value, 0);
		memset(value, 0, size);
		snprintf(value, size, "%s", val);
		env->ReleaseStringUTFChars(j_value, val);
		
		env->DeleteLocalRef(j_name);
		env->DeleteLocalRef(real);
		env->DeleteLocalRef(j_value);
		_release_env();
		ret = 0;
	}
   	SK_DBG("[sk_tr069_jni] sk_api_params_get begin name:%s,value:%s\n",name,value);
	return ret;
}

int sk_factory_reset()
{
	int ret = -1;
	if(0 != send_factory_reset_method)
	{
		JNIEnv* env = _get_env();
		
		if(env == NULL)
        {
		    SK_ERR("get env fail!!:%s:%d", __FUNCTION__, __LINE__);
            return -1;
        }

		jobject real = env->NewLocalRef(mJObj);

		if(real == NULL)
		{
			SK_ERR("object is null!!:%s:%d", __FUNCTION__, __LINE__);
			_release_env();
			return -1;
		}

		env->CallVoidMethod(real, send_factory_reset_method);

		if(env->ExceptionCheck() != 0)
		{
			SK_DBG("*** Uncaught exception returned from java call!!!");
			env->ExceptionDescribe();
		}

		env->DeleteLocalRef(real);
		_release_env();
		ret = 0;
	}

	return ret;
}

int sk_upgrade_start(const char *path)
{
	int ret = -1;
	if(0 != send_upgrade_method)
	{
		JNIEnv* env = _get_env();
		
		if(env == NULL)
        {
		    SK_ERR("get env fail!!:%s:%d", __FUNCTION__, __LINE__);
            return -1;
        }

		jobject real = env->NewLocalRef(mJObj);

		if(real == NULL)
		{
			SK_ERR("object is null!!:%s:%d", __FUNCTION__, __LINE__);
			_release_env();
			return -1;
		}
		
		jstring j_path = env->NewStringUTF(path);

		env->CallVoidMethod(real, send_upgrade_method, j_path);

		if(env->ExceptionCheck() != 0)
		{
			SK_DBG("*** Uncaught exception returned from java call!!!");
			env->ExceptionDescribe();
		}
		
		env->DeleteLocalRef(j_path);
		env->DeleteLocalRef(real);
		_release_env();
		ret = 0;
	}

	return ret;
}

char* sk_get_config_path()
{
	return mConfigPath;
}
