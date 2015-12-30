#ifndef  EVCPE_UPLOAD_H_
#define  EVCPE_UPLOAD_H_


struct evcpe_upload
{
	char commandkey[64];
	char  filetype[64];
    char  url[256];
    char  username[256];
    char  password[256];
    unsigned int   delayseconds;
};

struct evcpe_upload * evcpe_upload_new(void);
void evcpe_upload_free(struct evcpe_upload * method);

struct evcpe_upload_response
{
	int status;
	int start_time[32];			//开始时间
	int complete_time[32];		//完成时间	
};

struct evcpe_upload_response * evcpe_upload_response_new(void);
void evcpe_upload_response_free(struct evcpe_upload_response * method);
#endif

