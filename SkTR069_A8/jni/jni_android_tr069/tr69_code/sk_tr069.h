#ifndef __SK_TR069_H__
#define __SK_TR069_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _tag_tr069_boot_status_e
{
	E_TR069_BOOTSTRAP			=0,
	E_TR069_BOOT				,//1
	E_TR069_PERIODIC			,//2
	E_TR069_VALUE_CHANGED		,//3
	E_TR069_TRANSFER_COMPLETE	,//4
	E_TR069_DIAGNOSTICS_COMPLETE,//5
	E_TR069_M_REBOOT			,//6
	E_TR069_LOG_PERIODIC		,//7
	E_TR069_SHUT_DOWN           ,//8
	E_TR069_MAX
}sk_tr069_boot_status_e;

typedef enum _log_type
{
	ENUM_LOG_RTSP_INFO,
	ENUM_LOG_HTTP_INFO,
	ENUM_LOG_IGMP_INFO,
	ENUM_LOG_ERR_INFO,
	ENUM_LOG_PKG_TOTAL_ONE_SEC,
	ENUM_LOG_BYTE_TOTAL_ONE_SEC,
	ENUM_LOG_PKG_LOST_RATE,
	ENUM_LOG_AVARAGE_RATE,
	ENUM_LOG_BUFFER
}sk_log_type;

typedef enum _alarm_code
{
	ALARM_UPGRADE_FAILURE_CODE  =300101,				 //升级失败
	ALARM_DISK_FAILURE_CODE        =100102,				 //磁盘失败
	ALARM_CPU_FAILURE_CODE      =   100103, 			 //CPU
	ALARM_LOSTPACKET_FAILURE_CODE      =100106, 		 //丢包
	ALARM_AUTH_FAILURE_CODE      =300103, 				 //认证失败
	ALARM_JOINCHANNEL_FAILURE_CODE      =  300104, 		 //加入频道失败
	ALARM_DECODE_FAILURE_CODE  =100108,  				 //解码识别
	ALARM_UNKNOWN_FAILURE_CODE
}sk_alarm_code;

typedef enum upload_infotype
{
	INFO_CONFIG_TYPE = 0, /*配置*/
	INFO_QOS_TYPE,  /*统计性能参数qos*/
	INFO_LOG_TYPE	
}sk_upload_infotype;


typedef struct alarm_s
{
	int 						alarm_switch;			//是否是否开起	
	int 						alarm_level;			//上报级别	
	char 					cpu_alarm[20];			//上报类型
	char						memory_alarm[20];		//上报的服务器	
	char						disk_alarm[20];			//syslog开始上报的时间	
	
	char 					band_width_alarm[20];	//持续时间
	char 					packet_lost_alarm[20];	//
	char                     			 errorcode[10];
	//四川:
	char 					packet_lostframe_alarm[20];	//
	char 					packet_timedelay_alarm[20];	//
	char 					packet_cushion_alarm[20];	//
	//
	int                                       alarmflag;
	
}sk_alarm_t;

int sk_tr069_start();								//启动Tr069服务

int sk_tr069_shutdown();							//机顶盒关闭时应该调用本方法，告知服务端STB已经shutdown了

int sk_set_param_value_changed(const char *param_name);	//参数值发生变化时通知tr069模块

int sk_tr069_set_first_usenet_time();				//设置首次成功建立网络连接的日期和时间。
	
int sk_tr069_set_last_restart_time();				//机顶盒最后一次重启后的时间

int sk_tr069_set_upgrade_status(int value);			//设置机顶盒当前的升级状态:1，正在升级，0:没有升级

int sk_tr069_get_upgrade_status();					//获得当前升级状态:1，正在升级，0:没有升级

int sk_tr069_get_first_connect_status();			//返回当前连接状态，如果是第一次连接，则返回1，否则返回0

int sk_tr069_set_log_info(int log_type,char *log_info);	//设置日至消息
int sk_tr069_set_log_pkgtotal(unsigned int value);   //1秒内收到的package包数，丢失的package包数量
int sk_tr069_set_log_bytetotal(unsigned int value);  //1秒内收到的字节数，丢失的字节数
int sk_tr069_set_log_pkglostrate(unsigned int value);//10秒统计信息：前10秒的网络丢包率
int sk_tr069_set_log_avaragerate(unsigned int value);//10秒统计信息：前10秒的平均码率
int sk_tr069_set_log_buffer(int value);				 //播放器缓存占用率，以及播放器缓存容量（以字节为单位）
void sk_band_width_test_response_start();


int sk_tr069_alarm_report(int type, int code);
int sk_tr069_get_alarminfo(sk_alarm_t* alarminfo);
int sk_tr069_alarm_reportaction(int type,int code);
int sk_tr069_xingneng_report();



typedef enum{ 
    GET_PARAM, // TM ---> IPTV ??è?2?êy 
    SET_PARAM, // TM ---> IPTV éè??2?êy 
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
    char msg[1024 * 30];
    int len;
}bindermsg;

typedef void(*sk_tr069_callback_t)(const void* data, int arg1, int arg2);
int sk_binder_send(const void* data,int len);
int sk_binder_set_cb(sk_tr069_callback_t cb);
int sk_binder_cb(const void* data,int len);
void sk_send_msg(char *users,int cmd,op_type type,char *msg,int len);


#ifdef __cplusplus
}
#endif


#endif

