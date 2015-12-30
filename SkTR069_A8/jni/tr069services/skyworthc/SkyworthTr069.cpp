#include <utils/Log.h>
#include <android/log.h>
#include "SkyworthTr069.h"
#include "sktr069.h"
#include <binder/IServiceManager.h>

#include "../skyworths/ISkyworthTr069.h"
#include "../skyworths/ISkyworthTr069Service.h"

#define LOG_TAG "[skyworthc]SkyworthTr069"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR , LOG_TAG , __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO , LOG_TAG , __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG , __VA_ARGS__)

using namespace android;

Mutex SkyworthTr069::mCreateLock;
sp<ISkyworthTr069> instance;
sp<ISkyworthTr069Service> mSkyworthTr069Service;

const sp<ISkyworthTr069> SkyworthTr069::getInstance() {
    Mutex::Autolock _l(SkyworthTr069::mCreateLock);
    
    if(NULL == instance.get())
    {
	LOGD("create new tr069 client");
	instance = new SkyworthTr069();
	const sp<ISkyworthTr069Service> service = getService();
	if (service != NULL) {
	    // service->asBinder()->linkToDeath(instance);
	    service->bindClient(instance);
	}else{
	    LOGE("getSkyworthTr069Service fail");   
	}
    }
    LOGD("get tr069 client");
    return instance;
}

SkyworthTr069::SkyworthTr069() {
}

SkyworthTr069::~SkyworthTr069() {
}

const sp<ISkyworthTr069Service>& SkyworthTr069::getService() {
    if (mSkyworthTr069Service.get() == 0) {
	LOGD("find skyworth tr069 service");
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder;
	do {
	    binder = sm->getService(String16(SkyworthTr069_SERVICE_NAME));
	    if (binder != 0)
		break;
	    usleep(500000); // 0.5 s
	} while (true);
	mSkyworthTr069Service = interface_cast<ISkyworthTr069Service> (binder);
		if(mSkyworthTr069Service == 0)
		{
			LOGE("no mSkyworthTr069Service!?");
		}
    }
    LOGD("get skyworth tr069 service");
    return mSkyworthTr069Service;
}
status_t SkyworthTr069::send(const void* data,size_t len)
{
    LOGD("call send");
    sp<ISkyworthTr069Service> service = getService();
    return service->recv(data,len);
}
status_t SkyworthTr069::recv(const void* data,size_t len)
{
    LOGD("recv message");
    sk_binder_cb(data,len);
    return NO_ERROR;
}
