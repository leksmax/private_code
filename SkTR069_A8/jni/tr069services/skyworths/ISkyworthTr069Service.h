#ifndef SKS_ISKYWORTH_TR069_SERVICE_H_
#define SKS_ISKYWORTH_TR069_SERVICE_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "ISkyworthTr069.h"

namespace android {

#define SkyworthTr069_SERVICE_NAME "skyworth.tr069"

    class ISkyworthTr069Service: public IInterface {
    public:
	DECLARE_META_INTERFACE(SkyworthTr069Service);

	virtual status_t bindClient(sp<ISkyworthTr069> client) = 0;
	virtual status_t send(const void* data,size_t len) = 0;
	virtual status_t recv(const void* data,size_t len) = 0;
    };

    class BnSkyworthTr069Service: public BnInterface<ISkyworthTr069Service> {
    public:
	virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
				    uint32_t flags = 0);
    };

}

#endif /* SKS_ISKYWORTH_TR069_SERVICE_H_ */

