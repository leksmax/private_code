#include <binder/Parcel.h>
#include <binder/IServiceManager.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sk_param_porting.h"
#include "sk_params.h"


#define LOG_PARA_TAG    "JNILOG"
#define LOGD(...)  __android_log_print(ANDROID_LOG_INFO,LOG_PARA_TAG,__VA_ARGS__)

typedef struct _param{
	char name[40];
	char value[128];
	}tr069_params;

//typedef struct _s_sk_tr069_callback_info_t{
//	void *obj;
//	sk_tr069_callback_t cb;
//}sk_tr069_callback_info;

//static sk_tr069_callback_info m_callback[1] = {0};

static tr069_params paramCastTbl[]=
{	
	{"tr069_event","skyworth.params.tr069.event"},
	{"tr069_inform","skyworth.params.tr069.inform"},	
	{"tr069_inform_period","skyworth.params.tr069.inform_period"},	

	{"product_type","skyworth.params.sys.product_type"},		

	{"net_mode","skyworth.params.net.netmode"},		
	{"net_type","skyworth.params.net.connectmode"},
	{"gateway","skyworth.params.net.gateway"},	
	{"dns1","skyworth.params.net.dns1"},	
	{"dns2","skyworth.params.net.dns2"},	
	
	{"lan_ip","skyworth.params.net.lan_ip"},	
	{"lan_mask","skyworth.params.net.lan_mask"},				
	{"tr069_acs","skyworth.params.tr069.acs"},
	{"tr069_user2","skyworth.params.tr069.user2"},
	{"tr069_pwd","skyworth.params.tr069.pwd"},
	{"tr069_upgrage","skyworth.params.tr069.upgrage"},
	{"sn","skyworth.params.sys.sn"},
	{"mac","skyworth.params.sys.mac"},

	{"AlarmSwitch","skyworth.params.tr069.AlarmSwitch"},
	{"cpu_usage","skyworth.params.tr069.cpu_usage"},
	{"mem_total","skyworth.params.tr069.mem_total"},
	{"mem_free","skyworth.params.tr069.mem_free"},
	//使用vlanid 暂时代替
	//{"AlarmSwitch", "skyworth.params.net.pppoedev"},

	{"tr069_local_user","skyworth.params.tr069.local_user"},
	{"tr069_local_pwd","skyworth.params.tr069.local_pwd"},
	{"software_version","skyworth.params.sys.software_version"},
	{"hardware_version","skyworth.params.sys.hardware_version"},
	{"ntp_server","skyworth.params.sys.ntp_server"},
	{"ntp_server2","skyworth.params.sys.ntp_server2"},
	{"pppoe_user","skyworth.params.net.pppoeusr"},
	{"pppoe_pwd","skyworth.params.net.pppoepwd"},
	{"timezone","skyworth.params.sys.timezone"},
	{"log_server","skyworth.params.itv.log_server"},
	{"record_interval","skyworth.params.qos.record_interval"},
	{"upload_interval","skyworth.params.qos.upload_interval"},
	{"monitor_interval","skyworth.params.qos.monitor_interval"},
//:add by liangzhen
//:this params not contain in param list
	{"tr069_commandkey","skyworth.params.tr069.commandkey"},
	{"tr069_upgrade_server","skyworth.params.tr069.upgrade_server"},
//:end

	{"fileorrealtime","skyworth.params.qos.fileorrealtime"},
	{"qos_enable","skyworth.params.qos.qos_enable"},
		
	{"upload_style","skyworth.params.itv.log_style"},
	{"upload_url","skyworth.params.itv.log_server"},
	{"upload_username","skyworth.params.qos.upload_usr"},
	{"upload_password","skyworth.params.qos.upload_pwd"},
	
	{"packets_lostr1","skyworth.params.qos.packets_lostr1"},
	{"packets_lostr2","skyworth.params.qos.packets_lostr2"},
	{"packets_lostr3","skyworth.params.qos.packets_lostr3"},
	{"packets_lostr4","skyworth.params.qos.packets_lostr4"},
	{"packets_lostr5","skyworth.params.qos.packets_lostr5"},
	{"bit_rater1","skyworth.params.qos.bit_rater1"},
	{"bit_rater2","skyworth.params.qos.bit_rater2"},
	{"bit_rater3","skyworth.params.qos.bit_rater3"},
	{"bit_rater4","skyworth.params.qos.bit_rater4"},
	{"bit_rater5","skyworth.params.qos.bit_rater5"},
		
	{"pppoe_username","skyworth.params.net.pppoeusr"},
	{"username","skyworth.params.itv.username"},
	{"password","skyworth.params.itv.password"},
	{"home_page","skyworth.params.itv.homepage"},
	{"home_page2","skyworth.params.itv.homepage2"},

//sunjain tr111:
   {"tr069_stunenbale","skyworth.params.stun.enable"},
	{"tr069_stunaddr","skyworth.params.stun.addr"},
	{"tr069_stunport","skyworth.params.stun.port"},
	{"tr069_stunname","skyworth.params.stun.name"},
	{"tr069_stunpwd","skyworth.params.stun.pwd"},
    {"tr069_udpconnectaddr","skyworth.params.stun.udpaddr"},
    
    {"tr069_ping","skyworth.params.netmag.ping"},
    {"tr069_route","skyworth.params.netmag.traceroute"},
    {"tr069_reboot","skyworth.params.sys.reboot"}, 
	{"tr069_vendor_log","skyworth.params.sys.vendor_log"},
	{"tr069_url_modify_flag","skyworth.params.tr069.url_modify_flag"},
	{"tr069_address_notificationlimit","skyworth.params.tr069.address_notificationlimit"},
    //江苏移动项目,新添加的接口
	{"user_intall_application","skyworth.params.sys.user_intall_application"},
	{"tr069_bak_acs","skyworth.params.tr069.bak_acs"},
	{"tr069_platform_url","skyworth.params.tr069.platform_url"},
	{"tr069_platform_bak_url","skyworth.params.tr069.platform_bak_url"},
	{"tr069_hdc_url","skyworth.params.tr069.hdc_url"},
	{"tr069_silent_upgrade","skyworth.params.tr069.silent_upgrade"},
	{"tr069_forced_upgrade","skyworth.params.tr069.forced_upgrade"},
	{"tr069_system_build_time","skyworth.params.tr069.system_build_time"},
	{"tr069_bak_system_build_time","skyworth.params.tr069.bak_system_build_time"},

	{"tr069_pcap_state","skyworth.params.tr069.tr069_pcap_state"},
	{"tr069_pcap_ip_addr","skyworth.params.tr069.tr069_pcap_ip_addr"},
	{"tr069_pcap_ip_port","skyworth.params.tr069.tr069_pcap_ip_port"},
	{"tr069_pcap_duration","skyworth.params.tr069.tr069_pcap_duration"},
	{"tr069_pcap_uploadurl","skyworth.params.tr069.tr069_pcap_uploadurl"},
	{"tr069_pcap_username","skyworth.params.tr069.tr069_pcap_username"},
	{"tr069_pcap_password","skyworth.params.tr069.tr069_pcap_password"},
	{"tr069_pcap_start","skyworth.params.tr069.tr069_pcap_start"},


	{"tr069_bandwidth_state","skyworth.params.tr069.tr069_bandwidth_state"},
	{"tr069_bandwidth_downloadurl","skyworth.params.tr069.tr069_bandwidth_downloadurl"},
	{"tr069_bandwidth_username","skyworth.params.tr069.tr069_bandwidth_username"},
	{"tr069_bandwidth_password","skyworth.params.tr069.tr069_bandwidth_password"},
	{"tr069_bandwidth_errorcode","skyworth.params.tr069.tr069_bandwidth_errorcode"},

	{"tr069_bandwidth_maxspeed","skyworth.params.tr069.tr069_bandwidth_maxspeed"},
	{"tr069_bandwidth_avgspeed","skyworth.params.tr069.tr069_bandwidth_avgspeed"},
	{"tr069_bandwidth_minspeed","skyworth.params.tr069.tr069_bandwidth_minspeed"},

	{"tr069_bandwidth_start","skyworth.params.tr069.tr069_bandwidth_start"},


	{"tr069_syslog_server","skyworth.params.syslog.server"},
	{"tr069_syslog_delay","skyworth.params.syslog.delay"},
	{"tr069_syslog_loglevel","skyworth.params.syslog.loglevel"},
	{"tr069_syslog_puttype","skyworth.params.syslog.puttype"},
	{"tr069_syslog_starttime","skyworth.params.syslog.starttime"},
	{"tr069_syslog_start","skyworth.params.syslog.start"},
				
	
	{"usb_permit_installed_app_nums","skyworth.params.usb_permit_installed_app_nums"},
	{"usb_install_packagename1","skyworth.params.usb_install_packagename1"},
	{"usb_install_packagename2","skyworth.params.usb_install_packagename2"},
	{"usb_install_packagename3","skyworth.params.usb_install_packagename3"},
	{"usb_install_packagename4","skyworth.params.usb_install_packagename4"},
	{"usb_install_packagename5","skyworth.params.usb_install_packagename5"},
	{"usb_install_packagename6","skyworth.params.usb_install_packagename6"},
	{"usb_install_packagename7","skyworth.params.usb_install_packagename7"},
	{"usb_install_packagename8","skyworth.params.usb_install_packagename8"},
	{"usb_install_packagename9","skyworth.params.usb_install_packagename9"},
	{"usb_install_packagename10","skyworth.params.usb_install_packagename10"},
	
	{"usb_install_classname1","skyworth.params.usb_install_classname1"},
	{"usb_install_classname2","skyworth.params.usb_install_classname2"},
	{"usb_install_classname3","skyworth.params.usb_install_classname3"},
	{"usb_install_classname4","skyworth.params.usb_install_classname4"},
	{"usb_install_classname5","skyworth.params.usb_install_classname5"},
	{"usb_install_classname6","skyworth.params.usb_install_classname6"},
	{"usb_install_classname7","skyworth.params.usb_install_classname7"},
	{"usb_install_classname8","skyworth.params.usb_install_classname8"},
	{"usb_install_classname9","skyworth.params.usb_install_classname9"},
	{"usb_install_classname10","skyworth.params.usb_install_classname10"},

	{"app_auto_run_blacklist_flag","skyworth.params.app_auto_run_blacklist_flag"},
	{"app_auto_run_blacklist_nums","skyworth.params.app_auto_run_blacklist_nums"},

	{"autorun_app_packagename1","skyworth.params.autorun_app_packagename1"},
	{"autorun_app_packagename2","skyworth.params.autorun_app_packagename2"},
	{"autorun_app_packagename3","skyworth.params.autorun_app_packagename3"},
	{"autorun_app_packagename4","skyworth.params.autorun_app_packagename4"},
	{"autorun_app_packagename5","skyworth.params.autorun_app_packagename5"},
	{"autorun_app_packagename6","skyworth.params.autorun_app_packagename6"},
	{"autorun_app_packagename7","skyworth.params.autorun_app_packagename7"},
	{"autorun_app_packagename8","skyworth.params.autorun_app_packagename8"},
	{"autorun_app_packagename9","skyworth.params.autorun_app_packagename9"},
	{"autorun_app_packagename10","skyworth.params.autorun_app_packagename10"},

	{"autorun_app_classname1","skyworth.params.autorun_app_classname1"},
	{"autorun_app_classname2","skyworth.params.autorun_app_classname2"},
	{"autorun_app_classname3","skyworth.params.autorun_app_classname3"},
	{"autorun_app_classname4","skyworth.params.autorun_app_classname4"},
	{"autorun_app_classname5","skyworth.params.autorun_app_classname5"},
	{"autorun_app_classname6","skyworth.params.autorun_app_classname6"},
	{"autorun_app_classname7","skyworth.params.autorun_app_classname7"},
	{"autorun_app_classname8","skyworth.params.autorun_app_classname8"},
	{"autorun_app_classname9","skyworth.params.autorun_app_classname9"},
	{"autorun_app_classname10","skyworth.params.autorun_app_classname10"}
};


