#include <stdlib.h>



#include <event2/event.h>   
#include <event2/util.h> 
#include <errno.h>

#include "stun.h"
#include "stun_bind.h"


#ifdef WIN32
#include "sk_misc.h"

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <fcntl.h>
#include "stun_ucr.h"
#include "log.h"

#include "evcpe.h"
#include "header.h"

#define STUN_DEFAULT_PERIODIC_TIMES     (120)       //STUN穿越周期时间 , 默认为120秒

#define SOCKET_RANDOM_PORT_START (49152)

#define SOCKET_RANDOM_PORT_END (65536)


static int Stun_Enable_Flag = 0;                    //stun 穿越 使能标志

static STUN_Context  m_stun_context = {0};

static void STUN_SendBindingRequest(STUN_Context *context);
static void timeout_cb(int fd, short event, void *arg);
static PACKAGE_TYPE STUN_GetPackageType(char *buffer);
static void STUN_Receive(STUN_Context *context);
static void STUN_Work(STUN_Context *context);
static bool STUN_CheckIPChanged(STUN_Context *context);
static bool STUN_ParseBindingResponse(STUN_Context *context, char *buf);
static int STUN_GetLocalIP(STUN_Context *context);


static unsigned char m_last_UDPConnectionRequestAddress[128] = {0};

static unsigned char m_UDPConnectionRequestAddress[128] = {0};

extern void STUN_set_UDPConnectionRequestAddress(unsigned char * STUNServerIp  , unsigned short STUNServerPort);

extern unsigned char * STUN_get_UDPConnectionRequestAddress(void);


extern void STUN_Dump_Parameter(STUN_Parameter *param)
{
#if 1
	evcpe_info(__func__,"STUNMaximumKeepAlivePeriod = %d\n",param->STUNMaximumKeepAlivePeriod);
	evcpe_info(__func__,"STUNMinimumKeepAlivePeriod = %d\n",param->STUNMinimumKeepAlivePeriod);

	evcpe_info(__func__,"STUNServerPort = %d\n",param->STUNServerPort);
	evcpe_info(__func__,"STUNServerAddress = %s\n",param->STUNServerAddress);

	evcpe_info(__func__,"STUNUsername = %s\n",param->STUNUsername);
	evcpe_info(__func__,"STUNPassword = %s\n",param->STUNPassword);

	evcpe_info(__func__,"ConnectionRequestUsername = %s\n",param->ConnectionRequestUsername);
	evcpe_info(__func__,"ConnectionRequestPassword = %s\n",param->ConnectionRequestPassword);

	evcpe_info(__func__,"STUNEnable = %d\n",param->STUNEnable);
#else
	printf("STUNMaximumKeepAlivePeriod = %d\n",param->STUNMaximumKeepAlivePeriod);
	printf("STUNMinimumKeepAlivePeriod = %d\n",param->STUNMinimumKeepAlivePeriod);

	printf("STUNServerPort = %d\n",param->STUNServerPort);
	printf("STUNServerAddress = %s\n",param->STUNServerAddress);

	printf("STUNUsername = %s\n",param->STUNUsername);
	printf("STUNPassword = %s\n",param->STUNPassword);

	printf("ConnectionRequestUserName = %s\n",param->ConnectionRequestUserName);
	printf("ConnectionRequestPassword = %s\n",param->ConnectionRequestPassword);

	printf("STUNEnable = %d\n",param->STUNEnable);
#endif

}


static void STUN_GetParams(STUN_Parameter * stunparam)
{
    STUN_Context*  context = STUN_get_object();
    if(!context || !stunparam)
    {
        evcpe_error(__func__ , "STUN_GetParams params is null!\n");
        return ;
    }
	memcpy(stunparam ,&(context->parameter), sizeof(STUN_Parameter));
}



