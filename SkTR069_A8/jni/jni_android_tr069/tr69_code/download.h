#ifndef  EVCPE_DOWNLOAD_H_
#define  EVCPE_DOWNLOAD_H_

#include "fault.h"

struct evcpe_download 
{
	char  			commandkey[32] ;		//��ǰ���񽻻������ݣ��ο� TR-069 ��˵��
    char  			filetype[64];			//�����ļ����� ����֧�֡�1 Firmware Upgrade Image���� X Firmware Directory
    char  			url[256];				//�����ļ��� URL��ͨ���ļ����ؽӿڻ�
    char  			username[256];			//���������ڵ�¼���·��������û�
    char  			password[256];			//���������ڵ�¼���·�����������
    unsigned int 	filesize;				//�ļ���С��Byte Ϊ��λ��
    char  			targetfilename[256];	//��Ҫ���ص��ļ���
    unsigned int  	delayseconds;			//Ҫ��������ӳ�һ��ʱ�������
    char  			successurl[256];		//
    char 			failureurl[256];
    char 			filename[64];			//��Ҫ���ص��ļ���
    char		 	file_path[256];
    char 			host[64];
};



struct evcpe_download_response
{
	int 	status;     		//0:��ʾ�ɹ���1:��ʾʧ��
	char 	commandkey[32] ;	//��ǰ���񽻻������ݣ��ο� TR-069 ��˵��
	char 	start_time[32];		//��ʼʱ��
	char 	complete_time[32];	//���ʱ��
};


struct evcpe_transfer_complete
{
	char				command_key[32];
	struct evcpe_fault	fault_struct;
	char	 			start_time[32];			//��ʼʱ��
	char 				complete_time[32];		//���ʱ��
};


struct evcpe_transfer_complete_response
{
	int reserved;
};





struct  evcpe_download * evcpe_download_new(void);
void  evcpe_download_free(struct  evcpe_download * method);

struct evcpe_transfer_complete *evcpe_transfer_complete_new(void);
void evcpe_transfer_complete_free(struct evcpe_transfer_complete *method);

struct evcpe_transfer_complete_response *evcpe_transfer_complete_response_new(void);
void evcpe_transfer_complete_response_free(struct evcpe_transfer_complete_response *method);












struct evcpe_download_response *evcpe_download_response_new(void);

void evcpe_download_response_free(struct evcpe_download_response *method);

#endif