#if 0
int sk_func_porting_set_callback(void* obj, sk_tr069_callback_t callback)
{
    LOGD("[sk_func_porting_set_callback]............");
	if(callback){
		LOGD("[sk_func_porting_set_callback] set value............");
		m_callback->obj = obj;
		m_callback->cb = callback;
		LOGD("[sk_func_porting_set_callback]  callback not null............");
		return 0;
	}
	LOGD("[sk_func_porting_set_callback] error...callback is null.");
	return -1;
}
#endif


int sk_func_porting_params_get(const char *name,char *value,int size)
{
	int i = 0;
	char tempbuf[2048]={0};
	int ret = 0;
	if(NULL == name || NULL == value /*||0 == size*/)
	{

		LOGD("sk_func_porting_params_get param is error!\n");
		return -1; 
	}
		
	for(int i=0;i<sizeof(paramCastTbl)/sizeof(tr069_params);i++)
	{
		if(0 == strcmp(paramCastTbl[i].name,name))
		{
			ret = sk_api_params_get(paramCastTbl[i].value,tempbuf,size);
			LOGD("get param name:%s value:%s,size:%d\n",name,tempbuf,strlen(tempbuf));
			memset(value,0,size);
			if(strcmp(name,"mac")==0)
			{
				for(i=0;i<strlen(tempbuf);i++)
				{
					//LOGD("get param name:%s i:%d  value:%x,size:%d\n",name,i,tempbuf[i]);
						
					if((tempbuf[i]!='\n') && (tempbuf[i]!=0xa))
					{
						value[i]=tempbuf[i];
					}
					else
					{
						value[i]='\0';
						break;
					}						
				}
			}
			else		
			{
				strcpy(value,tempbuf);
			}
			return ret;
		}
	}
	LOGD("sk_func_porting_params_get param not found!!! name:%s",name);
	return -1;
}


