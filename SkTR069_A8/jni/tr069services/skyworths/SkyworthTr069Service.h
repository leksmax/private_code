#ifndef SKS_SKYWORTH_TR069_SERVICE_H_
#define SKS_SKYWORTH_TR069_SERVICE_H_

#include <utils/KeyedVector.h>

#include "ISkyworthTr069Service.h"
#include "ISkyworthTr069.h"
#include "Tr069_def.h"
namespace android {

    class SkyworthTr069Service: public BnSkyworthTr069Service {

    public:
	static SkyworthTr069Service* getInstance();
	SkyworthTr069Service();
	virtual ~SkyworthTr069Service();
	status_t bindClient(sp<ISkyworthTr069> client);
	status_t send(const void* data,size_t len);
	status_t recv(const void* data,size_t len);
	status_t qos_start(char* epg_url);

    private:
	// mutable Mutex mLock;
	// SortedVector<wp<Client> > mClients;
	sp<ISkyworthTr069> mClient;
   	status_t send_msg_to_client(char * user, int cmd, op_type type, char * msg, int len);
	void traceroute_task(void * args);
 };

}

# ifdef __cplusplus
extern "C"
{
#endif


# ifdef __cplusplus
}
#endif

#endif /* SKS_SKYWORTH_TR069_SERVICE_H_ */
