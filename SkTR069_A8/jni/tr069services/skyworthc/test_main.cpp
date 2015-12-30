#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sktr069.h"

using namespace android;

extern "C" void * tro69_test(void*a);
static pthread_t th = NULL;

typedef enum{ 
GET_PARAM, // TM ---> IPTV ??¨¨?2?¨ºy 
SET_PARAM, // TM ---> IPTV ¨¦¨¨??2?¨ºy 
REBOOT, // TM ---> IPTV 
FACTORYRESET, // TM ---> IPTV 
UPDATE, // TM ---> IPTV 
VALUE_CHANGE_REPORT, // TM <--- IPTV 
START, // TM <--- IPTV 
HEART_BEAT, // TM <---> IPTV 
GET_PARAM_REPLY, // TM <--- IPTV 
SET_PARAM_REPLY, // TM <--- IPTV 
REBOOT_REPLY, // TM <--- IPTV 
FACTORYRESET_REPLY, // TM <--- IPTV 
UPDATE_REPLY, // TM <--- IPTV 
VALUE_CHANGE_REPORT_REPLY, // TM <--- IPTV 
START_FINISH, // TM ---> IPTV 
HEAT_BEAT_REPLY, // TM <---> IPTV 
AUTHENTICATEINFOR,
RATELOSS,
MULTIFAILINFO,
VODFAILINFO,
START_PING, //TM<-->IPTV
START_TRACEROUTE, //TM<->IPTV
START_TCPDUMP,//TM<->IPTV
}op_type;


typedef struct bindermsg_t{
    char user[20];
    int cmd;
    op_type type;
    char msg[1024];
    int len;
}bindermsg;

static void testcb(const void*data,int arg1,int arg2)
{
    LOGD("test cb size:%d",arg1);
    bindermsg* p =(bindermsg*)malloc(arg1);
    memcpy(p,data,arg1);
//    LOGE("test cb %s,%s",p->user,p->msg);
	
	LOGD("p.user:%s\n",p->user);
	LOGD("p.cmd:%d\n",p->cmd);
	LOGD("p.op_type:%d\n",p->type);
	LOGD("p.msg:%s\n",p->msg);
	LOGD("p.len:%d\n",p->len);
    delete(p);
    return;
}
void*tro69_test(void*a) {
	LOGD("test tro069 start...");
	bindermsg msg;
	strcpy(msg.user,"test client");
	strcpy(msg.msg,"long long age!");

	msg.cmd = 0;
	msg.type = START;//start
	msg.len = strlen(msg.msg);

	while(true)
	{
	usleep(5*1000 * 1000);
	sk_binder_send(&msg,sizeof(msg));	    
	}
	LOGD("test tro069 end.");
	return NULL;
}

int main() {
	LOGD("test start...");
	sk_binder_set_cb(&testcb);
	if (pthread_create(&th, NULL, tro69_test, NULL) != 0) {
		printf("create test thread failed.");
		return -1;
	}
	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
	LOGD("test end.");
	return 0;
}