static void STUN_TimeOut_Cb(int fd, short event, void *arg)
{
    STUN_Context*  context = (STUN_Context*)arg;
    if(NULL == context)
    {
        evcpe_info(__func__,"STUN_TimeOut_Cb error , param is error!\n");  
        return ;
    }
    
	evcpe_info(__func__,"STUN_TimeOut_Cb evtimer_add stun_nat_tv.tv_sec = %d\n" , context->stun_nat_tv.tv_sec);  
	STUN_SendBindingRequest(context);
	evtimer_add((struct event*)(&context->stun_nat_event), &context->stun_nat_tv);
}



static PACKAGE_TYPE STUN_GetPackageType(char *buffer)
{
	char data = 0;
	if(NULL == buffer )
	{
	    return PACKAGE_TYPE_NONE;
	}
	data =  buffer[0];
	if((data == 0)||(data == 1))
	{
		return PACKAGE_TYPE_STUN_RESPONSE;
	}
	else
	{
		return PACKAGE_TYPE_UDP_REQUEST;
	}
}

static void STUN_Receive(STUN_Context *context)
{
	char bufReq[REQ_IPCHANGE_LEN];
    char Recive_buff[256];
	PACKAGE_TYPE  packagetype;
	struct in_addr temp_addr;
    fd_set fds_read = {0};
    fd_set fds_error = {0};
    struct timeval timeout = {0};
    int ret = 0;
    static int count = 0;
    
	int nbyte = 0;
	int fd = context->fd;

	int addr_len = 0;
	struct sockaddr_in server_addr = {0};


    if(!context)
        return ;
    
	addr_len = sizeof(server_addr);
	memset(&server_addr, 0 ,addr_len);

    FD_ZERO(&fds_read); 
    FD_ZERO(&fds_error); 
    FD_SET(fd ,&fds_read);
    FD_SET(fd ,&fds_error);
    timeout.tv_sec = 0; 
    timeout.tv_usec = 0;
    
    ret = select(fd+1 ,&fds_read , NULL , &fds_error ,&timeout);

    if(ret > 0)
    {
         if(FD_ISSET(fd , &fds_read))
         {
            nbyte = recvfrom(fd, Recive_buff , sizeof(Recive_buff) , 0 ,(struct sockaddr *)&server_addr,&addr_len);

        	if(nbyte > 0)
        	{
                //HEX_INFO("STUN_Receive" , Recive_buff , nbyte);
        		packagetype = STUN_GetPackageType(Recive_buff);
        		switch(packagetype)
        		{
                	case PACKAGE_TYPE_UDP_REQUEST:
            			evcpe_info(__func__,"STUN_Receive  PACKAGE_TYPE_UDP_REQUEST \n");
            			if(STUN_UCR_Check(Recive_buff,nbyte,&(context->parameter.STUNUsername),&(context->parameter.STUNPassword)))
            			{
            				evcpe_debug(__func__,"STUN_Receive  PACKAGE_TYPE_UDP_REQUEST and begin to send  inform.\n");
                            evcpe_worker_add_msg(EVCPE_WORKER_CONNECTION_REQUEST);
            			}
            			break;

        			case PACKAGE_TYPE_STUN_RESPONSE:
        		
        			    evcpe_info(__func__,"STUN_Receive  PACKAGE_TYPE_STUN_RESPONSE \n");
        			    if(STUN_ParseBindingResponse(context,Recive_buff))
            			{
            		
            				context->localIP  = STUN_GetLocalIP(context);

                            //刚刚这个时候，有可能网线拔掉，获取的IP为null。
                            if(0 == context->localIP)
                            {
                                evcpe_error(__func__,"STUN_GetLocalIP is null ,not right! \n");
                                break;
                            }
                            
            				if(context->localIP != context->mappedIP)
            				{
            					context->result.nat_detected = 1;
            					context->result.STUNServerPort = context->mappedPort;
            					context->result.STUNServerIp = context->mappedIP;
       
            					temp_addr.s_addr = htonl(context->result.STUNServerIp);
            					sprintf(context->result.UDPConnectionRequestAddress,"%s:%u",inet_ntoa(temp_addr),context->result.STUNServerPort);
            				

            				    //LOGI("STUN_Receive  PACKAGE_TYPE_STUN_RESPONSE context->result.UDPConnectionRequestAddress:%s\n",context->result.UDPConnectionRequestAddress);
            				    //add by lijingchao
                                STUN_set_UDPConnectionRequestAddress(inet_ntoa(temp_addr) , context->result.STUNServerPort);
                                if(strcmp(m_last_UDPConnectionRequestAddress , m_UDPConnectionRequestAddress))
                                {
                                    strcpy(m_last_UDPConnectionRequestAddress , m_UDPConnectionRequestAddress);
                                    sk_set_nat_stun_flag(1);
                                    evcpe_worker_add_msg(EVCPE_WORKER_VALUE_CHANGE);
                                    evcpe_info(__func__,"STUN_Receive  PACKAGE_TYPE_STUN_RESPONSE  11 context->result.UDPConnectionRequestAddress:%s\n",context->result.UDPConnectionRequestAddress);
                          
                                }
                            }
            				else
            				{
            					// TODO: Check local port
            					context->result.nat_detected = 0;
            					context->result.STUNServerIp = context->localIP;
            					temp_addr.s_addr = context->result.STUNServerIp;
            				    sprintf(context->result.UDPConnectionRequestAddress,"%s:%u",inet_ntoa(temp_addr),context->result.STUNServerPort);
            		
                                 //add by lijingchao
                                STUN_set_UDPConnectionRequestAddress(inet_ntoa(temp_addr) , context->result.STUNServerPort);

                                if(strcmp(m_last_UDPConnectionRequestAddress , m_UDPConnectionRequestAddress))
                                {
                                    strcpy(m_last_UDPConnectionRequestAddress , m_UDPConnectionRequestAddress);
                                    sk_set_nat_stun_flag(0);
                                    evcpe_worker_add_msg(EVCPE_WORKER_VALUE_CHANGE);
                               	    evcpe_info(__func__,"STUN_Receive  PACKAGE_TYPE_STUN_RESPONSE  22 context->result.UDPConnectionRequestAddress:%s\n",context->result.UDPConnectionRequestAddress);
            			
                                }
                                //add end
            				}
            			
            			}
        			    break;

        			default:
        			    break;
        		}
        	}
         }
         else if(FD_ISSET(fd , &fds_error))
         {
            evcpe_info(__func__ , "select FD_ISSET fds_error errno=%d , strerror = %s!\n", errno, strerror(errno));
         }
    }
    else if(0 == ret) //超时
    {
        if(count++ >= 30) //30*300 = 9000
        {
            //evcpe_info(__func__ , "select timeout , can not find STUN response , errno=%d , strerror = %s!\n", errno, strerror(errno));
            if(STUN_CheckIPChanged(context))
            {
                evcpe_info(__func__ , "STUN_CheckIPChanged return true!\n");
                STUN_SendBindingRequest(context);
            }
            count = 0;
        }
    }
    else //错误
    {
        evcpe_info(__func__ , "select failed , errno=%d , strerror = %s!\n", errno, strerror(errno));

    }

	return;
}



