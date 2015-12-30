#ifndef  EVCPE_PACKET_CAPTURE_H_
#define  EVCPE_PACKET_CAPTURE_H_


/*
state 说明:
抓包状态，2用来ACS启动抓包任务，其他状态用于机顶盒向ACS反馈状态。包括：
1、未抓包 None 
2、开始抓包 Requested，ACS下发抓包命令时设为2 
3、正在抓包 
4、抓包失败 
5、正在上传 
6、上传完成
7、上传失败
*/

typedef struct evcpe_packet_capture_s{
	int   state;
    char  ip_addr[64];
    int   ip_port;
    int   duration;
    char  upload_url[256];
    char username[32];
    char password[32];
}evcpe_packet_capture_t;

int sk_start_packet_capture_diagnostics(evcpe_packet_capture_t * p_packet_capture);


#endif//EVCPE_PACKET_CAPTURE_H_

