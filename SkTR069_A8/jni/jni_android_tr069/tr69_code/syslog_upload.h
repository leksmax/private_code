#ifndef  EVCPE_SYSLOG_UPLOAD_H_
#define  EVCPE_SYSLOG_UPLOAD_H_


typedef struct syslog_s
{
	int 						log_type;				//是否是否开起	
	int 						log_level;				//上报级别
	int 						put_type;				//上报类型
	char						log_server[256];		//上报的服务器	
	char						log_user[60];
	char						log_pwd[60];
	
	char						upload_starttime[32];	//syslog开始上报的时间	
	int 						continue_time;			//持续时间
	
}sk_syslog_t;


int sk_start_syslog_diagnostics(sk_syslog_t *syslog);


#endif//EVCPE_SYSLOG_UPLOAD_H_


