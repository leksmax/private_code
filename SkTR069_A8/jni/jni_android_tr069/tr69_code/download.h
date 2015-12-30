#ifndef  EVCPE_DOWNLOAD_H_
#define  EVCPE_DOWNLOAD_H_

#include "fault.h"

struct evcpe_download 
{
	char  			commandkey[32] ;		//当前事务交互的内容，参考 TR-069 的说明
    char  			filetype[64];			//下载文件类型 至少支持“1 Firmware Upgrade Image”、 X Firmware Directory
    char  			url[256];				//更新文件的 URL，通过文件下载接口获
    char  			username[256];			//机顶盒用于登录更新服务器的用户
    char  			password[256];			//机顶盒用于登录更新服务器的密码
    unsigned int 	filesize;				//文件大小（Byte 为单位）
    char  			targetfilename[256];	//需要下载的文件名
    unsigned int  	delayseconds;			//要求机顶盒延迟一段时间后建立下
    char  			successurl[256];		//
    char 			failureurl[256];
    char 			filename[64];			//需要下载的文件名
    char		 	file_path[256];
    char 			host[64];
};



struct evcpe_download_response
{
	int 	status;     		//0:表示成功，1:表示失败
	char 	commandkey[32] ;	//当前事务交互的内容，参考 TR-069 的说明
	char 	start_time[32];		//开始时间
	char 	complete_time[32];	//完成时间
};


struct evcpe_transfer_complete
{
	char				command_key[32];
	struct evcpe_fault	fault_struct;
	char	 			start_time[32];			//开始时间
	char 				complete_time[32];		//完成时间
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
