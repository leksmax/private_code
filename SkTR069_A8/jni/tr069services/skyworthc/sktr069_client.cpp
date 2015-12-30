#include "SkyworthTr069.h"
#include "sktr069.h"
#include <stdlib.h>
#include <utils/Log.h>
#include <android/log.h>
using namespace android;
static sk_tr069_callback_t skcb = NULL;

#define LOG_TAG "[skyworthc]CWarper"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR , LOG_TAG , __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO , LOG_TAG , __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG , __VA_ARGS__)


typedef struct bindermsg_t{
    char user[20];
    int cmd;
    op_type type;
    char msg[1024 * 30];
    int len;
}bindermsg;

void sk_send_msg(char *users,int cmd,op_type type,char *msg,int len)
{
	bindermsg bmsg;

	LOGI("sk_send_msg%s:%d", __FUNCTION__, __LINE__);

	if(users == NULL)
		{
			strcpy(bmsg.user,"test01");
		}
	else
		{
			strcpy(bmsg.user,users);
		}
	if(msg == NULL)
		{
			strcpy(bmsg.msg,"msg_test");
		}
	else
		{
			strcpy(bmsg.msg,msg);
		}
	

	bmsg.cmd = cmd;
	bmsg.type = type;
	bmsg.len = len;
LOGI("sk_send_msg%s:%d,bmsg->user:%s", __FUNCTION__, __LINE__,bmsg.user);
LOGI("sk_send_msg%s:%d,bmsg ->msg:%s", __FUNCTION__, __LINE__,bmsg.msg);
LOGI("sk_send_msg%s:%d", __FUNCTION__, __LINE__);
	sk_binder_send(&bmsg,sizeof(bmsg));	    
	
}

int sk_binder_send(const void* data,int len)
{
    LOGD("sk_binder_send...");
    return SkyworthTr069::getInstance()->send(data,len);
}
int sk_binder_set_cb(sk_tr069_callback_t cb)
{
    skcb = cb;
    return 0;
}
int sk_binder_cb(const void* data,int len)
{
    if(skcb)
	(*skcb)(data,len,0);
    else
	LOGE("callback error");
    return 0;
}
