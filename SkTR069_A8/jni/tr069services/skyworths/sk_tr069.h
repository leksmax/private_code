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









int sk_tr069_start();								//����Tr069����

int sk_tr069_shutdown();							//�����йر�ʱӦ�õ��ñ���������֪�����STB�Ѿ�shutdown��

int sk_set_param_value_changed(const char *param_name);	//����ֵ�����仯ʱ֪ͨtr069ģ��

int sk_tr069_set_first_usenet_time();				//�����״γɹ������������ӵ����ں�ʱ�䡣
	
int sk_tr069_set_last_restart_time();				//���������һ���������ʱ��

int sk_tr069_set_upgrade_status(int value);			//���û����е�ǰ������״̬:1������������0:û������

int sk_tr069_get_upgrade_status();					//��õ�ǰ����״̬:1������������0:û������

int sk_tr069_get_first_connect_status();			//���ص�ǰ����״̬������ǵ�һ�����ӣ��򷵻�1�����򷵻�0

int sk_tr069_set_log_info(int log_type,const char *log_info);	//����������Ϣ
int sk_tr069_set_log_pkgtotal(unsigned int value);   //1�����յ���package��������ʧ��package������
int sk_tr069_set_log_bytetotal(unsigned int value);  //1�����յ����ֽ�������ʧ���ֽ���
int sk_tr069_set_log_pkglostrate(unsigned int value);//10��ͳ����Ϣ��ǰ10������綪����
int sk_tr069_set_log_avaragerate(unsigned int value);//10��ͳ����Ϣ��ǰ10���ƽ������
int sk_tr069_set_log_buffer(int value);				 //����������ռ���ʣ��Լ��������������������ֽ�Ϊ��λ��
//void sk_tr069_set_data1(int type,char *buffer,int size);

#ifdef __cplusplus
}
#endif


#endif

