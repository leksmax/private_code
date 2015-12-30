#define LOG_TAG "[skyworthc]ISkyworthTr069Service"
#include <utils/Log.h>
#include "../skyworths/ISkyworthTr069Service.h"

namespace android {

    enum {
	BIND = IBinder::FIRST_CALL_TRANSACTION,
	SEND,
	RECV,
    };

    class BpSkyworthTr069Service: public BpInterface<ISkyworthTr069Service> {
    public:
	BpSkyworthTr069Service(const sp<IBinder>& impl) :
	    BpInterface<ISkyworthTr069Service> (impl) {
	}

	status_t bindClient(sp<ISkyworthTr069> client) {
	    Parcel data, reply;
	    data.writeInterfaceToken(ISkyworthTr069Service::getInterfaceDescriptor());
	    data.writeStrongBinder(client->asBinder());
	    remote()->transact(BIND, data, &reply);
	    return reply.readInt32();
	}
	status_t send(const void* d,size_t len) {
	    // 暂不用到此方法
	    Parcel data, reply;
	    data.writeInterfaceToken(ISkyworthTr069Service::getInterfaceDescriptor());
	    remote()->transact(SEND, data, &reply);
	    return reply.readInt32();
	}
	status_t recv(const void* d,size_t len) {
	    Parcel data, reply;
	    data.writeInterfaceToken(ISkyworthTr069Service::getInterfaceDescriptor());
	    data.writeInt32(len);
	    data.write(d,len);
	    remote()->transact(RECV, data, &reply);
	    return reply.readInt32();
	}
    };

    IMPLEMENT_META_INTERFACE(SkyworthTr069Service, "android.television.ISkyworthTr069Service");

// ----------------------------------------------------------------------

    status_t BnSkyworthTr069Service::onTransact(uint32_t code, const Parcel& data,
						Parcel* reply, uint32_t flags) {
	switch (code) {
	case BIND: {
	    CHECK_INTERFACE(ISkyworthTr069Service, data, reply);
	    sp<ISkyworthTr069> lis = interface_cast<ISkyworthTr069> (
		data.readStrongBinder());
	    reply->writeInt32(bindClient(lis));
	    return NO_ERROR;
	}
	case SEND: {
	    CHECK_INTERFACE(ISkyworthTr069Service, data, reply);
	    // reply->writeInt32(send());
	    return NO_ERROR;
	}
	case RECV: {
	    CHECK_INTERFACE(ISkyworthTr069Service, data, reply);
	    size_t l = data.readInt32();
	    void *p = malloc(l);
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
