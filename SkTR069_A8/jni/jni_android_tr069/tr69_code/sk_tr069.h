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
	ALARM_UPGRADE_FAILURE_CODE  =300101,				 //����ʧ��
	ALARM_DISK_FAILURE_CODE        =100102,				 //����ʧ��
	ALARM_CPU_FAILURE_CODE      =   100103, 			 //CPU
	ALARM_LOSTPACKET_FAILURE_CODE      =100106, 		 //����
	ALARM_AUTH_FAILURE_CODE      =300103, 				 //��֤ʧ��
	ALARM_JOINCHANNEL_FAILURE_CODE      =  300104, 		 //����Ƶ��ʧ��
	ALARM_DECODE_FAILURE_CODE  =100108,  				 //����ʶ��
	ALARM_UNKNOWN_FAILURE_CODE
}sk_alarm_code;

typedef enum upload_infotype
{
	INFO_CONFIG_TYPE = 0, /*����*/
	INFO_QOS_TYPE,  /*ͳ�����ܲ���qos*/
	INFO_LOG_TYPE	
}sk_upload_infotype;


typedef struct alarm_s
{
	int 						alarm_switch;			//�Ƿ��Ƿ���	
	int 						alarm_level;			//�ϱ�����	
	char 					cpu_alarm[20];			//�ϱ�����
	char						memory_alarm[20];		//�ϱ��ķ�����	
	char						disk_alarm[20];			//syslog��ʼ�ϱ���ʱ��	
	
	char 					band_width_alarm[20];	//����ʱ��
	char 					packet_lost_alarm[20];	//
	char                     			 errorcode[10];
	//�Ĵ�:
	char 					packet_lostframe_alarm[20];	//
	char 					packet_timedelay_alarm[20];	//
	char 					packet_cushion_alarm[20];	//
	//
	int                                       alarmflag;
	
}sk_alarm_t;

int sk_tr069_start();								//����Tr069����

int sk_tr069_shutdown();							//�����йر�ʱӦ�õ��ñ���������֪�����STB�Ѿ�shutdown��

int sk_set_param_value_changed(const char *param_name);	//����ֵ�����仯ʱ֪ͨtr069ģ��

int sk_tr069_set_first_usenet_time();				//�����״γɹ������������ӵ����ں�ʱ�䡣
	
int sk_tr069_set_last_restart_time();				//���������һ���������ʱ��

int sk_tr069_set_upgrade_status(int value);			//���û����е�ǰ������״̬:1������������0:û������

int sk_tr069_get_upgrade_status();					//��õ�ǰ����״̬:1������������0:û������

int sk_tr069_get_first_connect_status();			//���ص�ǰ����״̬������ǵ�һ�����ӣ��򷵻�1�����򷵻�0

int sk_tr069_set_log_info(int log_type,char *log_info);	//����������Ϣ
int sk_tr069_set_log_pkgtotal(unsigned int value);   //1�����յ���package��������ʧ��package������
int sk_tr069_set_log_bytetotal(unsigned int value);  //1�����յ����ֽ�������ʧ���ֽ���
int sk_tr069_set_log_pkglostrate(unsigned int value);//10��ͳ����Ϣ��ǰ10������綪����
int sk_tr069_set_log_avaragerate(unsigned int value);//10��ͳ����Ϣ��ǰ10���ƽ������
int sk_tr069_set_log_buffer(int value);				 //����������ռ���ʣ��Լ��������������������ֽ�Ϊ��λ��
void sk_band_width_test_response_start();


int sk_tr069_alarm_report(int type, int code);
int sk_tr069_get_alarminfo(sk_alarm_t* alarminfo);
int sk_tr069_alarm_reportaction(int type,int code);
int sk_tr069_xingneng_report();



typedef enum{ 
    GET_PARAM, // TM ---> IPTV ??��?2?��y 
    SET_PARAM, // TM ---> IPTV ����??2?��y 
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

