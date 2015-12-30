#ifndef  EVCPE_SYSLOG_UPLOAD_H_
#define  EVCPE_SYSLOG_UPLOAD_H_


typedef struct syslog_s
{
	int 						log_type;				//�Ƿ��Ƿ���	
	int 						log_level;				//�ϱ�����
	int 						put_type;				//�ϱ�����
	char						log_server[256];		//�ϱ��ķ�����	
	char						log_user[60];
	char						log_pwd[60];
	
	char						upload_starttime[32];	//syslog��ʼ�ϱ���ʱ��	
	int 						continue_time;			//����ʱ��
	
}sk_syslog_t;


int sk_start_syslog_diagnostics(sk_syslog_t *syslog);


#endif//EVCPE_SYSLOG_UPLOAD_H_


