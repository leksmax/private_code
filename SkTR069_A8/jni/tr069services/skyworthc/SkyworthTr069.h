#ifndef SKC_SKYWORHT_TR069_H_
#define SKC_SKYWORHT_TR069_H_


#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include "../skyworths/ISkyworthTr069.h"
#include "../skyworths/ISkyworthTr069Service.h"

namespace android {
    
    class SkyworthTr069: public BnSkyworthTr069 {
    public:
	static const sp<ISkyworthTr069> getInstance();
	static const sp<ISkyworthTr069Service>& getService();
	virtual ~SkyworthTr069();
	status_t send(const void* data,size_t len);
	status_t recv(const void* data,size_t len);
    protected:
	SkyworthTr069();
    private:
	static Mutex mCreateLock;
    };

}
#endif /* SKC_SKYWORHT_TR069_H_ */