int sk_func_porting_params_set(const char *name,const char *value)
{
	for(int i=0;i<sizeof(paramCastTbl)/sizeof(tr069_params);i++)
	{
		if(0 == strcmp(paramCastTbl[i].name,name))
		{
			LOGD("sk_func_porting_params_set name:%s value:%s!",name,value);
			return sk_api_params_set(paramCastTbl[i].value,value);
		}
	}
	LOGD("sk_func_porting_params_set param not found!!! name:%s,value:%s",name,value);
	return -1;
}
int sk_func_porting_reboot()
{
	return 0;
}
int sk_func_porting_factoryreset()
{
//	sk_factory_reset();
	return 0;
}

int sk_tr069_porting_upgrade(const char* url)
{
	LOGD("call sk_tr069_porting_upgrade begin,url:%s",url);
	//return sk_api_upgrade_check_url(url);
	sk_upgrade_start(url);
	LOGD("call sk_tr069_porting_upgrade end");
	return 0;
}
#if 0
int sk_tr069_porting_set_logserverurl(char *url)
{
	LOGD("call sk_tr069_porting_set_logserverurl value:%s...",url);
	return (Tr069Service::getInstance())->set_logserverurl(url);
	LOGD("call sk_tr069_porting_set_logserverurl end");
}
int sk_tr069_porting_set_loguploadinterval(int interval)
{
	LOGD("call sk_tr069_porting_set_loguploadinterval value %d...",interval);
	return (Tr069Service::getInstance())->set_loguploadinterval(interval);
	LOGD("call sk_tr069_porting_set_loguploadinterval end");
}
int sk_tr069_porting_set_logrecordinterval(int interval)
{
	LOGD("call sk_tr069_porting_set_logrecordinterval value %d...",interval);
	return (Tr069Service::getInstance())->set_logrecordinterval(interval);
	LOGD("call sk_tr069_porting_set_logrecordinterval end");
}

