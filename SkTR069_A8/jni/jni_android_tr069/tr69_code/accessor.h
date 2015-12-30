// $Id: accessor.h 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EVCPE_ACCESSOR_H_
#define EVCPE_ACCESSOR_H_
#include <pthread.h>
#include "attr_schema.h"
#include "download.h"
#include "upload.h"
#include "bandwidth_diagnostics.h"
#include "net_diagnosis.h"
#include "sk_tr069.h"
#include "log.h"
#include "util.h"
#include "packet_capture.h"
#include "syslog_upload.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SK_BUFFER_SIZE			(1024)
#define SK_LOG_ARRAY_SIZE		(20)
#define SK_LOG_FILE_ARRAY_SIZE	(10)


#ifdef WIN32
#define SK_TR069_MODEL "E:\\iCode\\vc_sky_tr069\\vc_sky_tr069\\tr069_xml_modle\\SDYD_tr069_model.xml"
#else
#define SK_TR069_MODEL  "/system/lib/libtr069_model.so"
#endif

typedef struct match_table_s
{
	int  use_flag;               //0,û��ʹ�ã�1���Ѿ��޸ģ���δ�ϱ��޸ģ�2���Ѿ��޸ģ������Ѿ��ϱ����޸�
	char params_name[32];
	char tr069_param[64];
}sk_match_table_t;


typedef struct change_table_s
{
	char params_name[32];
}sk_change_table_t;



typedef struct play_attribute_s
{
	char play_url[1024];						//����play��ϵ����������ַ
	int  play_status;							//���԰�������DiffServ����㣬Ĭ��ֵΪ0
}sk_play_attribute_t;


typedef struct logbuffer_s
{
	char log_buffer[1024];		//����buffer
	int  flag;				//���:0,bufferδ��䣬1���Ѿ���䣬
}sk_logbuffer_t;

typedef struct logfile_s
{
	char log_file[512];		//����buffer
	int  flag;				//���:0,bufferδ��䣬1���Ѿ���䣬
}sk_logfile_t;


typedef struct shlog_s  //�Ϻ���־
{
	
	pthread_t            		logmsg_thread;
	char						log_info[9][1024];
	int							log_flag;
	int							log_enable;					//1Ϊ����־��0Ϊ�ر���־
	int							log_type;					//1Ϊʵʱ��ʽ��0Ϊ�ļ�ģʽ

	FILE*    					log_file_handle;
	char						log_file_name[512];
	int							write_total;
	int 						pkgtotal_onesec;
	int 						bytetotal_onesec;
	int 						pkglostrate;
	int							avaragerate;
	int							bufferrate;
	int						    uplog_starttime;//xdl for shanghai
	int							buffer_index;
	sk_logbuffer_t				buffer_queue[SK_LOG_ARRAY_SIZE];
	sk_logfile_t				log_file[SK_LOG_FILE_ARRAY_SIZE];
}sk_shlog_t;    



/*
typedef struct alarm_s
{
	int 						alarm_switch;			//�Ƿ��Ƿ���	
	int 						alarm_level;			//�ϱ�����
	char 					cpu_alarm[20];			//�ϱ�����
	char						memory_alarm[20];		//�ϱ��ķ�����	
	int						disk_alarm;			//syslog��ʼ�ϱ���ʱ��	
	
	char 					band_width_alarm[20];	//����ʱ��
	char 					packet_lost_alarm[20];	//
	char                     			 errorcode[10];
	//�Ĵ�:
	char 					packet_lostframe_alarm[20];	//
	char 					packet_timedelay_alarm[20];	//
	char 					packet_cushion_alarm[20];	//
}sk_alarm_t;

*/


typedef struct sk_tr069_params_s
{
	int							upgrage_managed;		//�Ƿ�����tr069ģ���������	
	int							inform_starttime;		//�ϱ���ʼʱ��(����ʱ��)
	int 						inform_interval;		//�м���
	
	int							first_usenet;			//��һ������
	int							first_usenet_time;		//�������״γɹ������������ӵ����ں�ʱ�䡣
	int							inform_enable;			//�����Ƿ�����ʱ�ϱ�
	int							last_restart_time;		//���һ���������ʱ��
	int 						is_downloading;
	
	
	char						play_url[1024];
	int							is_play_test;
	int							play_status;
	int							value_changed;			//�Ƿ��Ѿ���������ֵ�仯
	int							value_changed_flag;
	
 	sk_ping_attribute_t 		ping;					//ping����
	sk_ping_results_t			ping_result;			//ping����

	sk_traceroute_attribute_t  	traceroute;				//traceroute����
	sk_traceroute_results_t		traceroute_result;		//traceroute����

    evcpe_packet_capture_t      packet_capture_param;   //Զ��ץ��

    evcpe_bandwidth_attribute_t bandwidth_diagnostics_param; //����������
    evcpe_bandwidth_results_t   bandwidth_diagnostics_result; //�����ⷴ��
	
	struct evcpe_download 		download_param;			//download���ز���
	struct evcpe_upload 		upload_param;			//�ϴ����ز���

    
	char                        buffer[1024];
	
	sk_alarm_t					alarm;
	sk_syslog_t					syslog;					
	sk_shlog_t					shlog;					//�Ϻ���־����


	int 						stack_index;
	int							power_down;				//����������
	int                                       xineng_inform;

}sk_tr069_params_t;





int evcpe_get_curtime(struct evcpe_attr *attr,
		const char **value, unsigned int *len);

int evcpe_set_curtime(struct evcpe_attr *attr,
		const char *buffer, unsigned int len);




#ifdef __cplusplus
}
#endif


#endif /* EVCPE_ACCESSOR_H_ */
