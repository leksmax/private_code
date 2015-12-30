#ifndef __SK_TR069_H__
#define __SK_TR069_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _tag_tr069_boot_status_e
{
	E_TR069_BOOTSTRAP			=0,
	E_TR069_BOOT				,
	E_TR069_PERIODIC			,
	E_TR069_VALUE_CHANGED		,
	E_TR069_TRANSFER_COMPLETE	,
	E_TR069_DIAGNOSTICS_COMPLETE,
	E_TR069_M_REBOOT			,
	E_TR069_LOG_PERIODIC		,
	E_TR069_SHUT_DOWN           ,
	E_TR069_MAX
}sk_tr069_boot_status_e;


typedef enum _log_type
{
	//:by liangzhen @2012.10.14
	E_TR069_LOG_ENABLE,
	E_TR069_LOG_MSGORFILE,
	E_TR069_LOG_LOGFTPSERVER,
	E_TR069_LOG_LOGFTPUSER,
	E_TR069_LOG_LOGFTPPASSWORD,
	E_TR069_LOG_DURATION,
	//:end
	E_TR069_LOG_RTSP_INFO,
	E_TR069_LOG_HTTP_INFO,
	E_TR069_LOG_IGMP_INFO,
	E_TR069_LOG_ERR_INFO,
	E_TR069_LOG_PKG_TOTAL_ONE_SEC,
	E_TR069_LOG_BYTE_TOTAL_ONE_SEC,
	E_TR069_LOG_PKG_LOST_RATE,
	E_TR069_LOG_AVARAGE_RATE,
	E_TR069_LOG_BUFFER
}sk_tr069_log_type;

typedef enum _qos_type
{
	E_TR069_QOS_SET_AUTHNUMBERS,
	E_TR069_QOS_SET_AUTHFAILNUMBERS,
	E_TR069_QOS_SET_MULTIREQNUMBERS,
	E_TR069_QOS_SET_MULTIRRT,
	
	E_TR069_QOS_SET_ERROR,
	E_TR069_QOS_SET_BUFFERINFO,
	E_TR069_QOS_SET_RTSPINFO,
	E_TR069_QOS_SET_IGMPINFO,
	E_TR069_QOS_SET_HTTPINFO,
	E_TR069_QOS_SET_VODFAILINFO,
	E_TR069_QOS_SET_HTTPFAILINFO,
	E_TR069_QOS_SET_PLAYERRORINFO,
	E_TR069_QOS_SET_MULTIFAILINFO,
	
	E_TR069_QOS_SET_VODREQNUMBER,
	E_TR069_QOS_SET_VODRRT,
	E_TR069_QOS_SET_VODFAILNUMBER,

	E_TR069_QOS_SET_HTTPREQNUMBER,
	E_TR069_QOS_SET_HTTPRRT,
	E_TR069_QOS_SET_HTTPFAILNUMBER,
	
	E_TR069_QOS_SET_MULTIABENDNUMBER,
	
	E_TR069_QOS_SET_VODABENDNUMBER,
	E_TR069_QOS_SET_MULTIABENDUPNUMBER,
	E_TR069_QOS_SET_VODABENDUPNUMBER,
	E_TR069_QOS_SET_PLAYERRORNUMBER,
	

	E_TR069_QOS_SET_MULTIPLOSTR,
	E_TR069_QOS_SET_FECMULTIPLOSTR,

	E_TR069_QOS_SET_VODPLOSTR,
	E_TR069_QOS_SET_ARQVODPLOSTR,

	E_TR069_QOS_SET_MULTIBITRATER,
	E_TR069_QOS_SET_VODBITRATER,
	
	E_TR069_QOS_SET_BUFFERINCNMB,
	E_TR069_QOS_SET_BUFFERDECNMB,
	
	E_TR069_QOS_SET_FRAMELOSTR,

	E_TR069_QOS_MAX
}sk_tr069_qos_type;









int sk_tr069_start();								//启动Tr069服务

int sk_tr069_shutdown();							//机顶盒关闭时应该调用本方法，告知服务端STB已经shutdown了

int sk_set_param_value_changed(const char *param_name);	//参数值发生变化时通知tr069模块

int sk_tr069_set_first_usenet_time();				//设置首次成功建立网络连接的日期和时间。
	
int sk_tr069_set_last_restart_time();				//机顶盒最后一次重启后的时间

int sk_tr069_set_upgrade_status(int value);			//设置机顶盒当前的升级状态:1，正在升级，0:没有升级

int sk_tr069_get_upgrade_status();					//获得当前升级状态:1，正在升级，0:没有升级

int sk_tr069_get_first_connect_status();			//返回当前连接状态，如果是第一次连接，则返回1，否则返回0

int sk_tr069_set_log_info(int log_type,const char *log_info);	//设置日至消息
int sk_tr069_set_log_pkgtotal(unsigned int value);   //1秒内收到的package包数，丢失的package包数量
int sk_tr069_set_log_bytetotal(unsigned int value);  //1秒内收到的字节数，丢失的字节数
int sk_tr069_set_log_pkglostrate(unsigned int value);//10秒统计信息：前10秒的网络丢包率
int sk_tr069_set_log_avaragerate(unsigned int value);//10秒统计信息：前10秒的平均码率
int sk_tr069_set_log_buffer(int value);				 //播放器缓存占用率，以及播放器缓存容量（以字节为单位）
//void sk_tr069_set_data1(int type,char *buffer,int size);

#ifdef __cplusplus
}
#endif


#endif

