#ifndef  EVCPE_PACKET_CAPTURE_H_
#define  EVCPE_PACKET_CAPTURE_H_


/*
state ˵��:
ץ��״̬��2����ACS����ץ����������״̬���ڻ�������ACS����״̬��������
1��δץ�� None 
2����ʼץ�� Requested��ACS�·�ץ������ʱ��Ϊ2 
3������ץ�� 
4��ץ��ʧ�� 
5�������ϴ� 
6���ϴ����
7���ϴ�ʧ��
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

