#ifndef SKS_ISKYWORTH_TR069_H_
#define SKS_ISKYWORTH_TR069_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <stdint.h>

namespace android {

    class ISkyworthTr069: public IInterface {
    public:
	DECLARE_META_INTERFACE(SkyworthTr069);
	
	virtual status_t send(const void* data,size_t len) = 0;
	virtual status_t recv(const void* data,size_t len) = 0;
    };

    class BnSkyworthTr069: public BnInterface<ISkyworthTr069> {
    public:
	virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);
    };

}

#endif/* SKS_ISKYWORTH_TR069_H_ */

