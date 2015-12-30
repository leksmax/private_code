#include <utils/Log.h>
#include <android/log.h>
#include <pthread.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "SkyworthTr069Service.h"
#include "sk_param_client.h"
#include "sk_tr069.h"
#include "ping.c"
#include <sys/reboot.h>
int flag = 1;
int f = 1;


#define LOG_TAG "[skyworths]SkyworthTr069Service"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR , LOG_TAG , __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO , LOG_TAG , __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG , __VA_ARGS__)
namespace android {
	typedef struct kill_tcpdump_param_s{
		int pid;
		int timeout;
	}kill_tcpdump_param_t;
		
    static SkyworthTr069Service instance;
    SkyworthTr069Service* SkyworthTr069Service::getInstance() {
	return &instance;
    }

    SkyworthTr069Service::SkyworthTr069Service() {
	LOGD(" SkyworthTr069Service> construct");
	mClient = NULL;
    }

    SkyworthTr069Service::~SkyworthTr069Service() {
	LOGD("SkyworthTr069Service destroyed,never destroy normally");
    }

    status_t SkyworthTr069Service::bindClient(sp<ISkyworthTr069> client) {
	LOGD("bind new client");
	mClient = client;
	return NO_ERROR;
    }
    status_t SkyworthTr069Service::send(const void* data,size_t len)
    {
	LOGD("service send");
	if(mClient != NULL)
	{
	    return mClient->recv(data,len);
	}
	LOGE("there are no client!");
	return NO_INIT;
    }
	status_t SkyworthTr069Service::qos_start(char* epg_url)
    {
	 LOGD("call sk_jni_qosmon_epg_server");

    char *url = epg_url;

    if(url == NULL)
    {
	return -1;
    }
    LOGD("call tr069_qosmon_epg_server func %s",url);

    LOGD("support qos!");
    FILE *fp;
    //:TODO
    char para[] ={
    	"SERVICEUSER=%s\r\nPPPOEACCOUNT=%s\r\nEPGSERVER=%s\r\nOUI=990104\r\nSN=%s\r\nDOMAIN=%s\r\nHWVERSION=%s\r\nSWVERSION=%s\r\nDIALTYPE=%s\r\nMANUFACTURER=skyworth\r\nDHCPACCOUNT=%s\r\n"};
    char sn[33]={
    	0};
    char SERVICEUSER[33]={
    	0};
    char PPPOEACCOUNT[33]={
    	0};
    char DOMAIN[100]={
    	0};
    char HWVERSION[33]={
    	0};
    char SWVERSION[33]={
    	0};
    char DIALTYPE[33]={
    	0};
    char DHCPACCOUNT[100]={
    	0};

    char tmp_buff[1024] = {
    	0};
    char *p,epg[100];
    //read param ->sn
    /*
    sk_api_params_get("skyworth.params.sys.sn",sn, sizeof(sn));
    sk_api_params_get("skyworth.params.itv.username",SERVICEUSER, sizeof(SERVICEUSER));
    sk_api_params_get("skyworth.params.net.pppoeusr",PPPOEACCOUNT, sizeof(PPPOEACCOUNT));
    sk_api_params_get("skyworth.params.itv.homepage",DOMAIN, sizeof(DOMAIN));
    sk_api_params_get("skyworth.params.sys.hardware_version",HWVERSION, sizeof(HWVERSION));
    sk_api_params_get("skyworth.params.sys.software_version",SWVERSION, sizeof(SWVERSION));
    sk_api_params_get("skyworth.params.net.connectmode",DIALTYPE, sizeof(DIALTYPE));
    sk_api_params_get("skyworth.params.net.dhcpusr",DHCPACCOUNT, sizeof(DHCPACCOUNT));
	*/

    if(!strcmp(DIALTYPE,"lan"))
    {


    	sprintf(DIALTYPE,"%s","STATICIP");
    }
    if(!strcmp(DIALTYPE,"dhcp"))
    {


    	sprintf(DIALTYPE,"%s","DHCP");
    }
    if(!strcmp(DIALTYPE,"pppoe"))
    {


    	sprintf(DIALTYPE,"%s","PPPOE");
    }

    //sk_cmd_param_get("skyworth.params.sys.sn", sn, sizeof(sn));
    //sprintf(fmtstring,sn,domain....);
 //   sprintf(epg,"%s",url);
    sprintf(epg,"%s",url+7);
    p = strstr(epg,"/");
    *p = '\0';
    p = NULL;
    p = strstr(DOMAIN+7,"/");
    *(p+1) = '\0';

    sprintf(tmp_buff,para,SERVICEUSER,PPPOEACCOUNT,epg,sn,DOMAIN,HWVERSION,SWVERSION,DIALTYPE,DHCPACCOUNT);
    fp = fopen("/data/Certus.cfg","w+");

    //if(fwrite(tmp_buff,strlen(tmp_buff),1,fp))
    // fork

    if(fwrite(tmp_buff,strlen(tmp_buff),1,fp) == 1)
    {


    	system("/system/bin/qosmonloader_E5100.E5100 -d -C /data/Certus.cfg ¡§CS /data/data &");
    }

    if(fp)
    	fclose(fp);

    //:end
	 return 0;
    }
    status_t SkyworthTr069Service::recv(const void* data,size_t len)
    {
	LOGD("service recv");
	char CurTime[128];
	char ErrorInfor[1024];
	if(data == NULL || len == 0)
		return NO_INIT;
	
//	send(data,len);
	bindermsg* recv_msg =(bindermsg*)malloc(len);
	memcpy(recv_msg,data,len);
//	bindermsg * recv_msg;
//	recv_msg = (bindermsg*)data;
	LOGD("msg.user:%s\n",recv_msg->user);
	LOGD("msg.cmd:%d\n",recv_msg->cmd);
	LOGD("msg.op_type:%d\n",recv_msg->type);
	LOGD("msg.msg:%s\n",recv_msg->msg);
	LOGD("msg.len:%d\n",recv_msg->len);

	switch(recv_msg->type)
	{
		case GET_PARAM_REPLY:
			break;
		case SET_PARAM_REPLY:
			break;
		case REBOOT_REPLY:
			reboot(RB_AUTOBOOT);
			break;
		case START:
			LOGD("Recv start cmd!\n");
			/*
			sk_tr069_start();
			usleep(1*1000*1000);
			send_msg_to_client("",0,START_FINISH,"",0);
			*/
			break;
		case HEART_BEAT:
			break;
		case VALUE_CHANGE_REPORT:
		//	if(f == 1){
		//		sk_tr069_start();
		//	LOGD("Recv start cmd1!\n");
		//	f = 0;
		//		}
			
			if(recv_msg->cmd == 1005)
			{
				LOGD("start qos!\n");
				qos_start(recv_msg->msg);	
			}
			break;
		case AUTHENTICATEINFOR:
			//sk_tr069_set_data1(E_TR069_QOS_SET_AUTHFAILNUMBERS,recv_msg->msg,recv_msg->len);
			break;
		case RATELOSS:
			/*
			LOGD("Recv start cmd......... == %s!\n",recv_msg->msg);
			if(f == 1){
				sk_tr069_start();
				LOGD("Recv start cmd1!\n");
				f = 0;
			}
			*/
		//	ping(recv_msg->msg);
			break;
		case MULTIFAILINFO:
			GetCurTime(CurTime);
			sprintf(ErrorInfor,"%s-%s",CurTime,recv_msg->msg);
		//	sk_tr069_set_data1(E_TR069_QOS_SET_AUTHFAILNUMBERS,recv_msg->msg,recv_msg->len);
			break;
		case VODFAILINFO:
			GetCurTime(CurTime);
			sprintf(ErrorInfor,"%s-%s",CurTime,recv_msg->msg);
		//	sk_tr069_set_data1(E_TR069_QOS_SET_VODFAILINFO,recv_msg->msg,recv_msg->len);
			break;

		case START_PING:
			{
				char cmd[128] = {0};
				int ret = 0;
				LOGD("Recv start cmd......... == %s!\n",recv_msg->msg);
				snprintf(cmd , sizeof(cmd) , "%s %s" , recv_msg->msg , "> /ping.txt");
				ret = system(cmd);
				LOGD("system cmd = %s , ret = %d\n", cmd , ret);
				//if(0 == ret)
				{
					ret = system("chmod 555 /ping.txt");
					LOGD("chmod 555 /ping.txt ,ret = %d\n", ret);
				}
				
			}
			break;
			
		case START_TRACEROUTE:
			{
			/*
				char cmd[128] = {0};
				int ret = 0;
				LOGD("Recv start cmd......... == %s!\n",recv_msg->msg);
				snprintf(cmd , sizeof(cmd) , "%s %s" , recv_msg->msg , "> /traceroute.txt");
				ret = system(cmd);
				LOGD("system cmd = %s , ret = %d\n", cmd , ret);
				//if(0 == ret)
				{
					ret = system("chmod 555 /traceroute.txt");
					LOGD("chmod 555 /traceroute.txt ,ret = %d\n", ret);
				}
			*/
				LOGD("Recv start START_TRACEROUTE.........\n");
				sk_traceroute_attribute_t TracertAttribute;
				sk_traceroute_attribute_t* pTracertAttribute = &TracertAttribute;
				memcpy(pTracertAttribute, recv_msg->msg, sizeof(sk_traceroute_attribute_t));
				traceroute_task((void *)pTracertAttribute);
			}
			break;
		case START_TCPDUMP:
			{
				char buf[256] = {0};
				char system_cmd[256] = {0};
				int ret = 0;
				int pid = 0;
				char *ptr = NULL;
				int timeout = 0;
				pthread_t wait_pid = 0;
				kill_tcpdump_param_t param;
					
				system("rm *.pcap");
				LOGD("[START_TCPDUMP]Recv start cmd......... == %s!\n",recv_msg->msg);
				snprintf(buf , sizeof(buf)-1 , "%s" , recv_msg->msg);
				ptr = strstr(buf ,"|timeout=");
				ptr[0] = '\0';
				strcpy(system_cmd , buf);
				LOGD("[START_TCPDUMP]system_cmd = %s\n",system_cmd);
				
				ptr += strlen("|timeout=");
				timeout = strtol(ptr , NULL ,10);
				LOGD("[START_TCPDUMP]timeout = %d\n",timeout);

			
				param.pid = pid;
				param.timeout = timeout;
				
				void* kill_tcpdump(void *param);
				pthread_create(&wait_pid , NULL , kill_tcpdump , &param);
				system(system_cmd);
				
				ret = system("chmod 555 /tcpdump.pcap");
				LOGD("chmod 555 /tcpdump.pcap ,ret = %d\n", ret);
				pthread_detach(wait_pid);	
				
			}
			break;
		default:
			break;
	}
	delete(recv_msg);
	return NO_ERROR;
    }
	status_t SkyworthTr069Service::send_msg_to_client(char * user, int cmd, op_type type, char * msg, int len)
	{
		bindermsg send_msg;
		strcpy(send_msg.user,user);
		send_msg.cmd = cmd;
		send_msg.type = type;
		//strcpy(send_msg.msg,msg);
		memcpy(send_msg.msg, msg, len);
		send_msg.len = len;

		send(&send_msg,sizeof(bindermsg));
		LOGD("send msg.type:%d to client\n",send_msg.type);
		return NO_ERROR;
	}