static void STUN_Work(STUN_Context *context)
{

	int i = 0;
	int stun_periodic_times = 0;
	STUN_Parameter  stunparam = {0};
    
    pthread_detach(pthread_self());
    if(!context)
    {
        goto finally ;
    }
    
    STUN_Set_Enable();
	
	evtimer_set(&context->stun_nat_event, STUN_TimeOut_Cb, context); 
	evutil_timerclear(&context->stun_nat_tv); 
	STUN_GetParams(&stunparam);
    

	stun_periodic_times = stunparam.STUNMinimumKeepAlivePeriod/1000;    

    
    if(stun_periodic_times > STUN_DEFAULT_PERIODIC_TIMES || stun_periodic_times <= 0)
    {
        stun_periodic_times = STUN_DEFAULT_PERIODIC_TIMES;
    }

	evcpe_info(__func__,"STUN_Work nattime:%d, time2:%d \n" , stun_periodic_times ,stunparam.STUNMinimumKeepAlivePeriod /1000);

	
	context->stun_nat_tv.tv_sec = stun_periodic_times;
    
	event_add(&context->stun_nat_event, &context->stun_nat_tv);      

    for(i = 0 ; i < 3 ; i++)
    {
        STUN_SendBindingRequest(context);
        usleep(300*1000);
    }
  

	while(1)
	{
        if((0 == sk_tr069_get_init_status()) || (0 == STUN_Get_Enable()))
        {
            evcpe_info(__func__," STUN_Work run break; STUN_Get_Enable = %d\n" , STUN_Get_Enable());
            break;
        }
        
		STUN_Receive(context);
	    //LOGI("STUN_Receive  loop...333...\n");
		usleep(300*1000);
	}


    evtimer_del(&context->stun_nat_event);
    if(context->fd)
    {
        #ifdef WIN32
            closesocket(context->fd);
            context->fd = -1;
        #else
            close(context->fd);
            context->fd = -1;
        #endif
    }
    
    evcpe_info(__func__," STUN_Work return , Stun_Enable_Flag set 0\n");
    STUN_Set_Disable();    
finally:
    return;
}




