#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <android/log.h>
#include "SkyworthTr069Service.h"

#define LOG_TAG "[skyworths]SkyworthTr069Service"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR , LOG_TAG , __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO , LOG_TAG , __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG , __VA_ARGS__)

using namespace android;
int main(int argc, char** argv)
{
	LOGI("skyworthtr069server....start");
	sp<ProcessState> proc(ProcessState::self());
	sp<IServiceManager> sm = defaultServiceManager();
	LOGI("skyworthtr069server: sm  %p", sm.get());
	// SkyworthTr069Service::instantiate();
	LOGD(" instantiate> start");
	status_t st = sm->addService(String16(SkyworthTr069_SERVICE_NAME),SkyworthTr069Service::getInstance());
	LOGD("ServiceManager addService ret=%d", st);
	LOGD("instantiate> end");

	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
	LOGI("skyworthtr069server....end");
	return 0;
}


