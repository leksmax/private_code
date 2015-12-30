#include <stdbool.h>

#include <event2/event.h>   
#include <pthread.h>
#include "cpe.h"
#include "stun_bind.h"

#define STUN_STR_LEN (64)


typedef struct  STUN_Parameter_t
{
	char STUNServerAddress[STUN_STR_LEN];
	int STUNServerPort;
	bool   STUNEnable ;
	int STUNMaximumKeepAlivePeriod; 
	int STUNMinimumKeepAlivePeriod;
	char STUNUsername[STUN_STR_LEN];
	char STUNPassword [STUN_STR_LEN];
	int UDPConnectionRequestAddressNotificationLimit;
	char ConnectionRequestUsername[STUN_STR_LEN];
	char ConnectionRequestPassword[STUN_STR_LEN];
	void (*cbUdpConnectionReq)(void *param1, void *param2); 	// Uses this callback to send a infrom.
	void (*cbNotifyUdpAddress)(void *param1, void *param2); 	// Uses this callcack to send a "4-changed" infrom.
	char  sn[128];

}STUN_Parameter;


typedef struct STUN_Result_t
{
	int nat_detected;	
	char UDPConnectionRequestAddress[STUN_STR_LEN];
	int STUNServerIp;
	int STUNServerPort;
	int STUNResponseCode;

}STUN_Result;


typedef struct STUN_Context_t
{
	STUN_Parameter parameter;
	STUN_Result result;
    
	struct evcpe *cpe;
    struct event stun_nat_event;	 
    struct timeval stun_nat_tv;
	int stun_periodic_times;

	pthread_t  idThread;
//	sk_sem_id_t semParam;
//	sk_sem_id_t semSend;
	int  fd;

	UInt128 ID;
	int localIP;
	int mappedIP;
	int mappedPort;
}STUN_Context;

typedef enum 
{
    PACKAGE_TYPE_NONE ,
	PACKAGE_TYPE_UDP_REQUEST,
	PACKAGE_TYPE_STUN_RESPONSE ,
	PACKAGE_TYPE_UNDEFINED
} 
PACKAGE_TYPE;


extern void STUN_OnCmd(struct evcpe *cpe, STUN_Parameter *param);

extern void STUN_Dump_Parameter(STUN_Parameter *param);

extern void STUN_Set_Enable(void);

extern void STUN_Set_Disable(void);

extern int STUN_Get_Enable(void);

extern STUN_Context*  STUN_get_object(void);