bool STUN_start(struct evcpe *cpe, STUN_Parameter *param)
{
	pthread_t threadid = {0};
	int fd = -1;
    STUN_Context * context = STUN_get_object();
    int args = 0;
    
	evcpe_info(__func__,"STUN_start enter\n");

    //如果之前已经启动STUN功能，停止之前的STUN工，启动新的STUN ,因为新启动STUN端口号和地址可能有所改变
    if(STUN_Get_Enable())
    {
        evcpe_info(__func__,"stop old STUN\n");
        STUN_Set_Disable();
        sleep(2);
    }
  
    
	memset(context , 0 , sizeof(STUN_Context));
    
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		evcpe_error(__func__,"socket error\n");
        goto LAB_ERR;
	}
	context->cpe = cpe;
	memcpy(&(context->parameter),param ,sizeof(STUN_Parameter));
	context->fd = fd;
    context->localIP = getLocalIP(context->fd);

    {
         #if 0
         //针对某些平台内核，自动分配端口，不合理情况。
         struct sockaddr_in addrMe;
         unsigned short local_port = 0;
		 int ret = 0;

         srand(time(NULL));
		 local_port = rand() % (SOCKET_RANDOM_PORT_END - SOCKET_RANDOM_PORT_START) + SOCKET_RANDOM_PORT_START;
         memset(&addrMe,0,sizeof(addrMe));
         addrMe.sin_family = AF_INET;
         addrMe.sin_port = htons(local_port);
         addrMe.sin_addr.s_addr = INADDR_ANY; 
         ret = bind(fd, (struct sockaddr*)&addrMe, sizeof(addrMe));
         evcpe_info(__func__ ,"bind ret = %d; port = %d , errno = %s\n", ret,local_port , strerror(errno));
         printf("STUN_start bind ret = %d; port = %d , errno = %s\n", ret,local_port , strerror(errno));
         #endif
    }


    #ifdef WIN32
	{
		int blocking = 1;
		ioctlsocket(fd , FIONBIO, (unsigned long*)&blocking);
	}
    #else
	args = fcntl(fd, F_GETFL , 0);
	fcntl(fd , F_SETFL , args|O_NONBLOCK);
    #endif	


	pthread_create(&threadid, NULL, STUN_Work, (void *)context);

	evcpe_info(__func__,"STUN_start ok exit\n");
	return true;
    
LAB_ERR:
    evcpe_info(__func__,"STUN_start error exit\n");
    return false;
}

 
extern void STUN_OnCmd(struct evcpe *cpe, STUN_Parameter *param)
{

    //amlogic芯片 销毁线程，经常引起死机。在这里特殊处理
    if(1 == sk_tr069_get_call_shutdown_flag() && 1 == STUN_Get_Enable())
    {
        int i = 0;
        STUN_Context * context = STUN_get_object();
        evcpe_info(__func__ , "STUN_OnCmd sk_tr069_get_call_shutdown_flag() == 1 , STUN_Get_Enable() == 1");
        memset(m_last_UDPConnectionRequestAddress , 0 , sizeof(m_last_UDPConnectionRequestAddress));
        for(i = 0 ; i < 3 ; i++)
        {
            STUN_SendBindingRequest(context);
            usleep(300*1000);
        }
          
        return;
    }
    
    STUN_start(cpe , param);
    return;
}