int sk_tr069_porting_get_current_lost_rate()
{
	LOGD("call sk_tr069_porting_get_current_lost_rate ");
    if(m_callback->cb){
		LOGI("[sk_tr069_porting_get_current_lost_rate]  m_callback->cb not null............");
		m_callback->cb(m_callback->obj,SKTR069_CB_NM_ACCEPT_NOTIFY, NOTIFY_SET_PACKAGE_LOST_RATE, "NULL");
        LOGD("call sk_tr069_porting_set_loguploadinterval end");
        return 0;
	}
	//return (Tr069Service::getInstance())->get_current_lost_rate();
	LOGD("call sk_tr069_porting_set_loguploadinterval fail, no callback!");
    return -1;
}

int sk_tr069_porting_get_current_play_url()
{
	LOGI("call sk_tr069_porting_get_current_play_url ");
    if(m_callback->cb){
		LOGI("[sk_tr069_porting_get_current_play_url]  m_callback->cb not null............");
		m_callback->cb(m_callback->obj,SKTR069_CB_NM_ACCEPT_NOTIFY, NOTIFY_SET_CURRENT_PLAY_URL, "NULL");
        LOGI("call sk_tr069_porting_get_current_play_url end");
        return 0;
	}
	//return (Tr069Service::getInstance())->get_current_play_url();
	LOGI("call sk_tr069_porting_get_current_play_url fail, no callback!");
	return -1;
}

int sk_tr069_porting_set_play_url(const char* value )
{
	LOGI("call sk_tr069_porting_set_play_url : %s", value);
    if(m_callback->cb){
		LOGI("[sk_tr069_porting_set_play_url]  m_callback->cb not null............");
		m_callback->cb(m_callback->obj,SKTR069_CB_NM_SET_PLAY_URL, 1, value);
        LOGI("call sk_tr069_porting_set_play_url end");
        return 0;
	}
	//return (Tr069Service::getInstance())->get_current_play_url();
	LOGI("call sk_tr069_porting_set_play_url fail, no callback!");
	return -1;
	//return (Tr069Service::getInstance())->set_play_url(value);
}
#endif