	void* kill_tcpdump(void *param)
	{
		int pid = 0;
		int timeout = 0;
		kill_tcpdump_param_t *ptr = (kill_tcpdump_param_t *)param;
		time_t start_time = time(NULL);
		time_t end_time = {0};
		pthread_detach(pthread_self());
		
		pid = ptr->pid;
		timeout = ptr->timeout;
		
		while(1)
		{
			end_time = time(NULL);
			if(end_time-start_time >= timeout)
			{
				int ret = 0;
				ret = system("killall -9 tcpdump");
				LOGD("[START_TCPDUMP]wkillall -9 tcpdump ret = ",ret);
				break;
			}
			LOGD("[START_TCPDUMP]time = %u",end_time-start_time);
			sleep(1);
		}
		 
		return NULL;
	}
	
	void SkyworthTr069Service::traceroute_task(void * args)
	{
		LOGI("traceroute_task start\n");
		sk_traceroute_attribute_t* pTracertAttribute = (sk_traceroute_attribute_t*)args;
		sk_traceroute_results_t TracertResults;
		sk_traceroute_results_t* pTracertResults = &TracertResults;
		memset(pTracertResults, 0, sizeof(sk_traceroute_results_t));
		sk_start_traceroute(pTracertAttribute , pTracertResults);
		
		//send result to client
		send_msg_to_client("TR069", 0, START_TRACEROUTE, (char *) pTracertResults, sizeof(sk_traceroute_results_t));
	}

}