static void STUN_SendBindingRequest(STUN_Context *context)
{
	int fd = context->fd;
	UInt32 ip = 0;
	char bufReq[REQ_LEN] = {0};
	unsigned short p = 0;
    bool ret = false;
	STUN_Parameter  stunparam = {0};

    
	STUN_GetParams(&stunparam);
	p = htons(stunparam.STUNServerPort);
    ip = (UInt32)inet_addr(stunparam.STUNServerAddress);

    //江苏移动要求为序列号:  
	evcpe_debug(__func__ , "STUN_SendBindingRequest  STUNUsername :%s\n", stunparam.STUNUsername);
    evcpe_debug(__func__ , "STUN_SendBindingRequest  sn :%s\n", stunparam.sn);
	buildTestBindingRequest(bufReq ,false , false ,&(context->ID), stunparam.sn ,false);
    
	ret = sendBindingRequest(bufReq ,fd ,ip , p ,REQ_LEN);
    evcpe_debug(__func__ , "sendBindingRequest %s\n", ret == false ? "failed" : " OK ");

}

static bool STUN_CheckIPChanged(STUN_Context *context)
{
	int local_ip = 0;
	local_ip = STUN_GetLocalIP(context);

    if(0 == local_ip)
    {
        evcpe_error(__func__, "STUN_GetLocalIP is null , not right!,may be network is not work!\n");
        goto finally;
    }
    
	if(local_ip != context->localIP)
	{
		context->localIP = local_ip;
        memset(m_last_UDPConnectionRequestAddress , 0 , sizeof(m_last_UDPConnectionRequestAddress));
		return true;
	}
    
finally:
	return false;

}


static bool STUN_ParseBindingResponse(STUN_Context *context, char *buf)
{
	Address4 addr = {0};	
	if(parseBindingResponse(buf,&addr,&(context->ID),1))
	{
		context->mappedIP = addr.mapped_ip;
		context->mappedPort = addr.mapped_port;
		
		return true;
	}
	else
	{
		
		return false;
	}
}

static int STUN_GetLocalIP(STUN_Context *context)
{
	return getLocalIP(context->fd);
		
}



extern void STUN_set_UDPConnectionRequestAddress(unsigned char * STUNServerIp ,unsigned short STUNServerPort)
{
  
   snprintf(m_UDPConnectionRequestAddress , sizeof(m_UDPConnectionRequestAddress)-1 , "%s:%u", STUNServerIp , STUNServerPort);
   evcpe_info(__func__,"STUN_set_UDPConnectionRequestAddress:%s\n",m_UDPConnectionRequestAddress);

}


extern unsigned char * STUN_get_UDPConnectionRequestAddress(void)
{
    return m_UDPConnectionRequestAddress;
}

extern unsigned char * STUN_get_Nat_Detected(void)
{
    static char m_nat_detected[32] = {0};
    STUN_Context* context =  STUN_get_object();
    if(1 == context->result.nat_detected)
    {
        memset(m_nat_detected , 0 , sizeof(m_nat_detected));
        strcpy(m_nat_detected , "true");
    }
    else
    {
        memset(m_nat_detected , 0 , sizeof(m_nat_detected));
        strcpy(m_nat_detected , "false");
    }
    return m_nat_detected;
}

extern void STUN_Set_Enable(void)
{
    Stun_Enable_Flag = 1;
}

extern void STUN_Set_Disable(void)
{
    Stun_Enable_Flag = 0;
}

extern int STUN_Get_Enable(void)
{
    return Stun_Enable_Flag;
}

extern STUN_Context*  STUN_get_object(void)
{
    return &m_stun_context;
}
