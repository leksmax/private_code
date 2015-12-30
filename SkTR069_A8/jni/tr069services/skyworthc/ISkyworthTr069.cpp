#include <utils/Log.h>
#include <android/log.h>

#define LOG_TAG "[skyworthc]ISkyworthTr069"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO , LOG_TAG , __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG , __VA_ARGS__)

#include "../skyworths/ISkyworthTr069.h"

namespace android {

    enum {
	SEND = IBinder::FIRST_CALL_TRANSACTION,//
	RECV,
    };


    class BpSkyworthTr069: public BpInterface<ISkyworthTr069> {
    public:

	BpSkyworthTr069(const sp<IBinder>& impl) :
	    BpInterface<ISkyworthTr069> (impl) {
	}

	
	status_t send(const void* d, size_t len)
	    {
		// 该方法不会被用到
	    Parcel data, reply;
	    data.writeInterfaceToken(ISkyworthTr069::getInterfaceDescriptor());
	    // data.write(data,len);
	    remote()->transact(SEND, data, &reply);
	    return reply.readInt32();
	}
	status_t recv(const void* d,size_t len)
	    {
	    Parcel data, reply;
	    data.writeInterfaceToken(ISkyworthTr069::getInterfaceDescriptor());
	    data.writeInt32(len);
	    data.write(d,len);
	    remote()->transact(RECV, data, &reply);
	    return reply.readInt32();
	}

    };
    IMPLEMENT_META_INTERFACE(SkyworthTr069, "android.television.ISkyworthTr069");

// ----------------------------------------------------------------------

    status_t BnSkyworthTr069::onTransact(uint32_t code, const Parcel& data,
					 Parcel* reply, uint32_t flags) {
	switch (code) {
	case SEND: {
	    CHECK_INTERFACE(ISkyworthTr069, data, reply);
	    // reply->writeInt32(send(data.read(),data.readInt32()));
	    LOGD("idle function!");
	    return NO_ERROR;
	}
	case RECV: {
	    CHECK_INTERFACE(ISkyworthTr069, data, reply);
	    size_t l = data.readInt32();
	    void* p = malloc(l);
	    // reply->writeInt32(recv(data.read(),data.readInt32()));
	    data.read(p,l);
	    reply->writeInt32(recv(p,l));
	    delete(p);
	    return NO_ERROR;
	}
	default:
	    return BBinder::onTransact(code, data, reply, flags);
	}
    }
}
