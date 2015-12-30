// $Id: accessor.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <pthread.h>
#endif



#include "accessor.h"
#include "header.h"
//#include "qos.h"
#include "sk_param_porting.h"
#include "stun.h"
#include "url.h"


#define SK_LOG_PATH  "/tmp"                     //日志文件保存目录




#define TR069_BUFFER_LEN            (256)       //TR069使用的缓冲区的长度
#define	TR069_PARAMS_LISTEN_ARRARY_SIZE  (15)	//最大修改参数个数

#define APPEND_INTEGER(buffer, name, value)    m_tr069_params.log_length+= sprintf(buffer + m_tr069_params.log_length, "%s|%s|%d\r\n",currtime, name, value)
#define APPEND_STRING(buffer, name, value)     do {  if (*value != '\0') m_tr069_params.log_length += sprintf(buffer + m_tr069_params.log_length, "%s|%s|%s\r\n",currtime, name, value); }while (0)




//static function
static int sk_start_upload_configure_file(struct evcpe *cpe , struct evcpe_upload *upload_param);
static int sk_start_download_configure_file(struct evcpe *cpe , struct evcpe_download *download_param);

//end

//局部静态变量
static sk_tr069_params_t			m_tr069_params={0};					//所有局部静态参数都通过这个结构来管理

static sk_change_table_t            m_chage_table[TR069_PARAMS_LISTEN_ARRARY_SIZE]={0};

static long m_alarmid=0;
static int  m_arlamtype=0;
static  int alarmlevel=0;

//add by lijingchao 
static int  g_nat_stun_flag = -1;

static int reset_acs_url_flag = 0;


typedef struct sk_api_net_realtime_info_s
{
    char ip[20];
    char netmask[20];
    char gateway[20];
    char dns1[20];
    char dns2[20];
} sk_api_net_realtime_info_t;


static sk_match_table_t             m_match_table[]=
{  //以下节点对应仅针对上海标清，其他各地对应可能不同，需要针对各地节点作不同对应 
	
	{0, "software_version","Device.DeviceInfo.SoftwareVersion"},
	{0, "tr069_local_user","Device.ManagementServer.ConnectionRequestUsername"},
	{0, "tr069_local_pwd","Device.ManagementServer.ConnectionRequestPassword"},
	{0, "tr069_upgrage","Device.ManagementServer.UpgradesManaged"},
	{0, "tr069_url","Device.ManagementServer.URL"},
	{0, "lan_ip","Device.LAN.IPAddress"},
	{0, "net_mode","Device.LAN.AddressingType"},
	{0, "mac_override","Device.LAN.MACAddressOverride"},
	{0, "mac","Device.LAN.MACAddress"},
	{0, "pppoe_user","Device.X_CTC_IPTV.ServiceInfo.PPPoEID"},
	{0, "pppoe_pwd","Device.X_CTC_IPTV.ServiceInfo.PPPoEPassword"},
	{0, "dhcp_user","Device.X_CTC_IPTV.ServiceInfo.DHCPID"},
	{0, "dhcp_pwd","Device.X_CTC_IPTV.ServiceInfo.DHCPPassword"},
	{0, "username","Device.X_CTC_IPTV.ServiceInfo.UserID"},
	{0, "password","Device.X_CTC_IPTV.ServiceInfo.UserPassword"},
	{0, "home_page","Device.X_CTC_IPTV.ServiceInfo.AuthURL"},
	{0, "home_page2","Device.X_CTC_IPTV.ServiceInfo.AuthURLBackup"},
	{0, "language","Device.X_CTC_IPTV.UserInterface.CurrentLanguage"},
	{0, "timezone","Device.Time.LocalTimeZone"}				
									
		
};

extern int 			sk_tr069_set_boot_status(int);
extern int 			sk_tr069_add_boot_status(int);
extern int			sk_tr069_reset_boot_status();
extern unsigned int sk_log_set_mac(char *mac);
extern unsigned int sk_log_set_product_type(char *product_type);

extern unsigned int sk_log_set_software_version(char *software_version);
extern unsigned int sk_log_set_product_type(char *product_type);
extern void 		sk_log_set_terminal(char is_ture);
extern int sk_get_logmsg(char* pinfo);

static char* _tr096_get_log_filename(char *filename,int length);
int sk_save_syslog_to_file(char *log_file,char* buf,int length);


char  reauth_event_code[20]={0};

#ifndef HD_PLATFORM

extern int 			ipanel_porting_system_upgrade(int s);


extern int 			sk_download_cfg(char* url,char* username,char* password);
extern int 			sk_upload_cfg(char* url,char* username,char* password);
extern int			sk_net_upgrade_start();

#else	//

extern int 			sk_download_cfg(char* url,char* username,char* password);

extern int 			sk_api_net_upgrade_start();

extern int 			sk_qos_start();

#endif

//add by lijingchao
//检测IP字符串的合法性
//return->	-1:invalid;	0:valid
//标准输入为"192.168.28.61"
int sk_network_check_ip(const char* IPString);

static int _tr069_set_log_info(int log_type,char *log_info,int log_value);


int sk_tr069_gettickcount()
{		
	struct tm * timeinfo;	
	time_t rawtime;		
	time ( &rawtime );	
	timeinfo=localtime ( &rawtime ); 
	return mktime(timeinfo);		//return time(NULL);
}

int sk_tr069_str_datetime_to_int(char *str)
{	
	struct tm t;	if(NULL == str || strlen(str) < 18)	
	{		
	return 0;	
	}	
	t.tm_year = (str[0] - '0') * 1000 + (str[1] - '0') * 100	            
		 	+ (str[2] - '0') * 10 + (str[3] - '0') - 1900;
	t.tm_mon = (str[5] - '0') * 10 + (str[6] - '0') - 1 ;	
	t.tm_mday = (str[8] - '0') * 10 + (str[9] - '0');
	t.tm_hour = (str[11] - '0') * 10 + (str[12] - '0');	
	t.tm_min = (str[14] - '0') * 10 + (str[15] - '0');
	t.tm_sec = (str[17] - '0') * 10 + (str[18] - '0');	
	return mktime(&t);	//return timegm(&t);
}


int sk_set_param_value_changed(const char *param_name)
{
	if(param_name==NULL)
		return 0;
	//:add by liangzhen
	if(!strcasecmp("skyworth.params.sys.software_version",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"software_version");
	}	
	else if(!strcasecmp("skyworth.params.tr069.local_user",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"tr069_local_user");
	}
	else if(!strcasecmp("skyworth.params.tr069.local_pwd",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"tr069_local_pwd");
	}
	else if(!strcasecmp("skyworth.params.tr069.upgrage",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"tr069_upgrage");
	}
	/*
	else if(!strcasecmp("skyworth.params.net.lan_ip",param_name))
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"tr069_url");
	*/
	else if(!strcasecmp("skyworth.params.net.lan_ip",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"lan_ip");
	}
	else if(!strcasecmp("skyworth.params.net.netmode",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"net_mode");
	}
	/*
	else if(!strcasecmp("xxx",param_name))
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"mac_override");
	*/
	else if(!strcasecmp("skyworth.params.sys.mac",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"mac");
	}
	else if(!strcasecmp("skyworth.params.net.pppoeusr",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"pppoe_user");
	}
	else if(!strcasecmp("skyworth.params.net.pppoepwd",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"pppoe_pwd");
	}
	/*
	else if(!strcasecmp("xxx",param_name))
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"dhcp_user");
	else if(!strcasecmp("xxx",param_name))
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"dhcp_pwd");
	*/
	else if(!strcasecmp("skyworth.params.itv.username",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"username");
	}
	else if(!strcasecmp("skyworth.params.itv.password",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"password");
	}
	else if(!strcasecmp("skyworth.params.itv.homepage",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"home_page");
	}
	//:by liangzhen @2012.11.19
	else if(!strcasecmp("skyworth.params.itv.homepage2",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"home_page2");
	}
	//:end
	/*
	else if(!strcasecmp("xxx",param_name))
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"home_page2");
	else if(!strcasecmp("xxx",param_name))
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"language");
	*/
	else if(!strcasecmp("skyworth.params.sys.timezone",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"timezone");
	}
	else if(!strcasecmp("skyworth.params.tr069.inform_period",param_name))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"inform_period");
	}
	else if(!strcasecmp("skyworth.params.tr069.natdetect","natdetect"))
	{
		m_tr069_params.stack_index++;
		strcpy(m_chage_table[m_tr069_params.stack_index].params_name,"natdetect");
	}
	//
	else
		evcpe_info(__func__ , "need not report,param:%s",param_name);
	
	//m_tr069_params.stack_index++;
	
	//strcpy(m_chage_table[m_tr069_params.stack_index].params_name,param_name);
	//:end

	evcpe_info(__func__ , "[sk_set_param_value_changed],index=%d,name=%s\n",m_tr069_params.stack_index,param_name);
	
	if(m_tr069_params.stack_index>=TR069_PARAMS_LISTEN_ARRARY_SIZE)
	{
		m_tr069_params.stack_index=-1;
	}
	
	return 0;
	
}	


//Device. X_CTC_IPTV.LogMsg.
int sk_is_node_logmsg(char *node_name)
{
//	printf("node_name:%s,log_enable:%d\n",node_name,m_tr069_params.shlog.log_enable);
 	if(node_name==NULL ||m_tr069_params.shlog.log_flag<1)
		return 0;
	
//	printf("----node_name:%s,p:%s\n",node_name,strstr(node_name,"LogMsg"));
	if(strstr(node_name,"LogMsg")!=NULL)
	{
		//printf("node_name:%s,log_enable:%d\n",node_name,m_tr069_params.shlog.log_enable);
		return 1;
	}
	return 0;
}


//判断本地参数发生改变是否需要上报
int sk_get_node_inform(char *node_name)
{

	int i;
	int size=sizeof(m_match_table)/sizeof(sk_match_table_t);
	
	if(node_name==NULL)
		return 0;

    //上传日志时，不需要上传valuechange
	if(m_tr069_params.shlog.log_flag>0)
		return 0;
	

	
	//if(m_tr069_params.value_changed<1)
	//	return 0;

	//if(m_tr069_params.value_changed_flag<1)
	//	return 0;

	
	for(i=0;i<size;i++)
	{
		if(!strcmp(node_name,m_match_table[i].tr069_param) &&
			(m_match_table[i].use_flag==1))
		{
		//	printf("i am ok!tr069_param=%s\n",m_match_table[i].tr069_param);
			//m_match_table[i].use_flag=2;   //已经置位，准备上报这项数据
			return 1;
		}	
	}

	return 0;
}



int sk_tr069_print_time(char *info)
{
	struct timeval tv = {0};
	struct tm* ptm = NULL;
	char time_buffer[36]={0};
	char time_string[16]={0};
	long milliseconds = 0;

	gettimeofday(&tv, 0);
	/* 获得日期时间，并转化为 struct tm?*/
	ptm = localtime(&tv.tv_sec);
	/* 格式化日期和时间，精确到秒为单位。*/
	strftime(time_string, sizeof(time_string), "%H:%M:%S", ptm);
	/* 从微秒计算毫秒。*/
	milliseconds = tv.tv_usec / 1000;
	/* 以秒为单位打印格式化后的时间日期，小数点后为毫秒。*/
	sprintf(time_buffer, "%s.%03ld", time_string, milliseconds);

	if(NULL != info)
	{
		evcpe_info(__func__ , "[tr069] time=%s,info=%s\n",time_buffer,info);
	}
	else
	{
		evcpe_info(__func__ , "[tr069] time=%s\n",time_buffer);
	}
	return 0;
}


int sk_tr069_init()
{
	int i = 0;
	m_tr069_params.first_usenet=1;
	m_tr069_params.is_downloading=0;

	m_tr069_params.alarm.alarm_switch=0;
	m_tr069_params.alarm.alarm_level=4;
	//m_tr069_params.alarm.disk_alarm=5;
	m_tr069_params.stack_index=-1;
	m_tr069_params.power_down=0;
	m_tr069_params.shlog.buffer_index=0;
	m_tr069_params.shlog.log_enable=-1;
	m_tr069_params.value_changed=0;
	
	strcpy(m_tr069_params.alarm.cpu_alarm,"5,10");
	strcpy(m_tr069_params.alarm.memory_alarm,"5,10");
	strcpy(m_tr069_params.alarm.band_width_alarm,"5,10");
	strcpy(m_tr069_params.alarm.packet_lost_alarm,"5,10");
	strcpy(m_tr069_params.alarm.disk_alarm,"5,10");
	/*
	sk_api_params_get_int("AlarmSwitch", &m_tr069_params.alarm.alarm_switch);
     */
	for(i=0;i<SK_LOG_ARRAY_SIZE;i++)
		m_tr069_params.shlog.buffer_queue[i].flag=0;

   return 0;
}

int sk_tr069_get_power_down_status()
{
	return m_tr069_params.power_down;
}

//1为待机,0为未置
int sk_tr069_set_power_down_status(int status)
{
	m_tr069_params.power_down=status;
	return 0;
}

sk_tr069_params_t *sk_tr069_get_param_object()
{
	return &m_tr069_params;
}

static struct sockaddr_in m_destaddr;
static int m_fd=-1;



//启动syslog上报
int sk_tr069_syslog_start(char *log_sever)
{
	
	char dev[256] = {0};
	char *p = NULL;
    char  m_mac[20]={0};					//mac地址
    char  m_product_type[32]={0};			//产品类型
    char  m_software_version[32]={0};			//产品类型

	sk_func_porting_params_get("mac",m_mac,sizeof(m_mac));
	sk_func_porting_params_get("product_type",m_product_type,sizeof(m_product_type));
	sk_func_porting_params_get("software_version",m_software_version,sizeof(m_software_version));

	strlcpy(dev, log_sever, sizeof(dev));
	p =  strchr(log_sever, ':');
	if(p == NULL )
	{
		strlcat(dev,":514", sizeof(dev));
	}
	
	evcpe_info(__func__ , "sk_tr069_syslog_start dev=%s \n",dev);


	{
		int fd = 0;
		int val = 0;
		int flags = 0;
		int ret = 0;
		char *sub_str = strstr(dev, ":");
		
		dev[sub_str - dev] = '\0';
		sub_str ++;

		//bzero(&m_destaddr, sizeof(m_destaddr));
		memset(&m_destaddr , 0 , sizeof(m_destaddr));
		m_destaddr.sin_family = AF_INET;
		m_destaddr.sin_port = htons(atoi(sub_str));
		m_destaddr.sin_addr.s_addr = inet_addr(dev);		
		
		fd  = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd == -1)
		{
			evcpe_info(__func__ , "[SK_LOG] socket creating err in udptalk\n");
			return -1;
		}

		val = 1;
		
		ret  = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*) &val, sizeof(val));
		if (ret == -1)
		{
			evcpe_info(__func__ , "[SK_LOG] Could not set socket to reuse mode!\n");
			close(fd);
			return -1;
		}

		ret =	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&val, sizeof(val));
		if (ret ==  -1)
		{
			evcpe_info(__func__ , "[SK_LOG] Could not set socket buffer!\n");
			close(fd);
			return -1;
		}

		/* set fd noblock */
		
		#ifdef WIN32
		{
			int blocking = 1;
			ioctlsocket(fd , FIONBIO, (unsigned long*)&blocking);
		}
    	#else
		flags = fcntl(fd, F_GETFL , 0);
		fcntl(fd , F_SETFL , flags|O_NONBLOCK);
    	#endif	
	
		m_fd = fd;
	}
	
	return 0;
}

//停止syslog上报
int sk_tr069_syslog_stop()
{
	if(m_fd > 0)
	{
		close(m_fd);
	}
	m_fd = -1;

	return 0;
}

int sk_tr069_set_commandkey(char *value,int len)
{
	char buffer_local[64] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == value || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local-1;
	memcpy(buffer_local,value,len_local);
	return sk_func_porting_params_set("tr069_commandkey",buffer_local);
}

int sk_tr069_get_commandkey(const char *value, unsigned int len)
{
	if(NULL == value && len<=0)
	{
		return -1;
	}
	sk_func_porting_params_get("tr069_commandkey",value,len);
	evcpe_info(__func__ , "sk_tr069_get_commandkey value=%s",value);
    
	return 0;	

}

//获取当前启动状态
int sk_tr069_get_boot_status(const char **value, unsigned int *len)
{
	static char buffer[32] = {0};
    memset(buffer , 0 , sizeof(buffer));
	sk_func_porting_params_get("tr069_event" ,buffer ,sizeof(buffer));
	*value=buffer;
	if(*value != NULL)
	{
         evcpe_info(__func__ , "[tr069]read tr069_boot=%s",buffer);
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


/*
//重置上报状态为:E_TR069_BOOT
int sk_tr069_reset_boot_status()
{

	//设置类型为:E_TR069_BOOT
	return sk_tr069_reset_boot_status();

}
*/


//以下是需要porting的接口函数

//设置升级服务器地址
DLLEXPORT_API int sk_tr069_set_upgrade_url(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[256] = {0};
	int  len_local = sizeof(buffer_local);
	int   ret = 0;
	if(NULL == buffer || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	
    evcpe_info(__func__ , "sk_tr069_set_upgrade_url = %s\n", buffer_local);
    
	sk_func_porting_params_set("tr069_upgrade_server",buffer_local);


    ret = sk_tr069_porting_upgrade(buffer_local);
		
	evcpe_debug(__func__,"add boot stauts");
    
	//sk_tr069_set_upgrade_status(EVCPE_DOWNLOADING);	//当前正在下载
	//sk_tr069_set_power_down_status(1);//关闭心跳
	//sk_tr069_add_boot_status(E_TR069_TRANSFER_COMPLETE);


    //add by lijingchao
    //date:2014-04-14
    //备份当前系统编译时间，用于升级判断，作为升级成功的判断依据。
    //如果开机的时候，编译时间相同，证明上次没有升级成功，不应该发TRANSFER_COMPLETE 事件
    {
        char buffer[128]={0};
        sk_func_porting_params_get("tr069_system_build_time" , buffer , sizeof(buffer));
        sk_func_porting_params_set("tr069_bak_system_build_time", buffer);
        sk_api_params_set("tr069_upgrade_flag","1");
        
    }
    //add end
    
    return ret;
	
}

//设置升级服务器地址
DLLEXPORT_API int sk_tr069_get_upgrade_url(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[256] = {0};

    memset(buffer_local , 0 , sizeof(buffer_local));
    
	sk_func_porting_params_get("tr069_upgrade_server",buffer_local,sizeof(buffer_local));

	*value = buffer_local;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}
	return 0;
}


//获得当前节目名称
DLLEXPORT_API int sk_tr069_get_service_name(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	
	static char buffer_local[256]={0};	

    memset(buffer_local , 0 , sizeof(buffer_local));
    
	*value = buffer_local;
    
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

//获得当前流服务器地址
DLLEXPORT_API int sk_tr069_get_stream_server(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};

    memset(buffer_local , 0 , sizeof(buffer_local));
	strcpy(buffer_local,"rtsp://127.0.0.1:554");

	*value = buffer_local;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;	
	}
	return 0;
}

// 获得cdn地址(内容服务器)
DLLEXPORT_API int sk_tr069_get_cdn_ip(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};

    memset(buffer_local , 0 , sizeof(buffer_local));
	strcpy(buffer_local , "http://127.0.0.1:8080");

	*value = buffer_local;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len=0;	
	}
	return 0;
}


//获得wifi状态,是否连通,1为连通，0为未连通
int sk_tr069_get_status_wifi(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return 0;
}


//获取地址类型Static,DHCP,PPPoE,UNKOWN
DLLEXPORT_API int sk_tr069_get_address_type(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	char buffer[16] = {0};
    static char buffer_local[16] = {0};
    memset(buffer_local , 0 , sizeof(buffer_local));
    
	sk_func_porting_params_get("net_type",buffer,sizeof(buffer));
	//net_type
	if(!strcasecmp(buffer,"static") || !strcasecmp(buffer,"lan"))
	{
		strcpy(buffer_local,"Static");
	}
	else if(!strcasecmp(buffer,"dhcp"))
	{
		strcpy(buffer_local,"DHCP");
	}
	else 
	{
		strcpy(buffer_local,"PPPoE");
	}
    
	*value = buffer_local;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_address_type(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int ret = 0;
	char buffer_local[64] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
    
    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local,buffer,len_local);


	if(!strcasecmp(buffer,"Static") || !strcasecmp(buffer,"lan"))
	{
		strcpy(buffer,"lan");
	}
	else if(!strcasecmp(buffer,"DHCP"))
	{
		strcpy(buffer,"dhcp");
	}
	else 
	{
		strcpy(buffer,"pppoe");
	}

	ret=sk_func_porting_params_set("net_type",buffer_local);

	return 0;
}



//获得机顶盒IP
DLLEXPORT_API int sk_tr069_get_stb_ip(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};
    memset(buffer_local , 0 , sizeof(buffer_local));
	sk_func_porting_params_get("lan_ip", buffer_local, sizeof(buffer_local));
	*value = buffer_local;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


//获得机顶盒子网掩码
DLLEXPORT_API int sk_tr069_get_stb_mask(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};
    memset(buffer_local , 0 , sizeof(buffer_local));
	sk_func_porting_params_get("lan_mask" ,buffer_local ,sizeof(buffer_local));
	*value = buffer_local;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}



//获得机顶盒子网掩码
DLLEXPORT_API int sk_tr069_get_stb_gateway(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};

    memset(buffer_local , 0 , sizeof(buffer_local));
	sk_func_porting_params_get("gateway", buffer_local ,sizeof(buffer_local));

	*value = buffer_local;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}


	

//获得机顶盒dns
DLLEXPORT_API int sk_tr069_get_stb_dns(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};

	memset(buffer_local , 0 , sizeof(buffer_local));
	sk_func_porting_params_get("dns1",buffer_local,sizeof(buffer_local));
	*value = buffer_local;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


//获得机顶盒mac
DLLEXPORT_API int sk_tr069_get_stb_mac(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};

    memset(buffer_local , 0 , sizeof(buffer_local));
	sk_func_porting_params_get("mac",buffer_local,sizeof(buffer_local));
	*value = buffer_local;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}
	return 0;
}

//获得机顶盒sn
DLLEXPORT_API int sk_tr069_get_stb_sn(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};

    memset(buffer_local , 0 , sizeof(buffer_local));
	sk_func_porting_params_get("sn", buffer_local , sizeof(buffer_local));


	*value = buffer_local;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}



//获取应用程序软件版本
int sk_tr069_get_app_version(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[64] = {0};
	memset(buffer_local , 0 , sizeof(buffer_local));	
	sk_func_porting_params_get("software_version", buffer_local , sizeof(buffer_local));


	*value = buffer_local;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}
	return 0;
}


//获得码率
int sk_tr069_get_bit_rate(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32] = {0};
	
	memset(buffer, 0 ,sizeof(buffer));

	strcpy(buffer ,"0");
	*len = strlen(buffer);
	*value = buffer;

	return 0;
}


//从配置获取是否定时上报
DLLEXPORT_API int sk_tr069_get_inform_enable(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[16] = {0};

	memset(buffer, 0 ,sizeof(buffer));
	
	sk_func_porting_params_get("tr069_inform" , buffer , sizeof(buffer));

	*value = buffer;
    
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_set_inform_enable(struct evcpe_attr *attr,const char *buffer , unsigned int len)
{

    struct evcpe *cpe = get_cpe_object();

    char buffer_local[8]={0};

    int  len_local = sizeof(buffer_local);

    if (NULL == buffer || len <= 0)
	{
		return -1;
	}
    
    len_local = len_local>len ? len:len_local-1;
    
	memcpy(buffer_local , buffer , len_local);
	
	evcpe_info(__func__ , "set tr069_inform = %s\n" , buffer_local);
    
    evcpe_disable_periodic_inform(cpe);
    
    if(!strcmp("1",buffer_local) || !strcmp("true",buffer_local) || !strcmp("TRUE",buffer_local))
    {
         evcpe_enable_periodic_inform(cpe);
    }
    
	return sk_func_porting_params_set("tr069_inform", buffer_local);
}


//获取定时上报频率
DLLEXPORT_API int sk_tr069_get_period_inform_interval(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32] = {0};
	
	memset(buffer,0,sizeof(buffer));
    
	sk_func_porting_params_get("tr069_inform_period",buffer,sizeof(buffer));
	
	*len=strlen(buffer);

    *value = buffer;
	
	
	return 0;
}


	

DLLEXPORT_API int sk_tr069_set_period_inform_interval(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[32] = {0};
    int inform_enable = 0;
    unsigned char *value = NULL;
    int value_len = 0;
    
    if( NULL == buffer || len <= 0)
	{
		return -1;
	}
    
	len = sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	sk_func_porting_params_set("tr069_inform_period" , buf);

    sk_tr069_get_inform_enable(NULL , &value , &value_len);	

	if(NULL != value && len > 0)
	{
		sscanf(value ,"%d" , &inform_enable);
	}
    else
    {
		inform_enable = 1;
    }

    if(1 == inform_enable)
    {
        struct evcpe *cpe = get_cpe_object();
        evcpe_disable_periodic_inform(cpe);
        evcpe_enable_periodic_inform(cpe);
    }
    
    return 0;
}


//决定机顶盒发起 Inform 方法调用的时间参考绝对值。 每个 Inform 调用必须在 PeriodicInformInterval 加或减此参考时间的整数倍时进行。
DLLEXPORT_API int sk_tr069_get_period_inform_time(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[32]={0};
	time_t current;
	
	memset(buffer_local,0,sizeof(buffer_local));	

	
	current = time(NULL);
	strftime(buffer_local, sizeof (buffer_local), "%Y-%m-%dT%H:%M:%S", localtime(&current));

	//实际应该用这个
	m_tr069_params.inform_starttime = sk_tr069_str_datetime_to_int(buffer_local);


	*len=strlen(buffer_local);
	*value = buffer_local;
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_period_inform_time(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[32]={0};
	int  len_local = sizeof(buffer_local);
	if(NULL == buffer || len <= 0 )
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

	m_tr069_params.inform_starttime=sk_tr069_str_datetime_to_int(buffer_local);

	return 0;
}


//获取认证地址
DLLEXPORT_API int sk_tr069_get_authurl(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

    char tempbuf[256] = {0};
	static char buffer[1024]={0};

	sk_func_porting_params_get("home_page" , buffer , sizeof(buffer));   
	sk_func_porting_params_get("home_page2", tempbuf, sizeof(tempbuf));

    if(0 != tempbuf[0])
   	{
		strcat(buffer , ",");
		strcat(buffer , tempbuf);
   	}
	*value = buffer;
	if(*value!=NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}

	return 0;
}


DLLEXPORT_API int sk_tr069_set_authurl(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int i=0;
	char* ptrmp=NULL;
	char buffer_local[2048]={0};
	int  len_local=sizeof(buffer_local);
	char *p =  NULL;

	if ( NULL == buffer|| len <= 0)
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local;

	strncpy(buffer_local, buffer, len_local);
	buffer_local[len_local] = '\0';

	p =  strchr(buffer_local,'<');
	if(p != NULL)
	{
		*p = '\0';
	}
	
	p =  strchr(buffer_local,',');
	if(p != NULL)
	{
		*p = '\0';
	
		sk_func_porting_params_set("home_page2",p+1);
	
	}
	sk_func_porting_params_set("home_page",buffer_local);
	return 0;
}



//获取认证用户名
DLLEXPORT_API int sk_tr069_get_authuser(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64] = {0};
	memset(buffer , 0 , sizeof(buffer));
	sk_func_porting_params_get("username",buffer,sizeof(buffer));

	*value=buffer;
  
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_authuser(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[64]={0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local-1;
	memcpy(buffer_local , buffer , len_local);
	return sk_func_porting_params_set("username" , buffer_local);
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_authpass(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
	
	memset(buffer , 0  , sizeof(buffer));
	sk_func_porting_params_get("password",buffer,sizeof(buffer));
    
	*value = buffer;
	if(*value!=NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

DLLEXPORT_API int sk_tr069_set_authpass(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[64] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memset(buffer , 0 , sizeof(buffer));
	strncpy(buffer_local , buffer , len_local);

	return sk_func_porting_params_set("password",buffer_local);

}




DLLEXPORT_API int sk_tr069_get_upgrade_manage(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[16] = {0};
	
	memset(buffer , 0 , sizeof(buffer));
	sk_func_porting_params_get("tr069_upgrage",buffer,sizeof(buffer));
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	//这里不知道前端下发的是0,1还是true,false，现暂按0,1处理
	if(*value!=NULL)
	{
		sscanf(*value , "%d" , &m_tr069_params.upgrage_managed);
	}	

	return 0;
}

DLLEXPORT_API int sk_tr069_set_upgrade_manage(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16]={0};
	
	int  len_local=sizeof(buffer_local);
	
	if (NULL == buffer|| len <= 0)
	{
		return -1;
	}
	
	len_local = len_local>len? len:len_local-1;
	memcpy(buffer_local , buffer , len_local);
	return sk_func_porting_params_set("tr069_upgrage",buffer_local);

}

//针对江苏移动项目 网管地址主备地址切换
static char m_reauth_tr069_acs[512] = {0};

unsigned char* sk_tr069_get_reauth_tr069_acs(void)
{
    return m_reauth_tr069_acs;
}

void sk_tr069_set_reauth_tr069_acs(unsigned char *url)
{

    int len = 0;
    int len_local = 0;
	if(NULL== url || strlen(url)<=0)
    {
        LOGI("sk_tr069_set_reauth_tr069_acs failed! NULL== url or strlen(url) <=0\n");
        return;
	}
    
    memset(m_reauth_tr069_acs , 0 , sizeof(m_reauth_tr069_acs));
    len =  strlen(url);
    len_local = sizeof(m_reauth_tr069_acs);
	len_local = len_local>len ? len: len_local-1;
    
	strncpy(m_reauth_tr069_acs ,url ,len_local);
    
    evcpe_info(__func__ , "sk_tr069_set_reauth_tr069_acs m_reauth_tr069_acs = %s\n",m_reauth_tr069_acs);
      
}


//获取acs地址
DLLEXPORT_API int sk_tr069_get_acsurl(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512]={0};
	
	memset(buffer,0,sizeof(buffer));

	sk_func_porting_params_get("tr069_acs",buffer,sizeof(buffer));

	*value = buffer;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_acsurl(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[512]={0};
	int  len_local = sizeof(buffer_local);
	if (buffer==NULL || len<=0 )
	{
		return -1;
	}

    len_local = len_local>len ? len:len_local-1;
	memcpy(buffer_local, buffer , len_local);

    evcpe_debug(__func__,"[sk_tr069_set_acsurl]buffer_local = %s !\n",buffer_local);
    

    sk_func_porting_params_set("tr069_acs",buffer_local);
    evcpe_info(__func__ , "[sk_tr069_set_acsurl] set worktimer_type EVCPE_WORKER_REAUTH !\n");

    sk_tr069_set_reauth_tr069_acs(buffer_local);
    
    evcpe_worker_add_msg(EVCPE_WORKER_REAUTH);
	
	return 0;
}

//获取acs地址
DLLEXPORT_API int sk_tr069_get_acsbakurl(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512] = {0};
	
	memset(buffer,0,sizeof(buffer));

    sk_func_porting_params_get("tr069_bak_acs",buffer,sizeof(buffer));
	
	*value = buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_acsbakurl(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[512] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local-1;
	memcpy(buffer_local , buffer , len_local);
    evcpe_info(__func__ , "value:%s len:%d!\n",buffer_local , len);
    sk_func_porting_params_set("tr069_bak_acs" , buffer_local);
    
	return 0;
}

//获取platform url 地址
DLLEXPORT_API int sk_tr069_get_platform_url(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512] = {0};
	
	memset(buffer,0,sizeof(buffer));

    sk_func_porting_params_get("tr069_platform_url",buffer,sizeof(buffer));
	
	*value = buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}



DLLEXPORT_API int sk_tr069_set_platform_url(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[512] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

   	evcpe_info(__func__ , "value:%s len:%d!\n",buffer_local,len);
    sk_func_porting_params_set("tr069_platform_url",buffer_local);
	return 0;
}

//获取platform back url 地址
DLLEXPORT_API int sk_tr069_get_platform_bak_url(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512] = {0};

	memset(buffer,0,sizeof(buffer));

    sk_func_porting_params_get("tr069_platform_bak_url",buffer,sizeof(buffer));
	
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}



DLLEXPORT_API int sk_tr069_set_platform_bak_url(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[512] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	
	len_local = len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);
	
   	evcpe_info(__func__ , "value:%s len:%d!\n",buffer_local , len);

    sk_func_porting_params_set("tr069_platform_bak_url",buffer_local);
    
	return 0;
}


//获取HDC url 地址
DLLEXPORT_API int sk_tr069_get_hdc_url(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512] = {0};
	
	memset(buffer , 0 , sizeof(buffer));
    
	sk_func_porting_params_get("tr069_hdc_url",buffer,sizeof(buffer));
	
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}



DLLEXPORT_API int sk_tr069_set_hdc_url(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[512] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);
    
	evcpe_info(__func__ , "value:%s len:%d!\n" , buffer_local , len);
    
	sk_func_porting_params_set("tr069_hdc_url",buffer_local);
	
	return 0;
}

//获取tr069_forced_upgrade
int sk_tr069_get_forced_upgrade(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512]={0};
	memset(buffer,0,sizeof(buffer));
    sk_func_porting_params_get("tr069_forced_upgrade",buffer,sizeof(buffer));
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}



int sk_tr069_set_forced_upgrade(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[512]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;
	
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

    LOGI("[sk_tr069_set_forced_upgrade] value:%s len:%d!\n",buffer_local , len);
    
    sk_func_porting_params_set("tr069_forced_upgrade",buffer_local);
	return 0;
}



//获取tr069_silent_upgrade
DLLEXPORT_API int sk_tr069_get_silent_upgrade(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[512]={0};
	
	memset(buffer,0,sizeof(buffer));
    
	sk_func_porting_params_get("tr069_silent_upgrade", buffer , sizeof(buffer));
	
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}



DLLEXPORT_API int sk_tr069_set_silent_upgrade(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[512]={0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}

	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
   	evcpe_info(__func__ , "value:%s len:%d!\n",buffer_local,len);

    sk_func_porting_params_set("tr069_silent_upgrade",buffer_local);
	
	return 0;
}


//end


//获取认证用户名
DLLEXPORT_API int sk_tr069_get_acsuser(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
	
	memset(buffer , 0 , sizeof(buffer));
	
	sk_func_porting_params_get("tr069_user2",buffer,sizeof(buffer));
	
	*value = buffer;
	
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}	
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_acsuser(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[64] = {0};
	int  len_local=sizeof(buffer_local);
	if (NULL == buffer|| len<=0)
	{
		return -1;
	}
	len_local=len_local>len ? len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	sk_func_porting_params_set("tr069_user2",buffer_local);
	return 0;
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_acspass(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
    
	sk_func_porting_params_get("tr069_pwd",buffer,sizeof(buffer));

	*value = buffer;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_acspass(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[64] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local-1;
	memcpy(buffer_local , buffer , len_local);
	sk_func_porting_params_set("tr069_pwd",buffer_local);
    return 0;
}


//获取acs访问stb的地址
DLLEXPORT_API int sk_tr069_get_request_url(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[256] = {0};
	char now_ip[64] = {0};
    
    sk_func_porting_params_get("lan_ip",now_ip,sizeof(now_ip));
    
    memset(buffer , 0 , sizeof(buffer));
	sprintf(buffer , "http://%s:7546",now_ip);
	evcpe_info(__func__ , "[tr069] sk_tr069_get_request_url =%s!\n",buffer);
	*len = strlen(buffer);
	*value = buffer;
	return 0;
}


//获取acs访问stb的用户名,目前没有做判断，临时写了个
DLLEXPORT_API int sk_tr069_get_requser(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

	static char buffer[128]={0};
	memset(buffer , 0  ,sizeof(buffer));
	
	sk_func_porting_params_get("tr069_local_user",buffer,sizeof(buffer));
	*value = buffer;
	if(*value!=NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}



DLLEXPORT_API int sk_tr069_set_requser(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[128] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len <= 0)
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local , buffer , len_local);
    evcpe_info(__func__ , "sk_tr069_set_requser = %s", buffer_local);
	
	sk_func_porting_params_set("tr069_local_user",buffer_local);
	return 0;
}



//获取acs访问stb的密码,目前没有做判断，临时写了个
DLLEXPORT_API int sk_tr069_get_reqpass(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	
	static char buffer[128] = {0};
    //上海电信规范要求，上报时，密码设为空

	memset(buffer , 0  ,sizeof(buffer));
	sk_func_porting_params_get("tr069_local_pwd",buffer,sizeof(buffer));
	//strcpy(buffer,"ac5entry");

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

    //add by lijingchao
    //修正 江苏移动，华为网管 问题
    //date:2014-05-19
    //根据TR069规范，Device.ManagementServer.ConnectionRequestPassword是不应该主动上报的，即使终端网管查询，终端也应该返回空，而不是具体值
    if(NULL != attr)
    {
        *len=0;
    }
    //add end

	return 0;
}


DLLEXPORT_API int sk_tr069_set_reqpass(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[128]={0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
    evcpe_info(__func__ , "sk_tr069_set_reqpass = %s\n" , buffer_local );
	sk_func_porting_params_set("tr069_local_pwd",buffer_local);

	return 0;
}

//获取硬件版本信息
DLLEXPORT_API int sk_tr069_get_hd_version(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
	memset(buffer , 0 , sizeof(buffer));
	sk_func_porting_params_get("hardware_version",buffer,sizeof(buffer));


	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

//获取软件版本信息
DLLEXPORT_API int sk_tr069_get_sw_version(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64] = {0};
	memset(buffer , 0  ,sizeof(buffer));
	sk_func_porting_params_get("software_version",buffer,sizeof(buffer));
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


//获取STBID
DLLEXPORT_API int sk_tr069_get_stbid(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64] = {0};
	int ulen  = 0;
    int i = 0;
	memset(buffer , 0  ,sizeof(buffer));
	sk_func_porting_params_get("sn",buffer,sizeof(buffer));
    ulen = strlen(buffer);
    if(ulen < 32)
    {       
        for(i = 0 ; i<32 ; i++)       
        {               
            if(buffer[i]=='\0')             
            {                       
                buffer[i] = 0x30;             
            }
        }
    }
    
	*value = buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}	
	return 0;
}

DLLEXPORT_API int sk_tr069_get_product_type(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64] = {0};
	
	memset(buffer , 0  ,sizeof(buffer));
	sk_func_porting_params_get("product_type",buffer,sizeof(buffer));

	*value = buffer;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
    
	return 0;
}

//获取认证类型
DLLEXPORT_API int sk_tr069_get_authtype(struct evcpe_attr *attr,char **value,int *len)
{
	static char buffer[16]={0};
	
	memset(buffer,0,sizeof(buffer));	

	strcpy(buffer,"NONE");
	
	*len=strlen(buffer);
	
	*value = buffer;
	
	
	return 0;
}





//从机顶盒最后一次重启后的时间（以秒计）。
int sk_tr069_set_last_restart_time()
{
	time_t current = {0};
	
    struct tm *timeptr = NULL;

	current = time(NULL);

	m_tr069_params.last_restart_time = (int)current;

	return 0;
}


DLLEXPORT_API int sk_tr069_get_play_status(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	char *status = NULL;
	int  speed = 0;
	static char buffer[32] = {0};
    
	if(!m_tr069_params.is_play_test)  //当前没有进行播控测试
	{
		m_tr069_params.play_status=1;
	}
	
	memset(buffer , 0 , sizeof(buffer));
	
	sprintf(buffer,"%d",m_tr069_params.play_status);

	*len=strlen(buffer);
	
	*value = buffer;


	return 0;
}



DLLEXPORT_API int sk_tr069_get_up_time(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	int  		up_time = 0 ;
	static char buffer[32] = {0};
	
	
	time_t current = {0};
    struct tm *timeptr = NULL ;

	current = time(NULL);

	//取得最后一次启动的时间，与当前时间的差值
	up_time=current-m_tr069_params.last_restart_time;
	memset(buffer,0,sizeof(buffer));	
	sprintf(buffer,"%d",up_time);
	*len=strlen(buffer);
	*value = buffer;
	
	return 0;
}




//机顶盒首次成功建立网络连接的日期和时间。
int sk_tr069_set_first_usenet_time()
{
	time_t current = 0;
    struct tm *timeptr = NULL;
	char	buffer[32] = {0};
	current = time(NULL);

	m_tr069_params.first_usenet_time=(int)current;

	sprintf(buffer,sizeof(buffer),"%d",m_tr069_params.first_usenet_time);
	

	return sk_func_porting_params_set("tr069_usenettime",buffer);
}


DLLEXPORT_API int sk_tr069_get_first_use_date(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32] = {0};
	
	memset(buffer,0,sizeof(buffer));	
	
	strftime(buffer, sizeof (buffer), "%Y-%m-%dT%H:%M:%S", localtime(&m_tr069_params.first_usenet_time));
	
	*len=strlen(buffer);
	
	*value = buffer;
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_ntp_server(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char _buffer[32] = {0};
	int  length=sizeof(_buffer);
	
	if (NULL == buffer|| len <= 0)
	{
		return -1;
	}
	length = length>len ? len:length-1;

	strncpy(_buffer,buffer,length);
	
	return sk_func_porting_params_set("ntp_server",_buffer);

}	

//第一个 NTP 时间服务器。可以为域名或 IP 地址，由业务管理平台通过Authentication.CTCSetConfig（“NPTDomain”，“NTPServer1”）方法进行配置。当发生改变时，需立即上报终端管理平台
DLLEXPORT_API int sk_tr069_get_ntp_server(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32] = {0};

	memset(buffer , 0 ,sizeof(buffer));	
	sk_func_porting_params_get("ntp_server",buffer,sizeof(buffer));

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_dhcpid(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char _buffer[32] = {0};
	int  length = sizeof(_buffer);
	
	if (NULL == buffer|| len <= 0)
	{
		return -1;
	}
	length = length>len?len:length-1;

	strncpy(_buffer,buffer,length);
	
	
	return sk_func_porting_params_set("dhcp_user",_buffer);
}


DLLEXPORT_API int sk_tr069_get_dhcpid(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
	

	memset(buffer, 0 ,sizeof(buffer));	
	sk_func_porting_params_get("dhcp_user", buffer , sizeof(buffer));


	if(strlen(buffer)<=0)
	{
		strcpy(buffer,"1");
	}

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_dhcppwd(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char _buffer[64] = {0};
	int  length = sizeof(_buffer);
	
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	length = length>len?len:length-1;

	strncpy(_buffer,buffer,length);
	return sk_func_porting_params_set("dhcp_pwd",_buffer);

}


DLLEXPORT_API int sk_tr069_get_dhcppwd(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	memset(buffer,0,sizeof(buffer));	
	
	sk_func_porting_params_get("dhcp_pwd",buffer,sizeof(buffer));

	if(strlen(buffer)<=0)
	{
		strcpy(buffer,"1");
	}

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;	
	}
	return 0;
}



DLLEXPORT_API int sk_tr069_get_ForceUpgrade(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	

	memset(buffer,0,sizeof(buffer));	
	
	sk_api_params_get("force_upgrade",buffer,sizeof(buffer));


	if(strlen(buffer)<=0)
	{
		strcpy(buffer,"1");
	}

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_ForceUpgrade(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[32]={0};
	int  length = sizeof(local_buffer);
	
	if (NULL == buffer || len<=0 )
	{
		return -1;
	}
	length = length>len?len:length-1;

	strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "sk_tr069_set_ForceUpgrade = %s " , local_buffer);
	
	

	return sk_api_params_set("force_upgrade", local_buffer);

}

DLLEXPORT_API int sk_tr069_set_pppoepass(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[32] = {0};
	int  length=sizeof(local_buffer);
	
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	length=length>len?len:length-1;

	strncpy(local_buffer ,buffer,length);
	
	return sk_api_params_set("pppoe_pwd",local_buffer );

}	


DLLEXPORT_API int sk_tr069_get_pppoepass(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
	
	memset(buffer , 0 , sizeof(buffer));
   	sk_func_porting_params_get("pppoe_pwd",buffer,sizeof(buffer));

   	evcpe_info(__func__ , "buffer=%s\n",buffer);

   	*value = buffer;
    
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	
	return 0;
}

	

DLLEXPORT_API int sk_tr069_get_pppoeid(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64] = {0};
	
	memset(buffer , 0 , sizeof(buffer));
	
	sk_func_porting_params_get("pppoe_user",buffer,sizeof(buffer));
    /*
	if(strlen(buffer)<=0)
	{
		strcpy(buffer,"1");
	}
    */
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

DLLEXPORT_API int sk_tr069_set_pppoeid(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char _buffer[32] = {0};
	int  length = sizeof(_buffer);
	
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	length = length>len?len:length-1;

	strncpy(_buffer,buffer,length);
	

	return sk_api_params_set("pppoe_user",_buffer);

}	



//机顶盒本地时区中的当前日期和时间
DLLEXPORT_API int sk_tr069_get_current_time(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	time_t current = {0};
	
	memset(buffer,0,sizeof(buffer));	
	
	current = time(NULL);

	strftime(buffer, sizeof (buffer), "%Y-%m-%dT%H:%M:%S", localtime(&current));
	
	*len=strlen(buffer);
	
	*value = buffer;

	
	return 0;
}

//本地时间与 UTC 的偏差，形式如下："+hh:mm" "－hh:mm"。当发生改变时，需立即上报终端管理平台。
DLLEXPORT_API int sk_tr069_get_time_zone(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));	
	
	sk_func_porting_params_get("timezone",buffer,sizeof(buffer));
	
	*value = buffer;
	
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


//日志文件服务器的URL信息，应该包含上传URL的鉴权信息
DLLEXPORT_API int sk_tr069_get_log_uploadurl(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[256]={0};
	memset(buffer,0,sizeof(buffer));	
	sk_func_porting_params_get("upload_url",buffer,sizeof(buffer));

	*value = buffer;
	if(*value != NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_set_log_uploadurl(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[256]={0};
	int  len_local=sizeof(buffer_local);
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);
	return sk_func_porting_params_set("log_server",buffer_local);
}



DLLEXPORT_API int sk_tr069_get_log_uploadinterval(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[16] = {0};

	memset(buffer_local,0,sizeof(buffer_local));	
	
	sk_func_porting_params_get("upload_interval",buffer_local,sizeof(buffer_local));

	*value=buffer_local;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_log_uploadinterval(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16]={0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

	return sk_func_porting_params_set("upload_interval",buffer_local);

}

DLLEXPORT_API int sk_tr069_get_log_recordinterval(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[16]={0};
	memset(buffer_local,0,sizeof(buffer_local));	
	sk_func_porting_params_get("record_interval",buffer_local,sizeof(buffer_local));

	*value = buffer_local;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


//四川:
/*
int sk_tr069_set_log_enable(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
		if ( (buffer==NULL) || 	(len<=0) )
		  return -1;
}
int sk_tr069_get_log_enable(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
}
*/
DLLEXPORT_API int sk_tr069_set_log_user(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int  len_local = sizeof(m_tr069_params.syslog.log_user);
	if (NULL == buffer || len<=0)
	{
		   return -1;
	}
	
	len_local = len_local>len?len:len_local-1;
	
	
	memcpy(m_tr069_params.syslog.log_user,buffer,len_local);

	return 0;
}

DLLEXPORT_API int sk_tr069_get_log_user(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[256]={0};	

	memset(buffer_local , 0 , sizeof(buffer_local));
	
	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_log_pwd(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	
	int  len_local = sizeof(m_tr069_params.syslog.log_pwd);
	
	if (NULL == buffer || len<=0)
	{
		return -1;
	}

	
	len_local=len_local>len?len:len_local-1;
	
	
	memcpy(m_tr069_params.syslog.log_pwd,buffer,len_local);

	
	return 0;
}

DLLEXPORT_API int sk_tr069_get_log_pwd(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[256] = {0};	
	memset(buffer_local , 0 , sizeof(buffer_local));
	*value = buffer_local;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}



/* //sunjian
丢包率、丢帧率、媒体流速率的统计周期，单位：s，格式0xAABBCCDD
AA：保留，暂停为0x00
BB：丢包率统计周期，默认值0x0A，即整数10
CC：丢帧率统计周期，默认值0x0A，即整数10
DD：媒体流速率统计周期，默认值0x05，即整数5
*/

DLLEXPORT_API int sk_tr069_set_log_StatInterval(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char starttime[128] = {0};
	char buffer_local[8] = {0};
	int  len_local=sizeof(buffer_local);
	int now = 0;
	if (NULL == buffer ||len<=0)
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

	evcpe_info(__func__ ,"startinterval=%s ",buffer_local);
	
	m_tr069_params.syslog.continue_time=atoi(buffer_local);
    now = sk_tr069_gettickcount();
	sprintf(starttime,"%d",now);
    //sk_tr069_set_syslog_starttime(NULL,starttime,128);

    memset(m_tr069_params.syslog.upload_starttime ,0 , sizeof(m_tr069_params.syslog.upload_starttime));
    memcpy(m_tr069_params.syslog.upload_starttime , starttime,len_local);

    return 0;
}


DLLEXPORT_API int sk_tr069_get_log_StatInterval(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[16]={0};

	memset(buffer_local , 0 ,sizeof(buffer_local));
	
	sk_func_porting_params_get("StatInterval",buffer_local,sizeof(buffer_local));
	
	*value=buffer_local;
	
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_log_recordinterval(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[16]={0};
	int  len_local = sizeof(buffer_local);
	if ( NULL == buffer || len <= 0)
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local , buffer , len_local);

	return sk_func_porting_params_set("record_interval",buffer_local);

}

//add by lijingchao 针对江苏移动项目需求
//对用户安装apk权限进行控制
DLLEXPORT_API int sk_tr069_get_user_intall_application(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[64]={0};
    
	memset(buffer , 0 , sizeof(buffer));
	
    sk_func_porting_params_get("user_intall_application", buffer , sizeof(buffer));
    
	*value = buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
    return 0;
}

DLLEXPORT_API int sk_tr069_set_user_intall_application(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
        evcpe_info(__func__ , "buffer is null or len is 0!\n");
		return -1;
	}
    evcpe_info(__func__ , "value:%s len:%d!\n",buffer,len);
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
    sk_func_porting_params_set("user_intall_application",buffer_local);
	return 0;
    
}

//end

DLLEXPORT_API int sk_tr069_set_syslog_starttime(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	int  len_local = 0;
	if (NULL == buffer || len<=0)
	{
		return -1;
	}
	len_local = sizeof(m_tr069_params.syslog.upload_starttime);
		
	len_local = len_local>len?len:len_local-1;

    memset(m_tr069_params.syslog.upload_starttime , 0 , sizeof(m_tr069_params.syslog.upload_starttime));
	memcpy(m_tr069_params.syslog.upload_starttime, buffer ,len_local);

	sk_func_porting_params_set("tr069_syslog_starttime",m_tr069_params.syslog.upload_starttime);

	return 0;

}

DLLEXPORT_API int sk_tr069_get_syslog_starttime(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

	sk_func_porting_params_get("tr069_syslog_starttime",m_tr069_params.syslog.upload_starttime,sizeof(m_tr069_params.syslog.upload_starttime));


	*len=strlen(m_tr069_params.syslog.upload_starttime);
	*value = m_tr069_params.syslog.upload_starttime;

	return 0;
}

DLLEXPORT_API int sk_tr069_set_syslog_server(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int  len_local=0;
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=sizeof(m_tr069_params.syslog.log_server);
	
	len_local=len_local>len?len:len_local-1;
	
	
	memcpy(m_tr069_params.syslog.log_server,buffer,len_local);


	sk_func_porting_params_set("tr069_syslog_server",m_tr069_params.syslog.log_server);


	return 0;

}

DLLEXPORT_API int sk_tr069_get_syslog_server(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	if(strlen(m_tr069_params.syslog.log_server)<=0)
	{
		strcpy(m_tr069_params.syslog.log_server," ");
	}
		
	*len=strlen(m_tr069_params.syslog.log_server);
	
	*value = m_tr069_params.syslog.log_server;
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_syslog_loglevel(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16]={0};
	int  length = sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	strncpy(buffer_local,buffer,length);
	
	m_tr069_params.syslog.log_level=atoi(buffer_local);
	
	sk_func_porting_params_set("tr069_syslog_loglevel",m_tr069_params.syslog.log_server);
    
	return 0;

}

DLLEXPORT_API int sk_tr069_get_syslog_loglevel(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

    static char buffer_local[16]={0};

	memset(buffer_local , 0 , sizeof(buffer_local));
	
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.syslog.log_level);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_set_syslog_logtype(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16]={0};
	int  length = sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length = length>len?len:length-1;

	strncpy(buffer_local,buffer,length);
	
	m_tr069_params.syslog.log_type = atoi(buffer_local);

	evcpe_info(__func__ , "sk_tr069_set_syslog_logtype logtype=%d",m_tr069_params.syslog.log_type);

    sk_func_porting_params_set("tr069_syslog_logtype",buffer_local);
    
	return 0;

}

DLLEXPORT_API int sk_tr069_get_syslog_logtype(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

    static char buffer_local[16]={0};

	memset(buffer_local , 0 , sizeof(buffer_local));
	
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.syslog.log_type);
	
	*value = buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_set_syslog_puttype(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16]={0};
	int  length=sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len ? len:length-1;

	strncpy(buffer_local,buffer,length);
	
	m_tr069_params.syslog.put_type=atoi(buffer_local);

    sk_func_porting_params_set("tr069_syslog_puttype",buffer_local);
 
	return 0;

}

DLLEXPORT_API int sk_tr069_get_syslog_puttype(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

    static char buffer_local[16]={0};

	memset(buffer_local , 0 , sizeof(buffer_local));
	
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.syslog.put_type);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_syslog_continue_time(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[32] = {0};
	int  length = sizeof(buffer_local);
	int time = 0;

	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	strncpy(buffer_local,buffer,length);
	
	m_tr069_params.syslog.continue_time=atoi(buffer_local);
    
	sk_func_porting_params_set("tr069_syslog_delay",buffer_local);

	return 0;

}

DLLEXPORT_API int sk_tr069_get_syslog_continue_time(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

    static char buffer_local[16] = {0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));
	
	sk_func_porting_params_get("tr069_syslog_delay", buffer_local,sizeof(buffer_local));
    
	*value=buffer_local;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}




DLLEXPORT_API int sk_tr069_get_cpuload(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
    strcpy(buffer_local , "25");
	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_get_cpuusage(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));

    
    sk_func_porting_params_get("cpu_usage" , buffer_local , sizeof(buffer_local));
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_get_memtotal(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));

    
    sk_func_porting_params_get("mem_total" , buffer_local , sizeof(buffer_local));
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_get_memfree(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));

    
    sk_func_porting_params_get("mem_free" , buffer_local , sizeof(buffer_local));
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_get_memload(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
    strcpy(buffer_local , "25");
	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_get_storageload(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
    strcpy(buffer_local , "25");
	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}



DLLEXPORT_API int sk_tr069_get_qos_bytes_received(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
    strcpy(buffer_local , "25");
	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_get_qos_packets_received(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
    strcpy(buffer_local , "25");
	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}



DLLEXPORT_API int sk_tr069_set_alarmswitch(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if(NULL == buffer || 0 == len)
	{
		return -1;
	}

	if(!strncmp(buffer,"false",len) || !strncmp(buffer , "FALSE" , len) ||!strncmp(buffer,"0",len))
	{		
		m_tr069_params.alarm.alarm_switch = 0;		
		sk_func_porting_params_set("AlarmSwitch", "0");	
	}	
	else if(!strncmp(buffer,"true",len) || !strncmp(buffer , "TRUE" , len ) || !strncmp(buffer,"0",len))
	{
		m_tr069_params.alarm.alarm_switch = 1;		
		sk_func_porting_params_set("AlarmSwitch", "1");
	}	
	else
	{	
		evcpe_info(__func__ , "sk_tr069_set_alarmswitch param is error!");
	}	
	return 0;

}


DLLEXPORT_API int sk_tr069_get_alarmswitch(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
    static char buffer_local[16]={0};
	memset(buffer_local , 0 , sizeof(buffer_local));
	if(m_tr069_params.alarm.alarm_switch)
	{
		strcpy(buffer_local,"true");
	}
	else
	{
		strcpy(buffer_local,"false");
	}
	
	*value = buffer_local;

	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}


DLLEXPORT_API int sk_tr069_set_alarmlevel(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16] = {0};
	int  length = sizeof(buffer_local);
	
	if ( NULL == buffer || len <= 0)
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	strncpy(buffer_local,buffer,length);

	m_tr069_params.alarm.alarm_level = atoi(buffer_local);
		
	return 0;

}

DLLEXPORT_API int sk_tr069_get_alarmlevel(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
    static char buffer_local[16]={0};

	memset(buffer_local , 0 , sizeof(buffer_local));
    //alarmlevel
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.alarm.alarm_level);

	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}



DLLEXPORT_API int sk_tr069_set_cpu_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length=sizeof(m_tr069_params.alarm.cpu_alarm);
	
	if ( NULL == buffer || len <= 0)
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	memset(m_tr069_params.alarm.cpu_alarm , 0 ,sizeof(m_tr069_params.alarm.cpu_alarm));
	
	strncpy(m_tr069_params.alarm.cpu_alarm,buffer,length);
		
	return 0;

}

DLLEXPORT_API int sk_tr069_get_cpu_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.cpu_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}



DLLEXPORT_API int sk_tr069_set_memory_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length = sizeof(m_tr069_params.alarm.memory_alarm);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	memset(m_tr069_params.alarm.memory_alarm,0,sizeof(m_tr069_params.alarm.memory_alarm));
	
	strncpy(m_tr069_params.alarm.memory_alarm,buffer,length);
		
	return 0;

}

DLLEXPORT_API int sk_tr069_get_memory_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.memory_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}


DLLEXPORT_API int sk_tr069_set_disk_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buffer_local[16]={0};
	int  length = sizeof(buffer_local);
	
	if ((buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	
	length = length>len?len:length-1;

	strncpy(buffer_local , buffer , length);

	memset(m_tr069_params.alarm.disk_alarm , 0 , sizeof(m_tr069_params.alarm.disk_alarm));
	
	strncpy(m_tr069_params.alarm.disk_alarm, buffer , length);
		
	return 0;

}

DLLEXPORT_API int sk_tr069_get_disk_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{

    *value = m_tr069_params.alarm.disk_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}



DLLEXPORT_API int sk_tr069_set_bandwidth_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length=sizeof(m_tr069_params.alarm.band_width_alarm);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	memset(m_tr069_params.alarm.band_width_alarm,0,sizeof(m_tr069_params.alarm.band_width_alarm));
	
	strncpy(m_tr069_params.alarm.band_width_alarm,buffer,length);
		
	return 0;

}

DLLEXPORT_API int sk_tr069_get_bandwidth_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.band_width_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}


DLLEXPORT_API int sk_tr069_set_packet_lost_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length = sizeof(m_tr069_params.alarm.packet_lost_alarm);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	length = length>len?len:length-1;

	memset(m_tr069_params.alarm.packet_lost_alarm,0,sizeof(m_tr069_params.alarm.packet_lost_alarm));
	
	strncpy(m_tr069_params.alarm.packet_lost_alarm,buffer,length);
		
	return 0;

}

DLLEXPORT_API int sk_tr069_get_packet_lost_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.packet_lost_alarm;

	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}

/***********************************************************************************
四川
************************************************************************************/

int sk_tr069_set_packet_lostframe_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length=sizeof(m_tr069_params.alarm.packet_lostframe_alarm);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length = length>len?len:length-1;

	memset(m_tr069_params.alarm.packet_lostframe_alarm,0,sizeof(m_tr069_params.alarm.packet_lostframe_alarm));
	
	strncpy(m_tr069_params.alarm.packet_lostframe_alarm,buffer,length);
		
	return 0;

}

int sk_tr069_get_packet_lostframe_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.packet_lostframe_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}

int sk_tr069_set_timedelay_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length = sizeof(m_tr069_params.alarm.packet_timedelay_alarm);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length = length>len?len:length-1;

	memset(m_tr069_params.alarm.packet_timedelay_alarm,0,sizeof(m_tr069_params.alarm.packet_timedelay_alarm));
	
	strncpy(m_tr069_params.alarm.packet_timedelay_alarm,buffer,length);
		
	return 0;

}

int sk_tr069_get_timedelay_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.packet_timedelay_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}


int sk_tr069_set_cushion_alarm(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int length=sizeof(m_tr069_params.alarm.packet_cushion_alarm);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	memset(m_tr069_params.alarm.packet_cushion_alarm, 0 , sizeof(m_tr069_params.alarm.packet_cushion_alarm));
	
	strncpy(m_tr069_params.alarm.packet_cushion_alarm , buffer,length);
		
	return 0;

}

int sk_tr069_get_cushion_alarm(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
	*value=m_tr069_params.alarm.packet_cushion_alarm;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}


/****************************************************************/




//可选:终端接收TVMS消息的接口
int sk_tr069_set_tvms_port(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;	
	}
	return 0;
}

//可选:TVMS资源服务地址
int sk_tr069_set_tvms_service(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	return 0;
}


//可选:TVMS资源更新检查周期
int sk_tr069_set_tvms_interval(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	return 0;
}

//可选:TVMS资源更新检查周期
int sk_tr069_set_tvms_verifycode(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	return 0;
}

//ping参数:表示ping数据的情况，如果终端管理平台要求设置这个值，则可以是： "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"
DLLEXPORT_API int sk_tr069_get_ping_state(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
        
    if('\0' == m_tr069_params.ping_result.nStatus[0])
    {
       	strcpy(buffer,"Error_Other"); 
    }
    else
    {
        strcpy(buffer , m_tr069_params.ping_result.nStatus);
    }
    
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
    
	return 0;
}


DLLEXPORT_API int sk_tr069_set_ping_state(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	return 0;
}


//ping参数:设置用于ping诊断的主机名或地址。
DLLEXPORT_API int sk_tr069_set_ping_host(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	memset(m_tr069_params.ping.szHost,0,sizeof(m_tr069_params.ping.szHost));
	
	strncpy(m_tr069_params.ping.szHost,buffer,len);
	
	return 0;
}


//ping参数:在报告结果之前，ping诊断重复的次数。
DLLEXPORT_API int sk_tr069_set_ping_repeat(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[16]={0};
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.ping.nNumberOfRepetitions = atoi(buf);

	return 0;
}



//ping参数:用毫秒表示的ping诊断超时时间
DLLEXPORT_API int sk_tr069_set_ping_timeout(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[32] = {0};
	if ( NULL == buffer || len<=0)
	{
		return -1;
	}
	
	len = sizeof(buf)>len?len:sizeof(buf);

	strncpy(buf,buffer,len);
	
	m_tr069_params.ping.nTimeout=atoi(buf);

	return 0;
}


//ping参数:每个ping命令发送的数据大小，以字节为单位，要求固定大小为32字节
DLLEXPORT_API int sk_tr069_set_ping_datasize(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buf[32] = {0};
	
	if ((buffer==NULL) || (len<=0))
	{
		return -1;
	}
	
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.ping.nDataBlockSize = atoi(buf);

	return 0;
}


int sk_tr069_get_ping_datasize(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));
	
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.ping.nDataBlockSize);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
	
}


//ping参数:测试包中用于DiffServ的码点，默认值为0
DLLEXPORT_API int sk_tr069_set_ping_dscp(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[16] = {0};
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len = sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.ping.nDSCP = atoi(buf);

	return 0;
}




//traceroute参数:设置用于ping诊断的主机名或地址。
DLLEXPORT_API int sk_tr069_set_traceroute_host(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ((buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	
	memset(m_tr069_params.traceroute.szHost,0,sizeof(m_tr069_params.traceroute.szHost));

	strncpy(m_tr069_params.traceroute.szHost,buffer,len);
	
	return 0;
}


//traceroute参数:用毫秒表示的ping诊断超时时间
DLLEXPORT_API int sk_tr069_set_traceroute_timeout(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[32] = {0};
	if ((buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.traceroute.nTimeout=atoi(buf);

	return 0;
}


//traceroute参数:每个traceroute命令发送的数据大小，以字节为单位，要求固定大小为32字节
DLLEXPORT_API int sk_tr069_set_traceroute_datasize(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[32] = {0};
	if ((buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.traceroute.nDataBlockSize = atoi(buf);
	

	return 0;
}

DLLEXPORT_API int sk_tr069_get_traceroute_datasize(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
    static char buffer_local[32]={0};

	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.traceroute.nDataBlockSize);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
	
}

DLLEXPORT_API int sk_tr069_get_traceroute_maxhopcount(struct evcpe_attr *attr,const char **value, unsigned int *len)	
{
    static char buffer_local[32]={0};

	snprintf(buffer_local,sizeof(buffer_local),"%d",m_tr069_params.traceroute.nMaxHopCount);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
	
}




//traceroute参数:测试包中用于DiffServ的码点，默认值为0
DLLEXPORT_API int sk_tr069_set_traceroute_dscp(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[32]={0};
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len = sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.traceroute.nDSCP=atoi(buf);
	
	return 0;
}




DLLEXPORT_API int sk_tr069_set_traceroute_maxhopcount(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char buf[32] = {0};
	
	if ((buffer == NULL) || (len<=0))
	{
		return -1;
	}
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.traceroute.nMaxHopCount = atoi(buf);
	
	return 0;
}


//ping
DLLEXPORT_API int sk_tr069_get_ping_successcount(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));

	sprintf(buffer,"%d",m_tr069_params.ping_result.nSuccessCount);
	
	*value=buffer;
	
	*len=strlen(*value);
	
	return 0;
}

DLLEXPORT_API int sk_tr069_get_ping_failurecount(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));

	sprintf(buffer,"%d",m_tr069_params.ping_result.nFailureCount);
	
	*value=buffer;
	
	*len=strlen(*value);


	return 0;
}


DLLEXPORT_API int sk_tr069_get_ping_average(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer , 0 , sizeof(buffer));

	sprintf(buffer,"%d",m_tr069_params.ping_result.nAverageResponseTime);
	
	*value=buffer;
	
	*len=strlen(*value);
	
	return 0;
}



DLLEXPORT_API int sk_tr069_get_ping_mintime(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));

	sprintf(buffer,"%d",m_tr069_params.ping_result.nMinimumResponseTime);
	
	*value=buffer;
	
	*len=strlen(*value);
	
	return 0;
}

DLLEXPORT_API int sk_tr069_get_ping_maxtime(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));
	
	sprintf(buffer,"%d",m_tr069_params.ping_result.nMaximumResponseTime);
	
	*value=buffer;
	
	*len=strlen(*value);
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_play_url(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	
	char buffer_local[1024] = {0};
	int  len_local = sizeof(buffer_local);
	if ((buffer==NULL) || (len<=0))
	{
		return -1;
	}
	
	len_local = len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);

	strcpy(m_tr069_params.play_url , buffer_local);

	return 0;
}

//获取acs地址
DLLEXPORT_API int sk_tr069_get_play_url(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	char *play_url = NULL;
	char* pout= NULL;
	static char  tmpbuf[1024]={0};
	char * p = NULL;
	int ulen = 0;
	pout = tmpbuf;
	//play_url=sk_mc_get_play_url(1);
	if(NULL != play_url && strlen(play_url)>0)
	{
		//tmpbuf
		if(strstr(play_url ,"&amp")==NULL)
		{
			do
			{
				 p=strstr(play_url,"&");
				 if(p!=NULL)
				 {
				 	  ulen=p-play_url;
						memcpy(pout,play_url,ulen);
						memcpy(pout+ulen,"&amp;",5);
				 }
				 else
				 {
				 		strcpy(pout,play_url);
						break;
				 }
				 play_url= p + 1; 
				 pout = pout +ulen + 5; 
			}while(p!=NULL);
		}
	   
		*value = tmpbuf;
		p  = strchr(tmpbuf, ';');
		if( NULL != p)
		{
			*len = p - play_url;
		}
		else
		{
			*len = strlen(*value);
		}
	}
	else if(strlen(m_tr069_params.play_url)>0)
	{
		*value = m_tr069_params.play_url;
		*len = strlen(*value);
	}

	return 0;
}


//playstate参数:表示诊断数据的情况，如果终端管理平台要求设置这个值，则可以是： "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"
DLLEXPORT_API int sk_tr069_get_play_state(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[32]={0};
	
	
	memset(buffer,0,sizeof(buffer));
	strcpy(buffer,"Complete");
	
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}	

//ping参数:表示诊断数据的情况，如果终端管理平台要求设置这个值，则可以是： "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"
DLLEXPORT_API int sk_tr069_get_traceroute_state(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
	
    if('\0' == m_tr069_params.traceroute_result.nStatus[0])
    {
       	strcpy(buffer,"Error_Other"); 
    }
    else
    {
        strcpy(buffer , m_tr069_params.traceroute_result.nStatus);
    }
    
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
    
	return 0;
}


DLLEXPORT_API int sk_tr069_set_traceroute_state(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	return 0;
}


DLLEXPORT_API int sk_tr069_get_traceroute_responsetime(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32] = {0};

	memset(buffer,0,sizeof(buffer));
	
	sprintf(buffer,"%d",m_tr069_params.traceroute_result.nResponseTime);
	
	*value=buffer;
	
	*len=strlen(*value);
	
	return 0;
}

DLLEXPORT_API int sk_tr069_get_traceroute_numberofroute(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));
	
	sprintf(buffer,"%d",m_tr069_params.traceroute_result.nNumberOfRouteHops);
	
	*value=buffer;
	
	*len=strlen(*value);
	
	return 0;
}


int sk_is_node_traceroute(char *node_name)
{
	int i = 0;
 	if(NULL == node_name)
	{
		return 0;
	}
	if(strstr(node_name , ".RouteHops.") != NULL)
	{
		
		sscanf(node_name,"Device.LAN.TraceRouteDiagnostics.RouteHops.%d.",&i);
		if( i <= m_tr069_params.traceroute_result.nNumberOfRouteHops)
		{
			return 1;
		}
		else
		{
			return 0;
		}	
	}
	return 1;
}


DLLEXPORT_API int sk_tr069_get_traceroute_host(int index,const char **value, unsigned int *len)
{
	static char buffer[TRACERT_MAX_HOP_COUNT_DEFAULT][32]={{0}};

	
	if(index < 0 || index >= m_tr069_params.traceroute_result.nNumberOfRouteHops)
	{
		goto finally;
	}
	memset(buffer[index],0,sizeof(buffer[index]));

    
	if(strlen(m_tr069_params.traceroute_result.gHopHost[index].szHost) > 0)
	{
		sprintf(buffer[index],"IP:%s",m_tr069_params.traceroute_result.gHopHost[index].szHost);
	}
	else
	{
		sprintf(buffer[index],"noreach");
	}
finally:
	*value=buffer[index];
	
	*len=strlen(*value);
	
	return 0;
}


DLLEXPORT_API int sk_tr069_get_traceroute_host1(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(0,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host2(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(1,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host3(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(2,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host4(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(3,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host5(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(4,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host6(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(5,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host7(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(6,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host8(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(7,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host9(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(8,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host10(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(9,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host11(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(10,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host12(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(11,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host13(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(12,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host14(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(13,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host15(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(14,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host16(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(15,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host17(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(16,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host18(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(17,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host19(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(18,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host20(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(19,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host21(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(20,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host22(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(21,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host23(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(22,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host24(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(23,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host25(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(24,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host26(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(25,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host27(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(26,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host28(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(27,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host29(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(28,value,len);
}

DLLEXPORT_API int sk_tr069_get_traceroute_host30(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	return sk_tr069_get_traceroute_host(29,value,len);
}



//设置机顶盒重新启动
int sk_tr069_reboot()
{
	evcpe_info(__func__ , "[sk_tr069_reboot]start!\n");

	sk_tr069_set_power_down_status(1);
	sk_tr069_add_boot_status(E_TR069_M_REBOOT);
	usleep(5000*1000);
	//reboot(RB_AUTOBOOT);
	//system("reboot");
	evcpe_info(__func__ , "[sk_tr069_reboot]end!\n");
	return sk_func_porting_params_set("tr069_reboot","");
}

//设置机顶盒恢复出厂设置
int sk_tr069_factoryreset()
{

	evcpe_info(__func__ , "sk_tr069_factoryreset  enter!\n");

	sk_tr069_set_boot_status(E_TR069_BOOTSTRAP);
    
	sk_factory_reset();
	
	evcpe_info(__func__ , "sk_tr069_factoryreset end!\n");
	
	return 0;
}


//启动ping流程
int sk_tr069_ping()
{
	evcpe_t *this=get_evcpe_object();
	
	evcpe_info(__func__ , "[sk_tr069_ping] enter ");
	
    memset(&m_tr069_params.ping_result , 0 , sizeof(m_tr069_params.ping_result));
	
	sk_start_ping(&m_tr069_params.ping,&m_tr069_params.ping_result);
	
	sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);
	
	evcpe_handle_del_event(this->cpe);
	this->cpe->event_flag=EVCPE_NO_BOOT;
    
    //evcpe_start_session(this->cpe);
	
	evcpe_worker_start_session(this->cpe);
    
	evcpe_info(__func__ , "[sk_tr069_ping] end !\n");
	return 0;
}


int sk_tr069_play()
{

	int     speed=0;
	int 	length=0;
	
	char	*status=NULL;
	evcpe_t *this=get_evcpe_object();

	evcpe_info(__func__ , "sk_tr069_play enter!");
	length=strlen(m_tr069_params.play_url);
	if(length>0)
	{
		/*
		sk_mc_play(m_tr069_params.play_url);
		sleep(5);
		status = owb_porting_media_status(&speed);
		if(!strcmp(status,"stop"))
		{
			m_tr069_params.play_status=0;
		}
		else
		{
			m_tr069_params.play_status=1;
		}
		m_tr069_params.is_play_test=1;
		*/
	}
	else
	{
		m_tr069_params.play_status=0;
	}
	
	sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);
	
	evcpe_handle_del_event(this->cpe);
	
	//evcpe_create_inform(this->cpe);
	this->cpe->event_flag = EVCPE_NO_BOOT;
	
	evcpe_start_session(this->cpe);
	
	return 0;
}

//返回traceroute结果给acs
void sk_tr069_traceroute_response_cb(const void * data, int len, int arg)
{
	evcpe_info(__func__, "sk_tr069_traceroute_response_cb data len = %d\n", len);
	if (!data)
	{
		evcpe_info(__func__, "data is NULL, then return\n");
		return;
	}
	bindermsg msg;
	bindermsg * pMsg = &msg;
	memset(pMsg, 0, sizeof(bindermsg));
	memcpy(pMsg, data, sizeof(bindermsg));
	memcpy(&m_tr069_params.traceroute_result, pMsg->msg, sizeof(sk_traceroute_results_t));
	
	int i = -1;
	for (i = 0; i < m_tr069_params.traceroute_result.nNumberOfRouteHops; i++)
	{
		evcpe_info(__func__, "sk_tr069_traceroute_response_cb host = %s\n", m_tr069_params.traceroute_result.gHopHost[i].szHost);
	}
	
	evcpe_t *this = get_evcpe_object();
	sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);
	evcpe_handle_del_event(this->cpe);
	this->cpe->event_flag=EVCPE_NO_BOOT;
	//evcpe_start_session(this->cpe);
	evcpe_worker_start_session(this->cpe);
    
    evcpe_info(__func__ , "[sk_tr069_traceroute]  end....!\n");
	
	return 0;
}

//启动traceroute流程
int sk_tr069_traceroute()
{
	evcpe_t *this = get_evcpe_object();
	evcpe_info(__func__ , "[sk_tr069_traceroute]  begin....!\n");
	sk_binder_set_cb(sk_tr069_traceroute_response_cb);
    memset(&m_tr069_params.traceroute_result , 0 ,sizeof(m_tr069_params.traceroute_result));
	bindermsg msg = {0};
	
	strcpy(msg.user,"TR069");
	msg.type = START_TRACEROUTE;
	memcpy(msg.msg, &(m_tr069_params.traceroute), sizeof(sk_traceroute_attribute_t));
	msg.len = sizeof(sk_traceroute_attribute_t);
	sk_binder_send(&msg,sizeof(msg));
	//sk_start_traceroute(&m_tr069_params.traceroute , &m_tr069_params.traceroute_result);
	evcpe_info(__func__ , "[sk_tr069_traceroute]  start....!\n");
	
	/*
	sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);
	evcpe_handle_del_event(this->cpe);
	this->cpe->event_flag=EVCPE_NO_BOOT;
	//evcpe_start_session(this->cpe);
	evcpe_worker_start_session(this->cpe);
    
    evcpe_info(__func__ , "[sk_tr069_traceroute]  end....!\n");
	*/
	return 0;
} 


//在这里返回用户选择是否启动下载进程，如果不启动或者应用程序退回，则直接返回1
int sk_tr069_enable_download()
{
	return 1;
}

//设置下载参数到全局变量中
int sk_tr069_set_download_param(struct evcpe_download *download_param)
{
	if(NULL == download_param)
	{
		return -1;
	}
	
	memcpy(&m_tr069_params.download_param,download_param,sizeof(struct evcpe_download));
	
	return 0;	
}

//设置上传参数到全局变量中
int sk_tr069_set_upload_param(struct evcpe_upload *upload_param)
{
	if(NULL == upload_param)
	{
		return -1;
	}
	
	memcpy(&m_tr069_params.upload_param,upload_param,sizeof(struct evcpe_upload));
	return 0;	
}

int sk_tr069_set_upgrade_status(int value)
{
	m_tr069_params.is_downloading = value;
	return 0;
}




int sk_tr069_get_upgrade_status()
{
	return m_tr069_params.is_downloading;
	
}

int sk_tr069_set_first_connect_status(int value)
{
	if(m_tr069_params.first_usenet!=value)
		m_tr069_params.first_usenet=value;
	return 0;
}


//返回当前连接状态，如果是第一次连接，则返回1，否则返回0
int sk_tr069_get_first_connect_status()
{
	return m_tr069_params.first_usenet;
}


//启动下载
int sk_tr069_start_download()
{

	struct evcpe *cpe = get_cpe_object();
    
   	int ret = 0;

	evcpe_debug(__func__,"[sk_tr069_start_download] enter!\n");

	if(sk_tr069_get_upgrade_status()== EVCPE_DOWNLOADING)
	{
		return 0;
	}
    
    evcpe_debug(__func__,"m_tr069_params.download_param.filetype= %s",m_tr069_params.download_param.filetype);                                                       
	if(!strcasecmp(m_tr069_params.download_param.filetype,"1 Firmware Upgrade Image")) //升级包
	{		
        char download_url[512]={0};
		
		sk_func_porting_params_get("tr069_upgrade_server",download_url,sizeof(download_url));
		evcpe_debug(__func__,"get from param tr069_upgrade_server:%s",download_url);
		if(download_url!=NULL)
		{
			if(strcasecmp(download_url,m_tr069_params.download_param.url))
			{
				sk_func_porting_params_set("tr069_upgrade_server",m_tr069_params.download_param.url);
				evcpe_debug(__func__,"use platform seted url:%s",m_tr069_params.download_param.url);
			}
		}
		else
		{
   		    sk_func_porting_params_set("tr069_upgrade_server",m_tr069_params.download_param.url);
		}
		
		sk_func_porting_params_get("tr069_upgrade_server",download_url,sizeof(download_url));
		ret = sk_tr069_porting_upgrade(download_url);
		/*
		if(ret == 0)
		{
			sk_tr069_set_upgrade_status(EVCPE_NO_DOWNLOAD);	//不需要升级
			LOGI("need not upgrade");
		}
		*/
		evcpe_debug(__func__,"add boot stauts");
		//sk_tr069_set_upgrade_status(EVCPE_DOWNLOADING);	//当前正在下载
		//sk_tr069_set_power_down_status(1);//关闭心跳
		//sk_tr069_add_boot_status(E_TR069_TRANSFER_COMPLETE);


        //add by lijingchao
        //date:2014-04-14
        //备份当前系统编译时间，用于升级判断，作为升级成功的判断依据。
        //如果开机的时候，编译时间相同，证明上次没有升级成功，不应该发TRANSFER_COMPLETE 事件
        {
            char buffer[128]={0};
            sk_func_porting_params_get("tr069_system_build_time" , buffer , sizeof(buffer));
            sk_func_porting_params_set("tr069_bak_system_build_time", buffer);
            sk_api_params_set("tr069_upgrade_flag","1");
            
        }
        //add end
        
		//sunjian:江苏移动不要求 上报 版本变化:
		//sk_tr069_add_boot_status(E_TR069_VALUE_CHANGED);
		
	}
	else if(!strcasecmp(m_tr069_params.download_param.filetype,"3 Vendor Configuration File"))
	{
	    sk_start_download_configure_file(cpe , &m_tr069_params.download_param);
	}
	else
	{
		evcpe_debug(__func__,"[sk_tr069_start_download]other download type!");
		
	}
	return 0;	
}


static void sk_start_upload_file_cb(struct evhttp_request *http_req, void *arg) 
{
	int rc = 0;
	const char *cookies = NULL;    
	struct evhttp_connection *http_conn = (struct evhttp_connection *)arg;

    evcpe_info(__func__ , " enter\n");
	if (http_req == NULL || 0 == http_req->response_code)   
	{
		evcpe_info(__func__ ,"evcpe_session_http_cb  timed out\n");
		rc = ETIMEDOUT;
		goto close;
	}

    evcpe_info(__func__ , "HTTP response code: %d\n", http_req->response_code);
    evcpe_info(__func__ ," HTTP response content: \n%.*s\n",
			EVBUFFER_LENGTH(http_req->input_buffer),
			EVBUFFER_DATA(http_req->input_buffer));

close:
    evcpe_info(__func__ , "evcpe_session_close\n");
	evhttp_connection_set_closecb(http_conn , NULL, NULL);
    evhttp_connection_free(http_conn);
    http_conn = NULL;
}

static int sk_build_configure_file(struct evbuffer *buffer)
{
    char *value = NULL ;
    unsigned int len =  0;
    sk_tr069_get_acsurl(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "acsURL::%s\n", value);

    sk_tr069_get_acsbakurl(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "acsURLBackup::%s\n" , value);

    sk_tr069_get_authurl(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "AuthURL::%s\n",value);
    evcpe_add_buffer(buffer, "UserID::\n");
    evcpe_add_buffer(buffer, "Password::\n");

    sk_tr069_get_platform_url(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "PlatformURL::%s\n",value);

    sk_tr069_get_platform_bak_url(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "PlatformURLBackup::%s\n",value);

    sk_tr069_get_hdc_url(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "HDCURL::%s\n",value);

    sk_tr069_get_silent_upgrade(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "SilentUpgrade::%s\n",value);

    sk_tr069_get_user_intall_application(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "UserInstallApplication::%s\n",value);

    sk_tr069_get_ntp_server(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "NTPServer::%s\n",value);

    return 0;
}

static int sk_start_upload_configure_file_by_http(struct evcpe *cpe , struct evcpe_upload *upload_param)
{
	struct evhttp_connection *conn = NULL;
    struct evhttp_request *req = NULL;
    struct evcpe_url * upload_url = NULL;
    unsigned char tmp_buffer[128] = {0};
    int rc = 0;
    upload_url = evcpe_url_new();
    evcpe_url_from_str(upload_url , upload_param->url);
    if (!(conn = evhttp_connection_new(upload_url->host , upload_url->port))) 	//连接远程主机指定端口
	{
		evcpe_info(__func__ , "failed to create evhttp_connection"); 
	    goto LAB_ERR;
	}

	evhttp_connection_set_base(conn, cpe->evbase);
	evhttp_connection_set_timeout(conn, cpe->acs_timeout);
        
     if (!(req = evhttp_request_new(sk_start_upload_file_cb, conn))) 
	{
		evcpe_info(__func__ , "failed to create evhttp_request_new");
		goto LAB_ERR;
	}
    

    req->major = 1;
    req->minor = 1;

    sprintf(tmp_buffer, "%s:%d", upload_url->host,  upload_url->port);
	if ((rc = evhttp_add_header(req->output_headers,"Host", tmp_buffer))) 
	{
		evcpe_info(__func__ , "failed to add header: Host= %s\n",tmp_buffer);
		evhttp_request_free(req);
	    goto LAB_ERR;
	}


    if (rc = evhttp_add_header(req->output_headers,	"User-Agent", "Skyworth")) 
	{
		evcpe_info(__func__ , "failed to add header User-Agent!\n");
		evhttp_request_free(req);
		goto LAB_ERR;
	}

    sk_build_configure_file(req->output_buffer);
    
    if (rc = evhttp_make_request(conn, req ,EVHTTP_REQ_POST , upload_url->uri)) 
	{
		evcpe_info(__func__ , "failed to evhttp_make_request!\n");
		evhttp_request_free(req);
		goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    if(upload_url)
    {
        evcpe_url_free(upload_url);
        upload_url = NULL;
    }
    if(conn)
    {
        evhttp_connection_free(conn);
        conn = NULL;
    }
    return -1;
}

static int sk_start_upload_configure_file_by_ftp(struct evcpe *cpe , struct evcpe_upload *upload_param)
{
    struct evcpe_url * upload_url = NULL;
    unsigned char tmp_buffer[128] = {0};
    int rc = 0;
    FILE *file = NULL;
	int fd = 0;
    unsigned char *filename = "upload_configure.txt";
    
    struct evbuffer *output_buffer = NULL;
    
    if (!(output_buffer = evbuffer_new()))
	{
        rc = -1;
		evcpe_error(__func__, "failed to create evbuffer");
		goto finally;
	}
    
  
    if (!(file = fopen(filename, "wb")))
	{
		rc = errno;
		goto finally;
	}
	fd = fileno(file);
    
    sk_build_configure_file(output_buffer);
    upload_url = evcpe_url_new();
    evcpe_url_from_str(upload_url , upload_param->url);
    evbuffer_write(output_buffer ,fd);

    
    //rc = FtpPut(upload_url->host , upload_url->port , upload_url->username , upload_url->password , upload_url->uri , filename);
    
finally:

    if(file)
    {
        fclose(file);
    }
    
    if(output_buffer)
    {
        evbuffer_free(output_buffer);
        output_buffer = NULL;
    }

    if(upload_url)
    {
        evcpe_url_free(upload_url);
        upload_url = NULL;
    }
        
    return rc;
}

static int sk_start_upload_configure_file(struct evcpe *cpe , struct evcpe_upload *upload_param)
{
    struct evcpe_url * upload_url = NULL;
    int rc = 0;
    upload_url = evcpe_url_new();
    evcpe_url_from_str(upload_url , upload_param->url);
    if(!strcmp("http",upload_url->protocol))
    {
        rc = sk_start_upload_configure_file_by_http(cpe ,upload_param);
    }
    else if(!strcmp("ftp" , upload_url->protocol))
    {
        rc = sk_start_upload_configure_file_by_ftp(cpe ,upload_param);
    }
    else
    {
        evcpe_error(__func__ , "upload configure  protocol is other");
        rc = -1;
    }

    if(upload_url)
    {
        evcpe_url_free(upload_url);
        upload_url = NULL;
    }
    return rc ;
     
}



static void sk_start_download_file_cb(struct evhttp_request *http_req, void *arg) 
{
	int rc = 0;
	const char *cookies = NULL;    
	struct evhttp_connection *http_conn = (struct evhttp_connection *)arg;

    evcpe_info(__func__ , " enter\n");
	if (http_req == NULL || 0 == http_req->response_code)   
	{
		evcpe_info(__func__ ,"evcpe_session_http_cb  timed out\n");
		rc = ETIMEDOUT;
		goto close;
	}

    evcpe_info(__func__ , "HTTP response code: %d\n", http_req->response_code);
    evcpe_info(__func__ ," HTTP response content: \n%.*s\n",
			EVBUFFER_LENGTH(http_req->input_buffer),
			EVBUFFER_DATA(http_req->input_buffer));

close:
    evcpe_info(__func__ , "evcpe_session_close\n");
	evhttp_connection_set_closecb(http_conn , NULL, NULL);
    evhttp_connection_free(http_conn);
    http_conn = NULL;
}


static int sk_start_download_configure_file_by_http(struct evcpe *cpe , struct evcpe_download *download_param)
{
	struct evhttp_connection *conn = NULL;
    struct evhttp_request *req = NULL;
    struct evcpe_url * download_url = NULL;
    unsigned char tmp_buffer[128] = {0};
    int rc = 0;
    download_url = evcpe_url_new();
    evcpe_url_from_str(download_url , download_param->url);
    if (!(conn = evhttp_connection_new(download_url->host , download_url->port))) 	//连接远程主机指定端口
	{
		evcpe_info(__func__ , "failed to create evhttp_connection"); 
	    goto LAB_ERR;
	}

	evhttp_connection_set_base(conn, cpe->evbase);
	evhttp_connection_set_timeout(conn, cpe->acs_timeout);
        
     if (!(req = evhttp_request_new(sk_start_download_file_cb, conn))) 
	{
		evcpe_info(__func__ , "failed to create evhttp_request_new");
		goto LAB_ERR;
	}
    

    req->major = 1;
    req->minor = 1;

    sprintf(tmp_buffer, "%s:%d", download_url->host,  download_url->port);
	if ((rc = evhttp_add_header(req->output_headers,"Host", tmp_buffer))) 
	{
		evcpe_info(__func__ , "failed to add header: Host= %s\n",tmp_buffer);
		evhttp_request_free(req);
	    goto LAB_ERR;
	}


    if (rc = evhttp_add_header(req->output_headers,	"User-Agent", "Skyworth")) 
	{
		evcpe_info(__func__ , "failed to add header User-Agent!\n");
		evhttp_request_free(req);
		goto LAB_ERR;
	}

    sk_build_configure_file(req->output_buffer);
    
    if (rc = evhttp_make_request(conn, req ,EVHTTP_REQ_GET , download_url->uri)) 
	{
		evcpe_info(__func__ , "failed to evhttp_make_request!\n");
		evhttp_request_free(req);
		goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    if(download_url)
    {
        evcpe_url_free(download_url);
        download_url = NULL;
    }
    if(conn)
    {
        evhttp_connection_free(conn);
        conn = NULL;
    }
    return -1;
}


static int sk_start_download_configure_file_by_ftp(struct evcpe *cpe , struct evcpe_download *download_param)
{
    return 0;
}

static int sk_start_download_configure_file(struct evcpe *cpe , struct evcpe_download *download_param)
{
    struct evcpe_url * download_url = NULL;
    int rc = 0;
    download_url = evcpe_url_new();
    evcpe_url_from_str(download_url , download_param->url);
    if(!strcmp("http",download_url->protocol))
    {
        rc = sk_start_download_configure_file_by_http(cpe ,download_param);
    }
    else if(!strcmp("ftp" , download_url->protocol))
    {
        rc = sk_start_download_configure_file_by_ftp(cpe ,download_param);
    }
    else
    {
        evcpe_error(__func__ , "upload configure  protocol is other");
        rc = -1;
    }

    if(download_url)
    {
        evcpe_url_free(download_url);
        download_url = NULL;
    }
    return rc ;
     
}


static int sk_build_log_file(struct evbuffer *buffer)
{
    int rc = 0;
	int  fd = 0;
    FILE *file = NULL;
    unsigned char *filename = "/data/data/com.skyworthdigital.tr069/files/log.txt";
    char *value = NULL ;
    int len =  0;
    int read_size = 0;
    int failed_cout = 0;
   
    evcpe_info(__func__ , "enter !\n");

	evbuffer_drain(buffer, EVBUFFER_LENGTH(buffer));

   
    evcpe_add_buffer(buffer, "Manufacturer:SKYWORTH;\n");

    sk_tr069_get_product_type(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "ProductClass:%s;\n",value);

    sk_tr069_get_stbid(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "SerialNumber:%s;\n",value);

    sk_tr069_get_hd_version(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "HWVer:%s;\n",value);
    
    sk_tr069_get_sw_version(NULL ,&value , &len);
    evcpe_add_buffer(buffer, "SWVer:%s;\n",value);
    evcpe_add_buffer(buffer, "\r\n");
    
	if (NULL != (file = fopen(filename, "r")))
	{
      	fd = fileno(file);
    	do
    	{
    		len = evbuffer_read(buffer, fd, -1);
        }
    	while (len > 0);

    	rc = len < 0 ? -1 : 0;
    	fclose(file);
	}
    else
    {
     	evcpe_info(__func__ , "fopen filename:%s failed ,strerrno = %s\n",
            filename ,strerror(errno));

    }
    
finally:
    evcpe_info(__func__ ,"buffer = %.*s\n",EVBUFFER_LENGTH(buffer),EVBUFFER_DATA(buffer));
    evcpe_info(__func__ , "exit rc = %d!\n",rc);

    return rc;
}

int sk_start_upload_log_file(struct evcpe *cpe , struct evcpe_upload *upload_param)
{
	struct evhttp_connection *conn = NULL;
    struct evhttp_request *req = NULL;
    struct evcpe_url * upload_url = NULL;
    unsigned char tmp_buffer[128] = {0};
    int rc = 0;
    upload_url = evcpe_url_new();
    evcpe_url_from_str(upload_url , upload_param->url);
    if (!(conn = evhttp_connection_new(upload_url->host , upload_url->port))) 	//连接远程主机指定端口
	{
		evcpe_info(__func__ , "failed to create evhttp_connection"); 
	    goto LAB_ERR;
	}

	evhttp_connection_set_base(conn, cpe->evbase);
	evhttp_connection_set_timeout(conn, cpe->acs_timeout);
        
     if (!(req = evhttp_request_new(sk_start_upload_file_cb, conn))) 
	{
		evcpe_info(__func__ , "failed to create evhttp_request_new");
		goto LAB_ERR;
	}
    

    req->major = 1;
    req->minor = 1;

    sprintf(tmp_buffer, "%s:%d", upload_url->host,  upload_url->port);
	if ((rc = evhttp_add_header(req->output_headers,"Host", tmp_buffer))) 
	{
		evcpe_info(__func__ , "failed to add header: Host= %s\n",tmp_buffer);
		evhttp_request_free(req);
	    goto LAB_ERR;
	}


    if (rc = evhttp_add_header(req->output_headers,	"User-Agent", "Skyworth")) 
	{
		evcpe_info(__func__ , "failed to add header User-Agent!\n");
		evhttp_request_free(req);
		goto LAB_ERR;
	}

    sk_build_log_file(req->output_buffer);
    
    if (rc = evhttp_make_request(conn, req ,EVHTTP_REQ_POST , upload_url->uri)) 
	{
		evcpe_info(__func__ , "failed to evhttp_make_request!\n");
		evhttp_request_free(req);
		goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    if(upload_url)
    {
        evcpe_url_free(upload_url);
        upload_url = NULL;
    }
    if(conn)
    {
        evhttp_connection_free(conn);
        conn = NULL;
    }
    return -1;
}

//上载日志文件到服务器
int sk_tr069_start_upload(struct evcpe *cpe)
{
	char *upload_url = NULL;
	unsigned char *ptr =  NULL;
	int len = 0;
	evcpe_info(__func__ , "sk_tr069_start_upload enter\n");
    evcpe_info(__func__ , "commandkey = %s , filetype = %s , url = %s , username = %s ,password = %s,delayseconds = %d",
        m_tr069_params.upload_param.commandkey ,
        m_tr069_params.upload_param.filetype ,
        m_tr069_params.upload_param.url ,
        m_tr069_params.upload_param.username ,
        m_tr069_params.upload_param.password ,
        m_tr069_params.upload_param.delayseconds);

    evcpe_info(__func__ , "before url = %s\n" , m_tr069_params.upload_param.url);
	ptr = m_tr069_params.upload_param.url;
    while(ptr = strstr(ptr , "&amp;"))
    {
        len = strlen(ptr);
        memcpy(ptr+1 , ptr+5 , len-4);
        ptr+=5; 
    }
    evcpe_info(__func__ , "after url = %s\n" , m_tr069_params.upload_param.url);
        
								    
	if(!strcmp(m_tr069_params.upload_param.filetype,"2 Vendor Log File"))
	{

        sk_func_porting_params_set("tr069_vendor_log", "logcat -v time > /data/data/com.skyworthdigital.tr069/files/log.txt");
        
        sk_start_upload_log_file(cpe , &m_tr069_params.upload_param);
        sk_tr069_add_boot_status(E_TR069_TRANSFER_COMPLETE);

        evcpe_handle_del_event(cpe);
	    cpe->event_flag = EVCPE_NO_BOOT;
	    evcpe_worker_start_session(cpe);
      
	}
	else if(!strcmp(m_tr069_params.upload_param.filetype,"1 Vendor Configuration File"))
	{	
		//sk_upload_cfg(m_tr069_params.upload_param.url,m_tr069_params.upload_param.username,m_tr069_params.upload_param.password);
        //sk_qos_send_by_http(m_tr069_params.upload_param.url,m_tr069_params.upload_param.username,m_tr069_params.upload_param.password,INFO_CONFIG_TYPE);
       
        sk_start_upload_configure_file(cpe , &m_tr069_params.upload_param);
        sk_tr069_add_boot_status(E_TR069_TRANSFER_COMPLETE);

        evcpe_handle_del_event(cpe);
	    cpe->event_flag = EVCPE_NO_BOOT;
	    evcpe_worker_start_session(cpe);
    
	}
	else
	{
		evcpe_error(__func__ , "[sk_tr069_start_upload]other upload type!");
	}
	return 0;
	
}


DLLEXPORT_API int sk_tr069_set_log_enable(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[8]={0};
	
	int  enable = 0 ;
	
	int  len_local=sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	len_local = len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	
	m_tr069_params.syslog.log_type=atoi(buffer_local);
	evcpe_info(__func__ , "sk_tr069_set_log_enable syslog.log_type:%d",m_tr069_params.syslog.log_type);
	
	return 0;
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_log_enable(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[8]={0};

	sprintf(buffer,"%d",m_tr069_params.syslog.log_type);
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}


DLLEXPORT_API int sk_tr069_set_logtype(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[8]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);

	evcpe_info(__func__ , "log_type=%s\n\n",buffer_local);
	
	m_tr069_params.shlog.log_type=atoi(buffer_local);

	return 0;
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_logtype(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[8]={0};
	sprintf(buffer,"%d",m_tr069_params.shlog.log_type);

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}


DLLEXPORT_API int sk_tr069_set_log_username(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	int  len_local=sizeof(m_tr069_params.syslog.log_user);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=len_local>len?len:len_local-1;

	memcpy(m_tr069_params.syslog.log_user,buffer,len_local);
	return 0;
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_log_username(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	if(strlen(m_tr069_params.syslog.log_user) <=0 )
	{
		strcpy(m_tr069_params.syslog.log_user," ");
	}
	
	*value=m_tr069_params.syslog.log_user;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}


int sk_tr069_set_log_userpass(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	
	int  len_local=sizeof(m_tr069_params.syslog.log_pwd);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;

	memcpy(m_tr069_params.syslog.log_pwd,buffer,len_local);
	//printf("[sk_tr069_set_log_userpass] value=%s\n",m_tr069_params.syslog.log_pwd);
	return 0;
}


//获取认证密码
int sk_tr069_get_log_userpass(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	//printf("[sk_tr069_get_log_username] value=%s\n",m_tr069_params.syslog.log_pwd);
	if(strlen(m_tr069_params.syslog.log_pwd)<=0)
	{
		strcpy(m_tr069_params.syslog.log_pwd," ");
	}	
	*value=m_tr069_params.syslog.log_pwd;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

   


//获取认证密码
int sk_tr069_get_log_rtspinfo(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	*value=m_tr069_params.shlog.log_info[ENUM_LOG_RTSP_INFO];
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

//获取认证密码
int sk_tr069_get_log_igmpinfo(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	*value=m_tr069_params.shlog.log_info[ENUM_LOG_IGMP_INFO];
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_log_httpinfo(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	*value=m_tr069_params.shlog.log_info[ENUM_LOG_HTTP_INFO];
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}	

//获取认证密码
int sk_tr069_get_log_pkgtotal(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[16] = {0};
	
	memset(buffer,0,sizeof(buffer));
	
	sprintf(buffer,"%d",m_tr069_params.shlog.pkgtotal_onesec);
	
	*value=buffer;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

//获取认证密码
int sk_tr069_get_log_bytetotal(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	memset(buffer,0,sizeof(buffer));
	
	sprintf(buffer,"%d",m_tr069_params.shlog.bytetotal_onesec);
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

//获取认证密码
int sk_tr069_get_log_pkglostrate(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%d",m_tr069_params.shlog.pkglostrate);
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

//获取认证密码
int sk_tr069_get_log_avaragerate(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%d",m_tr069_params.shlog.avaragerate);
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}


//获取认证密码
int sk_tr069_get_log_buffer(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

	static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%d",m_tr069_params.shlog.bufferrate);

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

//获取认证密码
int sk_tr069_get_log_error(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	*value=m_tr069_params.shlog.log_info[ENUM_LOG_ERR_INFO];
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

int sk_tr069_set_dhcppass(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

	return sk_func_porting_params_set("dhcp_pwd",buffer_local);
}


//获取认证密码
int sk_tr069_get_dhcppass(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
    
	memset(buffer , 0 , sizeof(buffer));
	
	sk_func_porting_params_get("dhcp_pwd",buffer,sizeof(buffer));

    if(strlen(buffer) <= 0)
	{
		strcpy(buffer,"null");
	}

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_set_authurlbak(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);


	return sk_func_porting_params_set("home_page2",buffer_local);

}


//获取认证密码
int sk_tr069_get_authurlbak(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};
	
	memset(buffer , 0 , sizeof(buffer));
	sk_func_porting_params_get("home_page2",buffer,sizeof(buffer));

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_set_stb_macoverride(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[32]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);


	sk_func_porting_params_set("mac_override",buffer_local);



	return 0;
}


int sk_tr069_get_stb_macoverride(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
    
	memset(buffer,0,sizeof(buffer));
	
	sk_func_porting_params_get("mac_override",buffer,sizeof(buffer));
	
	*value=buffer;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}



int sk_tr069_set_stb_mac(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64] = {0};
	int  len_local = sizeof(buffer_local);
	if (NULL == buffer || len<= 0 )
	{
		return -1;
	}
	
	sk_func_porting_params_get("mac_override", buffer_local , sizeof(buffer_local));

	if('\0' == buffer_local[0]  ||  !strcmp(buffer,"0"))  //如果不允许修改mac地址
	{
		return 0;
	}

	len_local = len_local>len?len:len_local-1;
    
	memcpy(buffer_local , buffer ,len_local);
    
	sk_func_porting_params_set("mac",buffer_local);

	return 0;
}


int sk_tr069_set_stb_dns(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	
	int  len_local=sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);

	sk_func_porting_params_set("dns1",buffer_local);

	return 0;
}



int sk_tr069_set_stb_gateway(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);


	sk_func_porting_params_set("gateway",buffer_local);


	return 0;
}	


int sk_tr069_set_stb_mask(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);
	
	sk_func_porting_params_set("lan_mask",buffer_local);
	

	return 0;
}	

int sk_tr069_set_stb_ip(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};

	int  len_local=sizeof(buffer_local);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);

	sk_func_porting_params_set("lan_ip",buffer_local);

	return 0;
}	


int sk_tr069_set_current_time(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	
	memcpy(buffer_local,buffer,len_local);

	return 0;
}	

int sk_tr069_set_time_zone(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[8]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

	sk_func_porting_params_set("timezone",buffer_local);
	return 0;
}	

DLLEXPORT_API int sk_tr069_set_ntp_server2(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char _buffer[64] = {0};
	int  length=sizeof(_buffer);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	
	length=length>len?len:length-1;

	strncpy(_buffer,buffer,length);
	
	sk_func_porting_params_set("ntp_server2",_buffer);
	return 0;
}	

//第一个 NTP 时间服务器。可以为域名或 IP 地址，由业务管理平台通过Authentication.CTCSetConfig（“NPTDomain”，“NTPServer1”）方法进行配置。当发生改变时，需立即上报终端管理平台
DLLEXPORT_API int sk_tr069_get_ntp_server2(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
    
	sk_func_porting_params_get("ntp_server2",buffer,sizeof(buffer));

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


  
int sk_tr069_get_totalbyte_sent(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_totalbyte_recv(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	

	*value = buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}




int sk_tr069_get_totalpacket_sent(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	*value=buffer;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}



int sk_tr069_get_totalpacket_recv(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_currday_interval(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

      	
 
int sk_tr069_get_currdaybyte_sent(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}


int sk_tr069_get_currdaybyte_recv(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_currdaypacket_sent(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

      	
int sk_tr069_get_currdaypacket_recv(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_qhour_interval(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}
	

int sk_tr069_get_qhourbyte_sent(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}
	
int sk_tr069_get_qhourbyte_recv(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_qhourpacket_sent(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

int sk_tr069_get_qhourpacket_recv(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer[32]={0};
	
	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}

	return 0;
}

//保存所有数据到缓冲区中，之后再存为文件


//通过ftp方式发送指定文件
static int _logmsg_send_by_ftp(char *filename)
{
	//获得本地存储的ftp用户名和密码,以及url和端口,将指定文件通过ftp方式发送到远端

   	char upload_cmd[512] = {0};
    int  ret=0;
	if (filename == NULL)
	{
		
		return -1;
	}
	
	memset(upload_cmd, 0x00, sizeof(upload_cmd));
	snprintf (upload_cmd, sizeof(upload_cmd), "ftpput -u %s -p %s -v %s %s %s",m_tr069_params.syslog.log_user,m_tr069_params.syslog.log_pwd, m_tr069_params.syslog.log_server, filename+ sizeof(SK_LOG_PATH),  filename );
	printf("[_logmsg_send_by_ftp]filename=%s\n\n",filename);
	ret=system(upload_cmd);
    if(ret<0)
        return ret;
    
	return 0;
}

static char* _tr069_format_ip(char *ip)
{
	int i=0;
	
	if(ip==NULL)
		return NULL;
	while(ip[i]!='\0')
	{
		if(ip[i]==':')
			ip[i]='-';
		i++;
	}
	return ip;
}
static int _tr069_get_year2(int nYear4)
{
	int nYear = 0;
	int nLastNum = 0;
	int nRemain = 0;
	int nRet = 0;

	nYear = nYear4/1000;

	if(nYear <= 0)
	{
		return 0;
	}

	nRet = nLastNum = nYear4%10;
	nRemain = nYear4/10;

	nLastNum = nRemain%10;

	nRet += nLastNum*10;

	return nRet;
}



static char* _tr069_format_mac(char *mac)
{
	int i=0;
	
	if(mac==NULL)
		return NULL;
	while(mac[i]!='\0')
	{
		if(mac[i]==':')
			mac[i]='-';
		i++;
	}
	
	return mac;
}



int sk_tr069_set_log_pkgtotal(unsigned int value)
{
	if(m_tr069_params.shlog.log_enable<1)
		return 0;
		
	m_tr069_params.shlog.pkgtotal_onesec=value;

	_tr069_set_log_info(ENUM_LOG_PKG_TOTAL_ONE_SEC,NULL,value);


	return 0;
}
int sk_tr069_set_log_bytetotal(unsigned int value)
{
	if(m_tr069_params.shlog.log_enable<1)
		return 0;
	m_tr069_params.shlog.bytetotal_onesec=value;
	_tr069_set_log_info(ENUM_LOG_BYTE_TOTAL_ONE_SEC,NULL,value);


	return 0;
}

int sk_tr069_set_log_pkglostrate(unsigned int value)
{
	if(m_tr069_params.shlog.log_enable<1)
		return 0;
	m_tr069_params.shlog.pkglostrate=value;
	_tr069_set_log_info(ENUM_LOG_PKG_LOST_RATE,NULL,value);


	return 0;
}

int sk_tr069_set_log_avaragerate(unsigned int value)
{
	if(m_tr069_params.shlog.log_enable<1)
		return 0;
	m_tr069_params.shlog.avaragerate=value;
	_tr069_set_log_info(ENUM_LOG_AVARAGE_RATE,NULL,value);


	return 0;
}


int sk_tr069_set_log_buffer(int value)
{
	if(m_tr069_params.shlog.log_enable<1)
		return 0;
	m_tr069_params.shlog.bufferrate=value;
	_tr069_set_log_info(ENUM_LOG_BUFFER,NULL,value);

	return 0;
}
int sk_tr069_set_log_info(int log_type,char *log_info)
{
	return _tr069_set_log_info(log_type,log_info,0);
}

static char *_tr069_get_currtime(char *currtime)
{
	time_t      curr_time;
    struct tm   *curr_tm;
	
	curr_time = time(NULL);
    curr_tm = gmtime(&curr_time);
	sprintf(currtime, "%04d%02d%02d%02d%02d%02d", 
	(curr_tm->tm_year + 1900), curr_tm->tm_mon + 1, curr_tm->tm_mday, 
		curr_tm->tm_hour+8, curr_tm->tm_min,curr_tm->tm_sec);

	return currtime;
}
static char *_tr069_get_currtime2(char *currtime)
{
	time_t      curr_time;
    struct tm   *curr_tm;
	
	curr_time = time(NULL);
    curr_tm = gmtime(&curr_time);
	sprintf(currtime, "%04d-%02d-%02d:%02d-%02d-%02d", 
	(curr_tm->tm_year + 1900), curr_tm->tm_mon + 1, curr_tm->tm_mday, 
		curr_tm->tm_hour+8, curr_tm->tm_min,curr_tm->tm_sec);

	return currtime;
}

static char* _tr096_get_log_filename(char *filename,int length)
{
	char 	    szStartTime[32] = {0};	
	char 	    szEndTime[32] = {0};	
	char      	local_ip[80]={0};
	char		local_stbid[48]={0};
	static int	i=0;
	
	
	char		local_mac[20]={0};
	char		local_userid[64]={0};
	

    struct tm   *curr_tm;
	char 		*value;
	int 		len;

	if( filename==NULL )
	{
		return NULL;
	}	
	//printf("_tr096_get_log_filename 1\n");
	curr_tm = gmtime(& m_tr069_params.shlog.uplog_starttime);

	//printf("_tr096_get_log_filename 2\n");
    //获得当前时间
	sprintf(szStartTime, "%04d%02d%02d%02d%02d%02d", 
		(curr_tm->tm_year + 1900), curr_tm->tm_mon + 1, curr_tm->tm_mday, 
		curr_tm->tm_hour+8, curr_tm->tm_min,curr_tm->tm_sec);
	//printf("_tr096_get_log_filename 3\n");
	_tr069_get_currtime(szEndTime);
	//printf("_tr096_get_log_filename 4\n");
	sk_tr069_get_stb_ip(NULL,&value,&len);
	strcpy(local_ip,value);
    _tr069_format_ip(local_ip);
	//printf("_tr096_get_log_filename 5,local_ip=%s\n",local_ip);
	sk_tr069_get_stb_mac(NULL,&value,&len);
	strcpy(local_mac,value);
	_tr069_format_mac(local_mac);
	//printf("_tr096_get_log_filename 6,local_mac=%s\n",local_mac);
	sk_tr069_get_stbid(NULL,&value,&len);
	strcpy(local_stbid,value);
//	printf("_tr096_get_log_filename 7,local_stbid=%s\n",local_stbid);
	sk_tr069_get_authuser(NULL,&value,&len);
	strcpy(local_userid,value);
	
	//printf("_tr096_get_log_filename 8,local_userid=%s\n",local_userid);
	#if 1
	//<用户名>_<STBID>_<MAC地址>_<IP地址>_<开始记录时间>_<结束记录时间>.log
    snprintf(filename, length,"%s/%s_%s_%s_%s_%s_%s.log",SK_LOG_PATH,local_userid,local_stbid,local_mac,local_ip,szStartTime,szEndTime);
	evcpe_info(__func__ , "[_tr096_get_log_filename] filename:%s\n",filename);
	//snprintf(filename, length,"%s/%d.log",SK_LOG_PATH,i++);
	#else

	#endif
	return filename;
}


//按照规范将各参数按照格式保存到文件中,最好先存放到缓冲区中,
//最后一次性存入文件,可减少flash的读写次数
static int _tr069_set_log_info(int log_type,char *log_info,int log_value)
{
	char		currtime[32]={0};
	char		buffer[1024]={0};

	int			len;
	int 		index;
	//printf("[_tr069_set_log_info] 1\n");
	if(m_tr069_params.shlog.log_enable<1)
		return 0;
	//printf("[_tr069_set_log_info] 2\n");
	
	_tr069_get_currtime2(currtime);
	switch(log_type)
	{
		case ENUM_LOG_RTSP_INFO:
			len=snprintf(buffer,sizeof(buffer),"%s|RTSPInfo|%s\r\n",currtime,log_info);
			break;
		case ENUM_LOG_HTTP_INFO:
			len=snprintf(buffer,sizeof(buffer),"%s|HTTPInfo|%s\r\n",currtime,log_info);
			break;
		case ENUM_LOG_IGMP_INFO:
			len=snprintf(buffer,sizeof(buffer),"%s|IGMPInfo|%s\r\n",currtime,log_info);
			break; 
		case ENUM_LOG_ERR_INFO:
			len=snprintf(buffer,sizeof(buffer),"%s|ERROR|%s\r\n",currtime,log_info);
			break;
		case ENUM_LOG_PKG_TOTAL_ONE_SEC:
			len=snprintf(buffer,sizeof(buffer),"%s|PkgTotalOneSec|%d\r\n",currtime,log_value);
			break;
		case ENUM_LOG_BYTE_TOTAL_ONE_SEC:
			len=snprintf(buffer,sizeof(buffer),"%s|ByteTotalOneSec|%d\r\n",currtime,log_value);
			break;
		case ENUM_LOG_PKG_LOST_RATE:
			len=snprintf(buffer,sizeof(buffer),"%s|PkgLostRate|%d\r\n",currtime,log_value);
			break;
		case ENUM_LOG_AVARAGE_RATE:
			len=snprintf(buffer,sizeof(buffer),"%s|AvarageRate|%d\r\n",currtime,log_value);
			break;
		case ENUM_LOG_BUFFER:
			len=snprintf(buffer,sizeof(buffer),"%s|BUFFER|%d\r\n",currtime,log_value);
			break;
	}
	//printf("[_tr069_set_log_info] 3\n");
	if(m_tr069_params.shlog.log_type<1)  //文件方式
	{
		//printf("[_tr069_set_log_info] 4\n");
		if(m_tr069_params.shlog.buffer_index<SK_LOG_ARRAY_SIZE)
		{
			index=m_tr069_params.shlog.buffer_index;
			//printf("[_tr069_set_log_info] here,index=%d!\n",index);
			if(m_tr069_params.shlog.buffer_queue[index].flag==0)
			{
				//printf("[_tr069_set_log_info] here 2,index=%d!\n",index);
				memset(m_tr069_params.shlog.buffer_queue[index].log_buffer,0,sizeof(m_tr069_params.shlog.buffer_queue[index].log_buffer));
				memcpy(m_tr069_params.shlog.buffer_queue[index].log_buffer,buffer,len);
				m_tr069_params.shlog.buffer_queue[index].flag=1;
				
				m_tr069_params.shlog.buffer_index++;
				if(m_tr069_params.shlog.buffer_index>=SK_LOG_ARRAY_SIZE)
					m_tr069_params.shlog.buffer_index=0;
				
			}	
			
		}
	}
	else
	{
		//printf("[_tr069_set_log_info] log_type=%d,log_flag=%d\n",log_type,m_tr069_params.shlog.log_flag);
		switch(log_type)
		{
			case ENUM_LOG_RTSP_INFO:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_RTSP_INFO],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
			case ENUM_LOG_HTTP_INFO:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_HTTP_INFO],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
			case ENUM_LOG_IGMP_INFO:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_IGMP_INFO],buffer);
				m_tr069_params.shlog.log_flag=1;
				break; 
			case ENUM_LOG_ERR_INFO:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_ERR_INFO],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
/*			case ENUM_LOG_PKG_TOTAL_ONE_SEC:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_PKG_TOTAL_ONE_SEC],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
			case ENUM_LOG_BYTE_TOTAL_ONE_SEC:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_BYTE_TOTAL_ONE_SEC],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
			case ENUM_LOG_PKG_LOST_RATE:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_PKG_LOST_RATE],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
			case ENUM_LOG_AVARAGE_RATE:
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_AVARAGE_RATE],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
			case ENUM_LOG_BUFFER:	
				strcpy(m_tr069_params.shlog.log_info[ENUM_LOG_BUFFER],buffer);
				m_tr069_params.shlog.log_flag=1;
				break;
*/				
		}		
	}
   return 0;
}

int sk_tr069_write_logfile()
{
	int j;
	//LOGI("[_tr069_logmsg_file_upload] create thefile %s\n",log_filename);
	evcpe_info(__func__ , "liangzhen:enter sk_tr069_write_logfile");
	if(m_tr069_params.shlog.log_file_handle!=NULL)
	{
		fclose(m_tr069_params.shlog.log_file_handle);
		evcpe_info(__func__ , "[tr06] fclose ,csv_file=0x%x\n",m_tr069_params.shlog.log_file_handle);
		m_tr069_params.shlog.log_file_handle=NULL;
	}	
	m_tr069_params.shlog.write_total=0;
	for(j=0;j<SK_LOG_FILE_ARRAY_SIZE;j++)
	{
		if(m_tr069_params.shlog.log_file[j].flag==0)
		{
			strcpy(m_tr069_params.shlog.log_file[j].log_file,m_tr069_params.shlog.log_file_name);
			m_tr069_params.shlog.log_file[j].flag=1;
			break;
		}
	}

	
	m_tr069_params.shlog.uplog_starttime=time(NULL);
	return 0;	
}

static int _tr069_logmsg_file_create()
{

	//static int		write_total=0;
	static int 		write=0;
	//static char		log_filename[512]={0};
	int 			i;
	int 			j;
	int 			len;
	int 			ret=0;
	
	i=0;
	//printf("[_tr069_logmsg_file_upload] enter!\n");
	while(i<SK_LOG_ARRAY_SIZE)
	{
		//printf("[_tr069_logmsg_file_upload] i=%d!\n",i);
		if(m_tr069_params.shlog.buffer_queue[i].flag>0)
		{
		//	printf("[_tr069_logmsg_file_upload] 1\n");
			if( m_tr069_params.shlog.log_file_handle==NULL)
			{
				
			//	printf("log_filename=%s\n",log_filename);
				memset(m_tr069_params.shlog.log_file_name,0,sizeof(m_tr069_params.shlog.log_file_name));
				_tr096_get_log_filename(m_tr069_params.shlog.log_file_name,sizeof(m_tr069_params.shlog.log_file_name));
			//	strcpy(log_filename,"/tmp/kangzh.csv");
		  	   
			    m_tr069_params.shlog.log_file_handle = fopen (m_tr069_params.shlog.log_file_name, "w");
				if (m_tr069_params.shlog.log_file_handle == NULL)
				{
					printf("can not open %s\n", m_tr069_params.shlog.log_file_name);
					return -1;
				}
				 printf("[tr06] fopen ,csv_file=0x%x,filename=%s\n",m_tr069_params.shlog.log_file_handle,m_tr069_params.shlog.log_file_name);
				//printf("log_filename ok\n");
			}
			//printf("log_filename 1\n");
			len=strlen(m_tr069_params.shlog.buffer_queue[i].log_buffer);
			write=fwrite(m_tr069_params.shlog.buffer_queue[i].log_buffer,len,1,m_tr069_params.shlog.log_file_handle);
			//printf("log_filename 2\n");
			m_tr069_params.shlog.write_total+=len;
			//printf("[_tr069_logmsg_file_upload] data is write!write=%d,write_total=%d\n",write,write_total);
			m_tr069_params.shlog.buffer_queue[i].flag=0;
			if(m_tr069_params.shlog.write_total>=128*1024)  //128
			{

				sk_tr069_write_logfile();

				/*ret=_logmsg_send_by_ftp(log_filename);
				if(ret<0)
					return 0;
				*/	
				
			}				


		}	
		i++;	
			
	}
	return 0;
}
static int _tr069_logmsg_protocol_upload()
{
	int ret=0;
	if(m_tr069_params.shlog.log_flag>0)
	{
		evcpe_info(__func__ , "[_tr069_logmsg_protocol_upload] start!\n");
		ret=sk_tr069_log_periodic();
		m_tr069_params.shlog.log_flag++;

	}	
	return ret;
}



int sk_tr069_reset_logmsg_flag()
{
	if(m_tr069_params.shlog.log_flag>=2)  //允许认证的代码中，需要上传2次才能置位
		m_tr069_params.shlog.log_flag=0;
	return 0;
}

int sk_tr069_reset_valuechange_flag()
{
	if(m_tr069_params.value_changed>=2)  //允许认证的代码中，需要上传2次才能置位
	{
		m_tr069_params.value_changed=0;
		m_tr069_params.value_changed_flag=0;
	}	
	return 0;
}

int sk_tr069_set_start_time()
{
	m_tr069_params.shlog.uplog_starttime=time(NULL);
	return 0;
}

int sk_tr069_log_proc()
{

	if(m_tr069_params.shlog.log_enable<1)
		return 0;
	//printf("sk_tr069_log_proc\n");
	if(m_tr069_params.shlog.log_type<1)  //文件方式
	{
		_tr069_logmsg_file_create();

	}
	else
	{
		_tr069_logmsg_protocol_upload();
	}
	return 0;
}

//统计并发送线程
static void logmsg_work_thread()
{
	int i = 0;
	int ret = 0;
	
 	pthread_detach(pthread_self());
    
    while(1)
    {
		//LOGI("logmsg_work_thread\n");
		for(i=0;i<SK_LOG_FILE_ARRAY_SIZE;i++)
		{
			if(m_tr069_params.shlog.log_file[i].flag==1)
			{
				//printf("logmsg_work_thread,%s\n",m_tr069_params.shlog.log_file[i].log_file);
				ret=_logmsg_send_by_ftp(m_tr069_params.shlog.log_file[i].log_file);
				if(ret<0)
					continue;
				m_tr069_params.shlog.log_file[i].flag=0;
				//remove(m_tr069_params.shlog.log_file[i].log_file);
			}
		}
		
		sleep(1);		
			
	}
	printf("Logmsg exit.\n");

}

int sk_tr069_logmsg_listen_start()
{

    if (pthread_create (&m_tr069_params.shlog.logmsg_thread, NULL, (void*)logmsg_work_thread, NULL) < 0)
        return 0;
    
	return 1;
}


#ifdef TR069_ANDROID

int sk_tr069_add_boot_status(int value)
{
    char buffer[32]={0};
    char buffer2[32]={0};
	sk_func_porting_params_get("tr069_event",buffer,sizeof(buffer));

    if(!strcasecmp(buffer,"none"))
    {
        snprintf(buffer2,sizeof(buffer),"%d",value);		
    }
    else
    {
		char str_value[20]={0};

		sprintf(str_value,"%d",value);
		if(strstr(buffer,str_value)==NULL) //防止重复添加
        	snprintf(buffer2,sizeof(buffer2),"%d,%s",value,buffer);
		else
			strcpy(buffer2,buffer);
		
    }
    evcpe_info(__func__ , "[tr069]write tr069_event=%s\n",buffer2);
	return sk_func_porting_params_set("tr069_event",buffer2);
    
}



int sk_tr069_set_boot_status(int value)
{
    char buffer[32]={0};
    
    snprintf(buffer,sizeof(buffer),"%d",value);		
 
    //LOGI("[tr069]write tr069_boot=%s\n",buffer);
	evcpe_info(__func__ , "sk_tr069_set_boot_status buf:%s!",buffer);
	return sk_func_porting_params_set("tr069_event",buffer);
}



int  sk_tr069_reset_boot_status()
{
    char buffer[32]={0};

    snprintf(buffer,sizeof(buffer),"%d",1);		
    evcpe_info(__func__ , "[tr069]reset boot status");
	return sk_func_porting_params_set("tr069_event",buffer);
}



#endif

int sk_tr069_set_qos_enable(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	
	char buffer_local[10]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

	return sk_func_porting_params_set("qos_enable",buffer_local);

	return 0;
}

DLLEXPORT_API int sk_tr069_get_alarmcode(struct evcpe_attr *attr,const char **value, unsigned int *len)
{		
	static char buf[128]={0};
	evcpe_info(__func__ , "sk_tr069_get_alarmcode errorcode:%s\n",m_tr069_params.alarm.errorcode);
	if(m_tr069_params.alarm.errorcode[0]='\0')
	{
			sprintf(buf,"%d",0);
	}
	else
	{
			strcpy(buf,m_tr069_params.alarm.errorcode);
	}
	*value=buf;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;	
	}
	evcpe_info(__func__ , "sk_tr069_get_alarmcode errorcode:%s\n",*value);
	memset(m_tr069_params.alarm.errorcode,0,sizeof(m_tr069_params.alarm.errorcode));
	return 0;
}

DLLEXPORT_API int sk_tr069_get_alarm_starttime(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char starttime[128]={0};
	//20120105 12:03:01
	char mon[6]={0};
	char day[6]={0};
	char hour[6]={0};
	char min[6]={0};
	char sec[6]={0}
	;
	time_t   start;
	struct   tm     *timenow;  
	time(&start);
	timenow=localtime(&start);  
  	if(timenow->tm_mon+1<10)
  	{
  		sprintf(mon,"0%d",timenow->tm_mon+1);
  	}
	else
	{
			sprintf(mon,"%d",timenow->tm_mon+1);
	}

	 if(timenow->tm_mday<10)
  	{
  		sprintf(day,"0%d",timenow->tm_mday);
  	}
	else
	{
			sprintf(day,"%d",timenow->tm_mday);
	}

	 if(timenow->tm_hour<10)
  	{
  		sprintf(hour,"0%d",timenow->tm_hour);
  	}
	else
	{
			sprintf(hour,"%d",timenow->tm_hour);
	}

	if(timenow->tm_min<10)
  	{
  		sprintf(min,"0%d",timenow->tm_min);
  	}
	else
	{
			sprintf(min,"%d",timenow->tm_min);
	}

	if(timenow->tm_sec<10)
  	{
  		sprintf(sec,"0%d",timenow->tm_sec);
  	}
	else
	{
			sprintf(sec,"%d",timenow->tm_sec);
	}
	
//	sprintf(starttime,"%d-%s-%s %d:%d:%d",timenow->tm_year+1900,mon,day,timenow->tm_hour,timenow->tm_min,timenow->tm_sec );
   	sprintf(starttime,"%d%s%s%s%s%s",timenow->tm_year+1900,mon,day,hour,min,sec );
  	*value=starttime;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;	
	}
	evcpe_info(__func__ , "sk_tr069_get_alarm_starttime time:%s \n",*value);
	return 0;
}


DLLEXPORT_API int sk_tr069_get_alarm_endtime(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char starttime[128]={0};
	//20120105 12:03:01
	char mon[6]={0};
	char day[6]={0};
	char hour[6]={0};
	char min[6]={0};
	char sec[6]={0};
	time_t   start;
	struct   tm     *timenow;  
	time(&start);
	timenow=localtime(&start);  
  if(timenow->tm_mon+1<10)
  	{
  		sprintf(mon,"0%d",timenow->tm_mon+1);
  	}
	else
	{
			sprintf(mon,"%d",timenow->tm_mon+1);
	}

	 if(timenow->tm_mday<10)
  	{
  		sprintf(day,"0%d",timenow->tm_mday);
  	}
	else
	{
			sprintf(day,"%d",timenow->tm_mday);
	}

	 if(timenow->tm_hour<10)
  	{
  		sprintf(hour,"0%d",timenow->tm_hour);
  	}
	else
	{
			sprintf(hour,"%d",timenow->tm_hour);
	}

	if(timenow->tm_min<10)
  	{
  		sprintf(min,"0%d",timenow->tm_min+2);
  	}
	else
	{
			sprintf(min,"%d",timenow->tm_min+2);
	}

	if(timenow->tm_sec<10)
  	{
  		sprintf(sec,"0%d",timenow->tm_sec);
  	}
	else
	{
			sprintf(sec,"%d",timenow->tm_sec);
	}
	
//	sprintf(starttime,"%d-%s-%s %d:%d:%d",timenow->tm_year+1900,mon,day,timenow->tm_hour,timenow->tm_min,timenow->tm_sec );
   	sprintf(starttime,"%d%s%s%s%s%s",timenow->tm_year+1900,mon,day,hour,min,sec );
  *value=starttime;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;	
	LOGI("sk_tr069_get_alarm_endtime time:%s \n",*value);
	return 0;
}


/******************************************************************************************
sunjian:
*******************************************************************************************/
DLLEXPORT_API int sk_tr069_set_qos_flag(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
       char szurl[10]={0};
	int bufflag=0;
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;
	
	bufflag=atoi(buffer);
	printf("sk_tr069_set_qos_flag begin bufflag:%d,buffer:%s\n",bufflag,buffer);
       if(bufflag==1)
       {
       	sk_api_params_set("qos_boot","1");
       }
	else
	{
		sk_api_params_set("qos_boot","0");
	}      

	return 0;
}
//
DLLEXPORT_API int sk_tr069_get_qos_flag(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char tenobuf[4]={0};
	printf("sk_tr069_get_qos_flag begin\n");
	 sk_api_params_get("qos_boot", tenobuf ,sizeof(tenobuf));
	if(strlen(tenobuf)<=0)
		strcpy(tenobuf,"0");	

	else
	{
		int ret=atoi(tenobuf);
		if((ret==1)||(ret==4))
		{
			strcpy(tenobuf,"1");	
		}
		else
		     strcpy(tenobuf,"0");
	}
	*value=tenobuf;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;	

	return 0;
}

DLLEXPORT_API int sk_tr069_set_qos_infoflag(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int ret=0;
       char szurl[10]={0};
	//printf("sk_tr069_set_qos_infoflag begin\n");
	
	if ((buffer==NULL) || 	(len<=0))
		return -1;
	ret=atoi(buffer);
       if(ret==1)
       {
       	sk_api_params_set("qos_boot","2");
       }
	else
	{
		sk_api_params_set("qos_boot","0");
	}
       

	return 0;
}


DLLEXPORT_API int sk_tr069_get_qos_infoflag(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char tenobuf[4]={0};
	//printf("sk_tr069_get_qos_infoflag begin\n");
	 sk_api_params_get("qos_boot" , tenobuf , sizeof(tenobuf));
	if(strlen(tenobuf)<=0)
		strcpy(tenobuf,"0");	

	else
	{
		int ret=atoi(tenobuf);
		if((ret==2)||(ret==3))
		{
			strcpy(tenobuf,"1");	
		}
		else
		     strcpy(tenobuf,"0");
	}
	*value=tenobuf;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;	

	return 0;
}


DLLEXPORT_API int sk_tr069_set_fileflag(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	int ret=0;
       char szurl[10]={0};
	
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;
	//printf("sk_tr069_set_qos_flag begin buffer:%s\n",buffer);
	// sk_api_params_get("qos_boot",szurl);
	//if(strlen(szurl)<=0)
	//	return 0;
	ret=atoi(buffer);
	
       if(ret==1) /*文件形式*/
       {
       	
		sk_api_params_set("qos_boot","2");
       }
	else /*协议方式不需要QOS模块*/
	{		
		sk_api_params_set("qos_boot","3");
	}	
       

	return 0;
}



//获取认证密码
DLLEXPORT_API int sk_tr069_get_fileflag(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	int ret=0;
	static char tenobuf[4]={0};
	//printf("sk_tr069_get_fileflag begin\n");
	sk_api_params_get("qos_boot", tenobuf , sizeof(tenobuf));
	if(strlen(tenobuf)<=0)
		strcpy(tenobuf,"0");
	else
	{
		ret=atoi(tenobuf);
		if((2==ret)||(ret==3))
		{
			strcpy(tenobuf,"1");
		}
		else
			strcpy(tenobuf,"0");
	}
	*value=tenobuf;
	
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;
	

	return 0;
}


int sk_tr069_get_alarminfo(sk_alarm_t* alarminfo)
{
	//printf("sk_tr069_get_alarminfo alarm_switch:%d\n",m_tr069_params.alarm.alarm_switch);
	//memcpy(alarminfo,&(m_tr069_params.alarm),sizeof(sk_alarm_t));
	
	alarminfo->alarm_switch=m_tr069_params.alarm.alarm_switch;
	alarminfo->alarm_level=m_tr069_params.alarm.alarm_level;
	strcpy(alarminfo->disk_alarm,m_tr069_params.alarm.disk_alarm);

	
	strcpy(alarminfo->cpu_alarm,m_tr069_params.alarm.cpu_alarm);
	
	strcpy(alarminfo->memory_alarm,m_tr069_params.alarm.memory_alarm);

	strcpy(alarminfo->band_width_alarm,m_tr069_params.alarm.band_width_alarm);
	strcpy(alarminfo->packet_lost_alarm,m_tr069_params.alarm.packet_lost_alarm);
   return 0; 
}


int sk_tr069_set_errorcode(int code)
{
	//sprintf(m_tr069_params.alarm.errorcode,"%d",code);
	snprintf(m_tr069_params.alarm.errorcode,sizeof(m_tr069_params.alarm.errorcode),"%d",code);
	return 0;
}
/*
int sk_tr069_get_alarmcode(struct evcpe_attr *attr,const char **value, unsigned int *len)
{			
		printf("sk_tr069_get_alarmcode 000 errorcode:%s\n",m_tr069_params.alarm.errorcode);
	
		*value=m_tr069_params.alarm.errorcode;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;	
	printf("sk_tr069_get_alarmcode errorcode:%s, m_tr069_params.alarm.errorcode:%s\n",*value,m_tr069_params.alarm.errorcode);
	//memset(m_tr069_params.alarm.errorcode,0,sizeof(m_tr069_params.alarm.errorcode));
	return 0;
}
*/

DLLEXPORT_API int sk_tr069_get_alarmid(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

	static char buffer_local[10]={0};
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_alarmid);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;

	return 0;

	printf("sk_tr069_get_alarmid value:%s,id:%d,len:%d",*value,m_alarmid,*len);
	return 0;
}

DLLEXPORT_API int sk_tr069_get_alarmtype(struct evcpe_attr *attr,const char **value, unsigned int *len)
{

	static char buffer_local[10]={0};
	snprintf(buffer_local,sizeof(buffer_local),"%d",m_arlamtype);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;	

	printf("sk_tr069_get_alarmid value:%s,id:%d,len:%d",*value,m_alarmid,*len);
	return 0;

}

/*sunjian:华为平台 
type:0  清除 告警
type:1 上报告警
*/

int sk_tr069_alarm_reportaction(int type,int code)
{
	int rc;
	evcpe_t *thiz = get_evcpe_object();
  
  /*
	if((code!=ALARM_UPGRADE_FAILURE_CODE)&&
		(code!=ALARM_DISK_FAILURE_CODE)&&
		(code!=ALARM_CPU_FAILURE_CODE)&&
		(code!=ALARM_LOSTPACKET_FAILURE_CODE)&&
		(code!=ALARM_AUTH_FAILURE_CODE)&&
		(code!=ALARM_JOINCHANNEL_FAILURE_CODE))
		{
			return 0;
		}
*/
	//alarm_code=code;
	//sprintf(m_tr069_params.alarm.errorcode,"%d",code);
//	 sk_tr069_set_errorcode(code);
//	printf("sk_tr069_alarm_report m_tr069_params.alarm.errorcode:%s,alarm_code:%d \n",m_tr069_params.alarm.errorcode,alarm_code);
      evcpe_handle_del_event(thiz->cpe);
       if(type==1)
   	{
   		
		if ((rc = evcpe_repo_add_event(thiz->cpe->repo, "X CTC ALARM", "")))
		{
			evcpe_error(__func__, "failed to add boot event");
			goto finally;
		}
   	}
	 else
	 {
	 		if ((rc = evcpe_repo_add_event(thiz->cpe->repo, "X CTC CLEARALARM", "")))
			{
				evcpe_error(__func__, "failed to add boot event");
				goto finally;
			}
	 }
	
	thiz->cpe->event_flag=EVCPE_NO_BOOT;

	if ((rc = evcpe_start_session(thiz->cpe))) //建立会话，并组装inform包，发送出去
	{
		evcpe_error(__func__, "failed to start session");
		goto finally;
	}


finally:
		return rc;
}


int sk_tr069_alarm_report(int type,  int code)
{
	
	 //m_tr069_params.alarm.alarm_level
       
	/*四川告警定义:
	2000 CPU占用率CPU占用率过高80%  主要/次要告警
	2001内存占用率内存占用率过高80%主要/次要告警
	2002磁盘占用率磁盘占用率过高80%主要/次要告警
	2003解码解码失败次要告警
	2004解密解密失败主要告警
	2005缓冲缓冲溢出（上溢/下溢）主要告警
	2050丢包率UDP/RTP 5% 主要告警
       3000连接EPG服务器连接EPG失败主要告警
       3001访问媒体服务器访问媒体服务器失败主要告警
       3002文件服务器连接文件服务器连接失败主要告警
       3003媒体格式媒体格式不支持主要告警
	*/
	if(code<=2002)  	                                {   m_arlamtype=0; alarmlevel=2; }
	else if((code>2002)&&(code<=2004))  { m_arlamtype=0; alarmlevel=3;}
	else if(code==2005)                            {  m_arlamtype=2; alarmlevel=3;}
	else if(code==2006)   {  m_arlamtype=2; alarmlevel=1;}
	else if(code>=2050)   {  m_arlamtype=3;alarmlevel=1;}

	 if(0==type)
        {
        	//m_tr069_params.alarm.alarm_level=6;
        	alarmlevel=6;
        }
	else
	{
		m_alarmid++;
	}
	//
	if((alarmlevel>m_tr069_params.alarm.alarm_level)&&(alarmlevel!=6))
	{
		printf("sk_tr069_alarm_report drop alarm\n");
		return 0;
	}
	//printf("sk_tr069_alarm_report begin...\n");
	m_tr069_params.alarm.alarmflag=1;
	sk_tr069_set_errorcode(code);
	sk_tr069_alarm_reportaction(type,code);
	return 0;
}


int sk_is_node_lalarmmsg(char *node_name)
{												  // m_tr069_params.alarm.alarmflag
	//printf("node_name:%s,log_enable:%d\n",node_name,m_tr069_params.alarm.alarmflag);
 	if(node_name==NULL ||m_tr069_params.alarm.alarmflag<1)
		return 0;
	
	// printf("--sk_is_node_lalarmmsg--node_name:%s,p:%s\n",node_name,strstr(node_name,"1"));   

	if(strstr(node_name,"Device.DeviceInfo.X_CTC_IPTV_Alarm.1.")!=NULL)   
	{
		//printf("node_name:%s,log_enable:%d\n",node_name,m_tr069_params.shlog.log_enable);
		 m_tr069_params.alarm.alarmflag++;
		return 1;
	}
	
	return 0;
}
int sk_set_node_alarmmsg_flag(int flag)
{
	 m_tr069_params.alarm.alarmflag=flag;
	 return 0;
}

int sk_save_syslog_to_file(char *log_file,char* buf,int  length)
{
    time_t      curr_time;
    struct tm   *curr_tm;
   	FILE*       csv_file = NULL;
    char        log_file_name[256]={0};

    int         left=0;
    int         write=0;
    int         write_total=0;

    if((log_file==NULL) || (length<=0))
        return -1;
    
    
    csv_file = fopen (log_file, "w");
	if (csv_file == NULL)
	{
		printf("can not open %s\n", csv_file);
		return -1;
	}
    left=length;
  
    while(left>0)
    {
        write=fwrite(buf+write_total,sizeof(int),1,csv_file);
        write_total+=write;
        left-=write;
    }
   	fclose (csv_file);
   
	return 0;
}

int sk_tr069_xingneng_report()
{
	m_tr069_params.xineng_inform=1;
	return 0;
	//sk_tr069_xinneng_reportaction();
}


int  sk_tr069_xingneng_get()
{
	return m_tr069_params.xineng_inform;
}
int sk_is_node_xingneng(char *node_name)
{												  // m_tr069_params.alarm.alarmflag
	//printf("sk_is_node_xingneng node_name:%s,log_enable:%d\n",node_name,m_tr069_params.xineng_inform);
 	if(node_name==NULL ||m_tr069_params.xineng_inform<1)
		return 0;
	
	//printf("--sk_is_node_lalarmmsg--node_name:%s,p:%s\n",node_name,strstr(node_name,"1"));   

	if(strstr(node_name,"X_CTC_IPTV.Monitor.")!=NULL)   
	{
		//printf("node_name:%s,log_enable:%d\n",node_name,m_tr069_params.xineng_inform);
		 m_tr069_params.xineng_inform++;
		return 1;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_get_log_parameterList(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char buffer_local[256]={0};	

	*value=buffer_local;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
		*len=0;
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_log_parameterList(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	return 0;
}



/***********************************************************************************************/

#if 0
#define SK_API_NET_INITIAL_OFFSET				(0)
#define SK_API_NET_ROUTE_FILE_LINE_CONTENT_CNT	(10)
#define SK_API_NET_ROUTE_FLAGS_UG				(0x3)

#define SK_API_NET_ROUTE_FILE					"/proc/net/route"



#define OPEN_SOCKET(fd)					\
 do{									  \
	 fd = socket(AF_INET,SOCK_STREAM,0); \
	 if (fd < 0)						  \
	 	return -1;					  \
 }while(0)

#define CLOSE_SOCKET(fd)	\
  do{						  \
	  if (fd > 0)			  \
		  close(fd);		  \
  }while(0)


int get_ipaddr(const char *interface_name, unsigned long *ipaddr)
{
    int s;
    OPEN_SOCKET(s);

    struct ifreq ifr;
    bzero((char *)&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name));

    if(ioctl(s, SIOCGIFADDR, &ifr) < 0)
    {
    
        CLOSE_SOCKET(s);
        return -1;
    }

    struct sockaddr_in *ptr;

    ptr = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_addr;
    memcpy((char *)ipaddr, (char *)&ptr->sin_addr, sizeof(unsigned long));

    CLOSE_SOCKET(s);
    return 0;
}

int get_route(const char *interface_name, unsigned long *route)
{
	int ret = -1;
	char buff[256];
	int line_index = 0;
	int flgs, ref, use, metric, mtu, win, ir;
	unsigned long int destination_digit, gateway_digit, mask_digit;
	
	FILE *fp = fopen(SK_API_NET_ROUTE_FILE, "r");
	
	while(fgets(buff, sizeof(buff), fp) != NULL)
	{
		if(line_index)
		{
			int ifl = SK_API_NET_INITIAL_OFFSET;

			while(buff[ifl] != ' ' && buff[ifl] != '\t' && buff[ifl] != '\0')
			{
				ifl++;
			}
			buff[ifl] = '\0';

			if (memcmp(buff, interface_name, strlen(interface_name)))
			{
				continue;
			}
			
			if(sscanf(buff + ifl + 1, "%lx%lx%x%d%d%d%lx%d%d%d",
			          &destination_digit, &gateway_digit, (unsigned int *)&flgs, &ref, &use, &metric, &mask_digit, &mtu, &win,
			          &ir) != SK_API_NET_ROUTE_FILE_LINE_CONTENT_CNT)
			{
				break;
			}
			ifl = SK_API_NET_INITIAL_OFFSET;
			if(SK_API_NET_ROUTE_FLAGS_UG == flgs)
			{
				if (NULL != route)
				{
					*route = gateway_digit;
					ret = 0;
					break;
				}
			}
		}
		line_index++;
	}

	if(fp)
	{
		fclose(fp);
	}
	return ret;
}

int get_netmask(const char *interface_name, unsigned long *netmask)
{
    int s;
    OPEN_SOCKET(s);
    struct ifreq ifr;
    bzero((char *)&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name));

    if(ioctl(s, SIOCGIFNETMASK, &ifr) < 0)
    {
        CLOSE_SOCKET(s);
        return -1;
    }

    struct sockaddr_in *ptr;
    ptr = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_netmask;

    memcpy((char *)netmask, (char *)&ptr->sin_addr, sizeof(unsigned long));

    //SK_ERR("Netmask:%s\n", inet_ntoa(ptr->sin_addr));

    CLOSE_SOCKET(s);
    return 0;
}

int sk_network_lan_get_network_info(const char *dev, sk_api_net_realtime_info_t *p_info)
{
    sk_api_net_realtime_info_t info = {0};
    FILE *fp = NULL;
    char line[256] = {0};
    char *iter = NULL;
    int ret = -1;
	char dec[64] = {0};
	char ip[32] = {0};
	unsigned long ipaddr = 0;
    int got_dns1 = 0;
    int got_dns2 = 0;
    do
    {

        if(p_info == NULL || dev == NULL)
        {
            evcpe_error(__func__ ,"Invalid parameters!\n");
            break;
        }
        memset(&info, 0, sizeof(info));

        /* ip */
        ret = get_ipaddr(dev, &ipaddr);
        strlcpy(info.ip, inet_ntoa(*(struct in_addr *)&ipaddr), sizeof(info.ip));

        /* mask */
		ipaddr = 0;
        ret = get_netmask(dev, &ipaddr);
        strlcpy(info.netmask, inet_ntoa(*(struct in_addr *)&ipaddr)	, sizeof(info.netmask));

        /* gateway */
		ipaddr = 0;
		ret = get_route(dev, &ipaddr);
        strlcpy(info.gateway, inet_ntoa(*(struct in_addr *)&ipaddr)	, sizeof(info.gateway));

        

        /*dns*/
        fp = fopen("/etc/resolv.conf", "r");
        if(fp != NULL)
        {
            while(fgets(line, 256, fp) != NULL)
            {
                if(strstr(line, "nameserver ") != NULL)
                {
                    sscanf(line , "%s%s", dec, ip);
                    if(!got_dns1)
                    {
                        strlcpy(info.dns1, ip, sizeof(info.dns1));
                        got_dns1 = 1;
                    }
                    else if(!got_dns2)
                    {
                        strlcpy(info.dns2, ip, sizeof(info.dns2));
                        got_dns2 = 1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            fclose(fp);
            fp = NULL;
        }
        else
        {
            SK_ERR("fopen:%s\n", strerror(errno));
        }
	

        if(-1 == ret)
        {
            return -1;
        }
        else
        {
            memcpy(p_info, &info, sizeof(info));
            return 0;
        }
    }
    while(0);

    return -1;
}

static int get_pppoe_net_realtime_info(sk_api_net_realtime_info_t *p_info)
{
	int ret = -1;
	int got_dns1 = 0;
	int got_dns2 = 0;
    sk_api_net_realtime_info_t info;
    FILE *pf = NULL;
    char line[256] = {0};
    char *iter = NULL;
 
	FILE *file = NULL;
	char *end = NULL;
	int len = 0;
    do
    {
        if(p_info == NULL)
        {
            evcpe_error(__func__ , "Invalid parameters!\n");
            break;
        }
        memset(&info, 0, sizeof(info));

        //sk_ipc_system("ifconfig ppp0 | grep \"inet addr\" > /var/ppp0.conf");
        system("ifconfig ppp0 | grep \"inet addr\" > /var/ppp0.conf");
        file = fopen("/var/ppp0.conf", "r");
        if(file == NULL)
        {
            evcpe_error(__func__ , "Cannot open ppp0.conf!\n");
            break;
        }
        // inet addr:183.37.2.53  P-t-P:183.37.0.1  Mask:255.255.255.255
  
        if(fgets(line, 256, file) != NULL)
        {
            do
            {
                // ip
                iter = strstr(line, "inet addr:");
                if(iter == NULL)
                {
                    break;
                }
                iter += strlen("inet addr:");
                end = strchr(iter, ' ');
                if(end == NULL)
                {
                    break;
                }
                len = end - iter;
                if(len >= sizeof(info.ip))
                {
                    break;
                }
                memcpy(info.ip, iter, len);
                info.ip[len] = '\0';
                SK_DBG("IP: [%s]\n", info.ip);

                // gateway
                iter = strstr(line, "P-t-P:");
                if(iter == NULL)
                {
                    break;
                }
                iter += strlen("P-t-P:");
                end = strchr(iter, ' ');
                if(end == NULL)
                {
                    break;
                }
                len = end - iter;
                if(len >= sizeof(info.gateway))
                {
                    break;
                }
                memcpy(info.gateway, iter, len);
                info.gateway[len] = '\0';
                SK_DBG("GATEWAY: [%s]\n", info.gateway);

                // mask
                iter = strstr(line, "Mask:");
                if(iter == NULL)
                {
                    break;
                }
                iter += strlen("Mask:");
                end = strchr(iter, '\n');
                if(end == NULL)
                {
                    break;
                }
                len = end - iter;
                if(len >= sizeof(info.netmask))
                {
                    break;
                }
                memcpy(info.netmask, iter, len);
                info.netmask[len] = '\0';
                SK_DBG("NETMASK: [%s]\n", info.netmask);
            }
            while(0);
        }
        fclose(file);

        /*dns*/
		got_dns1 = 0;
		got_dns2 = 0;
        pf = fopen("/etc/resolv.conf", "r");
        if(pf != NULL)
        {
            while(fgets(line, 256, pf) != NULL)
            {
                iter = strstr(line, "nameserver ");

                if(iter == NULL)
                {
                    continue;
                }
                else
                {
                    iter += strlen("nameserver ");

                    if(got_dns1 == 0)
                    {
                        strncpy(info.dns1, iter, sizeof(info.dns1));
                        iter = strchr(info.dns1, '\n');
                        if(iter != NULL)
                        {
                            *iter = '\0';
                        }
                        SK_DBG("DNS1: [%s]\n", info.dns1);
                        got_dns1 = 1;
                    }
                    else if(got_dns2 == 0)
                    {
                        strncpy(info.dns2, iter, sizeof(info.dns2));
                        iter = strchr(info.dns2, '\n');
                        if(iter != NULL)
                        {
                            *iter = '\0';
                        }
                        SK_DBG("DNS2: [%s]\n", info.dns2);
                        got_dns2 = 1;
                    }
                    else
                    {
                        break;
                    }

                    if(got_dns1 == 1 && got_dns2 == 1)
                    {
                        break;
                    }
                }
            }

            fclose(pf);
        }
        else
        {
            evcpe_error(__func__ ,"fopen fali!\n");
            break;
        }

        ret = 0;
        memcpy(p_info, &info, sizeof(info));
    }
    while(0);

    return ret;
}
#endif

int sk_net_check()
{		
    char buffer[128]={0};
    int ret = -1;
   
	sk_func_porting_params_get("lan_ip",buffer,sizeof(buffer));
	LOGI("sk_net_check get ip from mipt buffer = %s\n",buffer);
    if('\0' == buffer[0])
    {
        return -1;
    }
    //检查一下IP地址合法性
    //ret = sk_network_check_ip(buffer);
    ret = 0;  
	return ret;
}

#define STUN_DEFAULT_PORT (3478)

static STUN_Parameter param;

static char *STUN_UCR_TrimAll(char *str)
{
    char *p = str;
    char *p1;
    if(p)
    {
            p1 = p + strlen(str) - 1;
            while(*p && isspace(*p)) p++;
            while(p1 > p && isspace(*p1)) 
	{
		*p1 = '\0';
		p1--;
            }	
    }
    return p;
}

static char *STUN_UCR_ToCase(char *str, bool LowerOrUpper)
{
char *p =str;
while(p && *p)
{
	if(LowerOrUpper) 
		*p = tolower(*p);
	else
		*p = toupper(*p);
	p++;
}
return str;
}


static bool ConvertStringToBool(char *psz, bool *pBool)
{
	char *p;
	if( (psz == NULL) || (strlen(psz) == 0) )
	{
		evcpe_info(__func__ , "psz = %p, len = %d, parameter error", psz, psz ? strlen(psz) : 0);
		return false;
	}
	p = STUN_UCR_TrimAll(psz);
	STUN_UCR_ToCase(p, true);
	
	if((strcmp(p, "true") == 0) || (strcmp(p, "1") == 0)) 
	{
		*pBool = true;
		return true;
	}
	if((strcmp(p, "false") == 0) || (strcmp(p, "0") == 0)) 
	{
		*pBool = false;
		return true;
	}
	
	evcpe_info(__func__ , "unknown , parameter error");
	return false;

}

static bool ConvertHostToIP(struct evcpe *cpe, char *pszUrl)
{
	char *address;
	if(pszUrl == NULL || strlen(pszUrl) == 0)
	{
		evcpe_info(__func__ , "pszUrl = %p, len = %d, parameter error", 
			pszUrl, pszUrl ? strlen(pszUrl) : 0);
		return false;
	}
	
	if ((address = evcpe_dns_cache_get(&cpe->dns_cache, pszUrl)) == NULL) 
	{
		evcpe_info(__func__ , "hostname not resolved: %s, get address failed", pszUrl);
		return false;
	}
	strncpy(pszUrl, address, STUN_STR_LEN);
	return true;
}

static int sk_tr069_stun_collect_enable(struct evcpe *cpe, STUN_Parameter *param)
{
	char sz[STUN_STR_LEN];
	memset(sz, '\0', sizeof(sz));
	
	if((evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNEnable", sz, STUN_STR_LEN) == 0) &&
		(ConvertStringToBool(sz, &(param->STUNEnable)))	)
	{
		evcpe_info(__func__, "STUNEnable == %d", param->STUNEnable);
        if(false == param->STUNEnable)
        {    
            return 0 ;
        }
        return 1;
	}
	else
	{
		param->STUNEnable = false;
		evcpe_warn(__func__, "can not find STUNEnable, uses default.");
        return 0;
	}
    return 0;
}

static bool sk_tr069_stun_collect_address(struct evcpe *cpe, STUN_Parameter *param)
{
	char sz[STUN_STR_LEN];
	memset(sz, '\0', sizeof(sz));

	if((evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNServerAddress", param->STUNServerAddress, STUN_STR_LEN) == 0) &&
		(evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNServerPort", sz, STUN_STR_LEN) == 0) &&
		(strlen(param->STUNServerAddress) >0) && strncmp(param->STUNServerAddress , "stunaddress",strlen("stunaddress")))
	{
		param->STUNServerPort = (unsigned short)atol(sz);
		evcpe_info(__func__, "STUNServerAddress == %s, STUNServerPort = %d", 
			param->STUNServerAddress, param->STUNServerPort);
	}
	else
	{
		if (cpe->acs_url != NULL)
		{
			strncpy(param->STUNServerAddress, 
				cpe->proxy_url ? cpe->proxy_url->host : cpe->acs_url->host, STUN_STR_LEN);
			evcpe_warn(__func__, "can not find STUNEnable, uses acs's ip.");

            evcpe_repo_set(cpe->repo,  "Device.ManagementServer.STUNServerAddress",
                param->STUNServerAddress , STUN_STR_LEN);
		}
		else
		{
			evcpe_info(__func__ , "get acs_url failed.");
			return false;
		}
	}

	if(!ConvertHostToIP(cpe, param->STUNServerAddress))
	{
		evcpe_info(__func__ , "convert url failed.");
		return false;
	}
	
	if(param->STUNServerPort  == 0)
		param->STUNServerPort = STUN_DEFAULT_PORT;

	evcpe_info(__func__, "STUNServerAddress == %s, STUNServerPort = %d", 
		param->STUNServerAddress, param->STUNServerPort);
	return true;

}

static bool sk_tr069_stun_collect_param(struct evcpe *cpe, STUN_Parameter *param)
{
	char sz[STUN_STR_LEN];
	memset(sz, '\0', sizeof(sz));
	
	if((cpe == NULL) || (cpe->repo == NULL) || (param == NULL))
	{
		evcpe_info(__func__ , "cpe(%p)->repo = %p, param = %p, parameter error", cpe, cpe ? cpe->repo : NULL, param);
		return false;
	}

    if(!sk_tr069_stun_collect_enable(cpe, param))//.Device.ManagementServer.STUNEnable
    {
        return false;
    }
	if(!sk_tr069_stun_collect_address(cpe, param))		//.Device.ManagementServer.STUNServerAddress / STUNServerPort
    {
        return false;       
	}			
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNUsername", param->STUNUsername, STUN_STR_LEN);
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNPassword", param->STUNPassword, STUN_STR_LEN);

	memset(sz, '\0', sizeof(sz));
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNMaximumKeepAlivePeriod", sz, STUN_STR_LEN);
	param->STUNMaximumKeepAlivePeriod = atol(sz);
	// TODO: default time
	if(param->STUNMaximumKeepAlivePeriod == 0)
		param->STUNMaximumKeepAlivePeriod = 20000;
	
	memset(sz, '\0', sizeof(sz));
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.STUNMinimumKeepAlivePeriod", sz, STUN_STR_LEN);
	param->STUNMinimumKeepAlivePeriod = atol(sz);
    
	memset(sz, '\0', sizeof(sz));
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.UDPConnectionRequestAddressNotificationLimit", sz, STUN_STR_LEN);
	param->UDPConnectionRequestAddressNotificationLimit = atol(sz);
	
	// TODO: Not provides a default username & password
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.ConnectionRequestUsername", param->ConnectionRequestUsername, STUN_STR_LEN);
	if(strlen(param->ConnectionRequestUsername) == 0)
	{
		evcpe_repo_getcpy(cpe->repo, "Device.DeviceInfo.SerialNumber", param->ConnectionRequestUsername, STUN_STR_LEN);
		evcpe_warn(__func__, "can not find ConnectionRequestUsername, uses default.");
	}
	evcpe_repo_getcpy (cpe->repo,  "Device.ManagementServer.ConnectionRequestPassword", param->ConnectionRequestPassword, STUN_STR_LEN);

    evcpe_repo_getcpy(cpe->repo, "Device.DeviceInfo.SerialNumber", param->sn, sizeof(param->sn));

	STUN_Dump_Parameter(param);		
    
	return true;
	
}

void sk_tr069_stun_on_cmd(evcpe_t* cpe)
{
    evcpe_info(__func__, "sk_tr069_stun_on_cmd begin...\n");
    if(cpe == NULL)
    {
    	evcpe_info(__func__ , "sk_tr069_stun_on_cmd\n");
    	return ;
    }
    memset(&param, '\0', sizeof(param));
    // Collects param;
    if(!sk_tr069_stun_collect_param(cpe, &param))
    {
    	evcpe_info(__func__ , "sk_tr069_stun_collect_param() failed, ignore STUNEnable.");
        STUN_Set_Disable();
        sk_set_nat_stun_flag(0);
        evcpe_worker_add_msg(EVCPE_WORKER_VALUE_CHANGE);
    	return ;
    }

    STUN_OnCmd(cpe, &param);
    	
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_stunenable(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
    
	sk_func_porting_params_get("tr069_stunenbale" , buffer , sizeof(buffer));
	//strcpy(buffer,"ac5entry");
    evcpe_info(__func__ , "sk_tr069_get_stunenable  value=%s\n\n",buffer);

    *value=buffer;

    if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_stunenable(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	evcpe_info(__func__ , "sk_tr069_set_stunenable  value=%s \n",buffer_local);

	sk_func_porting_params_set("tr069_stunenbale",buffer_local);
    return 0;
}



DLLEXPORT_API int sk_tr069_get_stunaddr(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};
    
    memset(buffer , 0 , sizeof(buffer));

    sk_func_porting_params_get("tr069_stunaddr",buffer,sizeof(buffer));
	//strcpy(buffer,"ac5entry");
    evcpe_info(__func__ , "sk_tr069_get_stunaddr  value=%s\n\n",buffer);


	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_stunaddr(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	evcpe_info(__func__ , "sk_tr069_set_stunname  value=%s\n\n",buffer_local);

	sk_func_porting_params_set("tr069_stunaddr",buffer_local);

    return 0;
}


//获取认证密码
DLLEXPORT_API int sk_tr069_get_stunport(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};
	
    memset(buffer , 0 , sizeof(buffer));
    
	sk_func_porting_params_get("tr069_stunport",buffer,sizeof(buffer));
	//strcpy(buffer,"ac5entry");

	*value=buffer;
    
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_stunport(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
    
	evcpe_info(__func__ , "sk_tr069_set_stunname  value=%s\n",buffer_local);
	

	sk_func_porting_params_set("tr069_stunport",buffer_local);

    return 0;
}

DLLEXPORT_API int sk_tr069_get_stunname(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
    
	sk_func_porting_params_get("tr069_stunname",buffer,sizeof(buffer));

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_stunname(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};
	int  len_local=sizeof(buffer_local);
	if ((buffer==NULL) || 	(len<=0))
	{
		return -1;
	}
	
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	evcpe_info(__func__ , "sk_tr069_set_stunname  value=%s\n",buffer_local);
	sk_func_porting_params_set("tr069_stunname",buffer_local);

    return 0;
}



DLLEXPORT_API int sk_tr069_get_stunpwd(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
       
	sk_func_porting_params_get("tr069_stunpwd",buffer,sizeof(buffer));
    evcpe_info(__func__ , "sk_tr069_get_stunpwd  pwd=%s\n\n",buffer);

	*value=buffer;
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_stunpwd(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};

	int  len_local=sizeof(buffer_local);
	if ((buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
 
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);

    evcpe_info(__func__ , "sk_tr069_set_stunpwd  pwd=%s \n",buffer_local);
	sk_func_porting_params_set("tr069_stunpwd",buffer_local);
    return 0;
}


DLLEXPORT_API int sk_tr069_get_udpconnectaddr(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
    evcpe_info(__func__ , "sk_tr069_get_udpconnectaddr =%s\n", STUN_get_UDPConnectionRequestAddress());
	*value= STUN_get_UDPConnectionRequestAddress();
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_udpconnectaddr(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[128]={0};

	int  len_local=sizeof(buffer_local);
	if ( (buffer==NULL) || 	(len<=0) )
		return -1;

	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local,buffer,len_local);
	
	evcpe_info(__func__ , "sk_tr069_set_udpconnectaddr =%s \n",buffer_local);

    return 0;
}

DLLEXPORT_API int sk_tr069_get_natdetect(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	*value = STUN_get_Nat_Detected();
    evcpe_info(__func__ , "sk_tr069_get_natdetect = %s\n",STUN_get_Nat_Detected()); 
	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_natdetect(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char  sk_netdect[16]={"false"};
	int  len_local = sizeof(sk_netdect);
	if ((buffer==NULL) || (len<=0))
	{
		return -1;
	}
	len_local = len_local>len ? len:len_local-1;
	memcpy(sk_netdect , buffer , len_local);
    evcpe_info(__func__ , "sk_tr069_set_natdetect = %s \n", sk_netdect);
    return 0;
}


/*
*func:宽带检测
*
*/

//参数:表示诊断数据的情况，如果终端管理平台要求设置这个值，则可以是： "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"

DLLEXPORT_API int sk_tr069_get_BandwidthDiagnostics_DiagnosticsState(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
        
    //snprintf(buffer, sizeof(buffer)-1 ,"%d", m_tr069_params.bandwidth_diagnostics_result.status);
    sk_func_porting_params_get("tr069_bandwidth_state", buffer , sizeof(buffer));
    
	*value = buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
    
	return 0;
}

DLLEXPORT_API int sk_tr069_set_BandwidthDiagnostics_DiagnosticsState(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	
    char buffer_local[32] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);

    m_tr069_params.bandwidth_diagnostics_param.diagnostics_state = strtol(buffer_local , NULL , 10);
	
    sk_func_porting_params_set("tr069_bandwidth_state", buffer_local);

    return 0;
    
}


DLLEXPORT_API int sk_tr069_set_BandwidthDiagnostics_DownloadURL(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

    char buffer_local[512] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);

    memset(m_tr069_params.bandwidth_diagnostics_param.download_url ,0,sizeof(m_tr069_params.bandwidth_diagnostics_param.download_url));
	
	strncpy(m_tr069_params.bandwidth_diagnostics_param.download_url , buffer_local ,len_local);

    sk_func_porting_params_set("tr069_bandwidth_downloadurl", buffer_local);
    
    return 0;
}



DLLEXPORT_API int sk_tr069_set_BandwidthDiagnostics_Username(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buffer_local[128] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);

    memset(m_tr069_params.bandwidth_diagnostics_param.username ,0,sizeof(m_tr069_params.bandwidth_diagnostics_param.username));
	
	strncpy(m_tr069_params.bandwidth_diagnostics_param.username , buffer_local ,len_local);

    sk_func_porting_params_set("tr069_bandwidth_username", buffer_local);

    return 0;
}




DLLEXPORT_API int sk_tr069_set_BandwidthDiagnostics_Password(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buffer_local[128] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);

    memset(m_tr069_params.bandwidth_diagnostics_param.password ,0,sizeof(m_tr069_params.bandwidth_diagnostics_param.password));
	
	strncpy(m_tr069_params.bandwidth_diagnostics_param.password , buffer_local ,len_local);

    sk_func_porting_params_set("tr069_bandwidth_password", buffer_local);


    return 0;
    
}

DLLEXPORT_API int sk_tr069_get_BandwidthDiagnostics_ErrorCode(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[32]={0};

	memset(buffer,0,sizeof(buffer));

	//sprintf(buffer,"%d",m_tr069_params.bandwidth_diagnostics_result.error_code);
	 sk_func_porting_params_get("tr069_bandwidth_errorcode", buffer , sizeof(buffer));
    
	*value=buffer;
	*len=strlen(*value);
	return 0;
}


DLLEXPORT_API int sk_tr069_get_BandwidthDiagnostics_MaxSpeed(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[20]={0};

	memset(buffer,0,sizeof(buffer));

	//sprintf(buffer,"%d",m_tr069_params.bandwidth_diagnostics_result.max_speed);
	sk_func_porting_params_get("tr069_bandwidth_maxspeed", buffer , sizeof(buffer));
    
	*value=buffer;
	*len=strlen(*value);
	return 0;
}

DLLEXPORT_API int sk_tr069_get_BandwidthDiagnostics_MinSpeed(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[20]={0};

	memset(buffer,0,sizeof(buffer));

	//sprintf(buffer,"%d",m_tr069_params.bandwidth_diagnostics_result.min_speed);
	sk_func_porting_params_get("tr069_bandwidth_minspeed", buffer , sizeof(buffer));
	*value=buffer;
	*len=strlen(*value);
	return 0;
}

DLLEXPORT_API int sk_tr069_get_BandwidthDiagnostics_AvgSpeed(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[20]={0};

	memset(buffer,0,sizeof(buffer));

	//sprintf(buffer,"%d",m_tr069_params.bandwidth_diagnostics_result.avg_speed);
	sk_func_porting_params_get("tr069_bandwidth_avgspeed", buffer , sizeof(buffer));
	*value=buffer;
	*len=strlen(*value);
	return 0;
}


int sk_tr069_bandwidth_diagnostics(struct evcpe *cpe)
{

    memset(&m_tr069_params.bandwidth_diagnostics_result , 0 ,sizeof(m_tr069_params.bandwidth_diagnostics_result));
    
	//sk_start_bandwidth_diagnostics(&m_tr069_params.bandwidth_diagnostics_param,&m_tr069_params.bandwidth_diagnostics_result);
     
    sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);

    evcpe_handle_del_event(cpe);
    
	cpe->event_flag = EVCPE_NO_BOOT;
	   
    //evcpe_start_session(cpe);
	
	evcpe_worker_start_session(cpe);
    
    return 0;
}




/*
*func:远程抓包
*
*/

DLLEXPORT_API int sk_tr069_set_PacketCapture_state(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buf[32] = {0};
	
	if ((buffer == NULL) || (len<=0))
	{
		return -1;
	}
	
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.packet_capture_param.state = atoi(buf);

    sk_func_porting_params_set("tr069_pcap_state" , buf);

	return 0;
}


DLLEXPORT_API int sk_tr069_get_PacketCapture_state(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));

    
    sk_func_porting_params_get("tr069_pcap_state" , buffer_local , sizeof(buffer_local));

	//snprintf(buffer_local,sizeof(buffer_local),"%d", m_tr069_params.packet_capture_param.state);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}


DLLEXPORT_API int sk_tr069_set_PacketCapture_IP(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buffer_local[64] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);
  	
	strncpy(m_tr069_params.packet_capture_param.ip_addr , buffer_local ,len_local);

    sk_func_porting_params_set("tr069_pcap_ip_addr" , buffer_local);
    
	return 0;
}


DLLEXPORT_API int sk_tr069_get_PacketCapture_IP(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
        
    strcpy(buffer , m_tr069_params.packet_capture_param.ip_addr);
    
	*value=buffer;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
    
	return 0;
}

DLLEXPORT_API int sk_tr069_set_PacketCapture_Port(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buf[32] = {0};
	
	if ((buffer == NULL) || (len<=0))
	{
		return -1;
	}
	
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.packet_capture_param.ip_port = atoi(buf);

    sk_func_porting_params_set("tr069_pcap_ip_port" , buf);

    return 0;
}


DLLEXPORT_API int sk_tr069_get_PacketCapture_Port(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));
	
	snprintf(buffer_local,sizeof(buffer_local),"%d", m_tr069_params.packet_capture_param.ip_port);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_PacketCapture_Duration(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buf[32] = {0};
	
	if ((buffer == NULL) || (len<=0))
	{
		return -1;
	}
	
	len=sizeof(buf)>len?len:sizeof(buf);
	
	strncpy(buf,buffer,len);
	
	m_tr069_params.packet_capture_param.duration= atoi(buf);

    sk_func_porting_params_set("tr069_pcap_duration" , buf);
   
	return 0;
}


DLLEXPORT_API int sk_tr069_get_PacketCapture_Duration(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
    static char buffer_local[32]={0};
	
	memset(buffer_local , 0 , sizeof(buffer_local));
	
	snprintf(buffer_local,sizeof(buffer_local),"%d", m_tr069_params.packet_capture_param.duration);
	
	*value=buffer_local;

	if(*value!=NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len=0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_PacketCapture_UploadURL(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buffer_local[256] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);
  	
	strncpy(m_tr069_params.packet_capture_param.upload_url, buffer_local ,len_local);

    sk_func_porting_params_set("tr069_pcap_uploadurl" , buffer_local);
     
	return 0;
}


DLLEXPORT_API int sk_tr069_set_PacketCapture_Username(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
    char buffer_local[32] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);
  	
	strncpy(m_tr069_params.packet_capture_param.username , buffer_local ,len_local);

    sk_func_porting_params_set("tr069_pcap_username" , buffer_local);
    
	return 0;
}

DLLEXPORT_API int sk_tr069_set_PacketCapture_Password(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{   
    char buffer_local[32] = {0};

    int  len_local = sizeof(buffer_local);

    if(NULL==buffer || len <= 0)
	{
		return -1;
	}

    len_local = len_local > len?len:len_local-1;

    memcpy(buffer_local , buffer , len_local);
  	
	strncpy(m_tr069_params.packet_capture_param.password , buffer_local ,len_local);

    sk_func_porting_params_set("tr069_pcap_password" , buffer_local);
    
	return 0;
}


DLLEXPORT_API int sk_tr069_get_address_notificationlimit(struct evcpe_attr *attr,const char **value, unsigned int *len)
{	
	static char buffer[64]={0};

    memset(buffer , 0 , sizeof(buffer));
       
	sk_func_porting_params_get("tr069_address_notificationlimit",buffer,sizeof(buffer));
    
    evcpe_info(__func__ , "tr069_address_notificationlimit =%s\n",buffer);

	*value = buffer;
    
	if(*value!=NULL)
	{
		*len = strlen(*value);
	}
	else
	{
		*len = 0;
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_set_address_notificationlimit(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{

	char buffer_local[64]={0};

	int  len_local=sizeof(buffer_local);
    
	if ((buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
 
	len_local=len_local>len?len:len_local-1;
	memcpy(buffer_local , buffer , len_local);

    evcpe_info(__func__ , "tr069_address_notificationlimit =%s \n",buffer_local);
	sk_func_porting_params_set("tr069_address_notificationlimit",buffer_local);
    return 0;
}

int sk_tr069_packet_capture(struct evcpe *cpe)
{
    sk_start_packet_capture_diagnostics(&m_tr069_params.packet_capture_param);

    /*
    sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);

    evcpe_handle_del_event(cpe);
    
	cpe->event_flag = EVCPE_NO_BOOT;
	   
    //evcpe_start_session(cpe);
	
	evcpe_worker_start_session(cpe);
    */
    return 0;
}

int  sk_tr069_syslog_upload(struct evcpe *cpe)
{
    sk_start_syslog_diagnostics(&m_tr069_params.syslog);

    /*
    sk_tr069_add_boot_status(E_TR069_DIAGNOSTICS_COMPLETE);

    evcpe_handle_del_event(cpe);
    
	cpe->event_flag = EVCPE_NO_BOOT;
	   
    //evcpe_start_session(cpe);
	
	evcpe_worker_start_session(cpe);
    */
    
    return 0;
}

int sk_tr069_get_pingresult(sk_ping_results_t* pPingResults , const char * pvalue)
{
	char szHost[64] = {0};				//应答ping诊断的主机地址；
	int nTotalCount = 0;				//在最近的ping测试中的总次数；
	int nSuccessCount = 0;				//在最近的ping测试中成功的次数
	int nFailureCount = 0;				//在最近的ping测试中失败的次数
	int nAverageResponseTime = 0;		//以毫秒为单位的最近一次ping测试所有成功响应的平均时间
	int nMinimumResponseTime = 0;		//以毫秒为单位的最近一次ping测试所有成功响应中的最短时间
	int nMaximumResponseTime = 0;		//以毫秒为单位的最近一次ping测试所有成功响应中的最长时间
	unsigned char *p_start = NULL;
	unsigned char *p_end = NULL;
	unsigned char *p_tmp = NULL;
	unsigned int len = 0;
	unsigned char buf[32];
	int i = 0;
	evcpe_info(__func__ , "sk_tr069_get_pingresult enter \n");
	if(NULL == pvalue)
	{
		goto LAB_ERR;
	}

	//主地址获取
	if(NULL != (p_tmp = strstr(pvalue , "PING")))
	{
		p_start = strchr(p_tmp , '(');
		p_end = strchr(p_start , ')');
		if(p_end > (p_start+1))
		{
			len = p_end- (p_start+1);
			if(len > sizeof(szHost))
			{
				len = sizeof(szHost)-1;
			}
			strncpy(szHost , p_start+1 , len);
			szHost[len] = '\0';
			evcpe_info(__func__ , "sk_tr069_get_pingresult recv szHost :%s\n",szHost);
		}
	}

	//获取ping 总次数
	if(NULL != (p_end = strstr(pvalue , "packets transmitted")))
	{
		p_start = p_end;
		while(*p_start != '-') --p_start;
		++p_start;
		while(*p_start == ' ') ++p_start;
		i = 0;
		while(p_start < p_end && *p_start != ' ') {
			buf[i++] = *p_start;
			p_start++;
		}
		buf[i] = '\0';
		nTotalCount = atoi(buf);
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nTotalCount :%d\n",nTotalCount);
		
		p_tmp  = p_end ;
		if(NULL != (p_end = strstr(p_tmp , "received")))
		{
			p_start = p_end;
			while(*p_start != ',') --p_start;
			++p_start;
			while(*p_start == ' ') ++p_start;	
			i = 0;
			while(p_start < p_end && *p_start != ' '){
				buf[i++] = *p_start;
				p_start++;
			}
			buf[i] = '\0';
			nSuccessCount = atoi(buf);
			evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nTotalCount :%d\n",nSuccessCount);
		}
		nFailureCount = nTotalCount - nSuccessCount;
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nFailureCount :%d\n",nFailureCount);
	}

	if(NULL != (p_tmp = strstr(pvalue , "min/avg/max")) && NULL != (p_start = strstr(p_tmp , "=")))
	{
        //获取最小响应时间
		++p_start;
		while(*p_start == ' ') ++p_start;
		i = 0;
		while(*p_start != '/') {
			buf[i++] = *p_start;
			p_start++;
		}
		buf[i] = '\0';
		nMinimumResponseTime = atoi(buf);
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nMinimumResponseTime :%d\n",nMinimumResponseTime);

        //获取平均响应时间
		++p_start;
		i = 0;
		while(*p_start != '/') {
			buf[i++] = *p_start;
			p_start++;
		}
		buf[i] = '\0';
		nAverageResponseTime = atoi(buf);
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nAverageResponseTime :%d\n",nAverageResponseTime);

        //获取最大响应时间
		++p_start;
		i = 0;
		while(*p_start != '/') {
			buf[i++] = *p_start;
			p_start++;
		}
		buf[i] = '\0';
		nMaximumResponseTime = atoi(buf);
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nMaximumResponseTime :%d\n",nMaximumResponseTime);

	}
	else
	{
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nMinimumResponseTime :%d\n",nMinimumResponseTime);
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nAverageResponseTime :%d\n",nAverageResponseTime);
		evcpe_info(__func__ , "sk_tr069_get_pingresult recv packets nMaximumResponseTime :%d\n",nMaximumResponseTime);
	}

    strcpy(pPingResults->szHost , szHost);
    pPingResults->nTotalCount = nTotalCount;
    pPingResults->nSuccessCount = nSuccessCount;
    pPingResults->nFailureCount = nFailureCount;
    pPingResults->nAverageResponseTime = nAverageResponseTime;
    pPingResults->nMinimumResponseTime = nMinimumResponseTime;
    pPingResults->nMaximumResponseTime = nMaximumResponseTime;
    strcpy(pPingResults->nStatus , "Complete");
	evcpe_info(__func__ , "sk_tr069_get_pingresult ok exit\n");
	return 0;

LAB_ERR:
    strcpy(pPingResults->nStatus , "Error_Other");
	evcpe_info(__func__ , "sk_tr069_get_pingresult error exit\n");
	return -1;
}



/*****************************************************************************

typedef struct traceroute_results_s
{
	char szHost[16];				//用于路由诊断的主机地址
	int nResponseTime;				//以毫秒表示的最近一次路由主机测试的响应时间，如果无法决定具体路由，则默认为0
	int nNumberOfRouteHops;			//用于发现路由的跳数，如果无法决定路由，则默认为0
	hop_host_t gHopHost[MAX_TTL];	//路由路径
}sk_traceroute_results_t;


*****************************************************************************/

int sk_tr069_get_tracerouteresult(sk_traceroute_results_t* pTracerouteResults , const char * pvalue)
{
    char szHost[64] = {0};				
	int nMaxHopCount = 0;			
    unsigned char *p_start = NULL;
	unsigned char *p_end = NULL;
	unsigned char *p_tmp = NULL;
	unsigned int len = 0;

    char seps[]= "\n";
    char *token = NULL;

    if(NULL == pvalue)
    {
        goto LAB_ERR;
    }
	
    token = strtok(pvalue , seps);
	if(NULL != token)
	{
        //LOGI( " %s\n", token );
		p_start = strchr(token , '(');
		p_end = strchr(p_start , ')');
		if(p_end > (p_start+1))
		{
			len = p_end- (p_start+1);
			if(len > sizeof(szHost))
			{
				len = sizeof(szHost)-1;
			}
			strncpy(szHost , p_start+1 , len);
			szHost[len] = '\0';
			strcpy(pTracerouteResults->szHost , szHost);
			LOGI("sk_tr069_get_tracerouteresult recv szHost :%s\n",pTracerouteResults->szHost);
		}
	}
    while(token != NULL)
    {
		token = strtok(NULL, seps );
		if(NULL != token)
		{
            //LOGI( " %s\n", token );
			if(NULL != strchr(token , '*'))
			{
					pTracerouteResults->gHopHost[nMaxHopCount].flag = 0;
			}
			else
			{
				p_start = strchr(token , '(');
				p_end = strchr(p_start , ')');
				if(p_end > (p_start+1))
				{
					float f1 = 0 , f2 = 0 , f3 = 0;
					len = p_end- (p_start+1);
					if(len > sizeof(szHost))
					{
						len = sizeof(szHost)-1;
					}
					strncpy(szHost , p_start+1 , len);
					szHost[len] = '\0';
					strcpy(pTracerouteResults->gHopHost[nMaxHopCount].szHost , szHost);
					pTracerouteResults->gHopHost[nMaxHopCount].flag = 1;
					//LOGI("No %d  , recv szHost :%s\n", nMaxHopCount , pTracerouteResults->gHopHost[nMaxHopCount].szHost);
					p_start = p_end +1;
					while(*p_start == ' ') p_start++;
					sscanf(p_start , "%f ms  %f ms  %f ms",&f1,&f2,&f3);
					//LOGI("tm = %f , %f  %f \n", f1 ,f2 , f3 );
                    pTracerouteResults->gHopHost[nMaxHopCount].nTTL = (f1+f2+f3)/3;
				}
			}
            nMaxHopCount++;
		}

    }
    pTracerouteResults->nNumberOfRouteHops  = nMaxHopCount;
    strcpy(pTracerouteResults->nStatus , "Complete");
    return 0;
LAB_ERR:
    strcpy(pTracerouteResults->nStatus , "Error_Other");
    return -1;
}


int sk_set_reauth_event_code()
{
	static char buffer[32]={0};
    memset(buffer , 0 , sizeof(buffer));

    sk_func_porting_params_get("tr069_event",buffer,sizeof(buffer));
	memset(reauth_event_code,0, sizeof(reauth_event_code));
	strcpy(reauth_event_code,buffer);
	evcpe_info(__func__ , "sk_set_reauth_event_code event :%s\n",reauth_event_code);
	return 0;
}

int sk_reauth_reset_event_code()
{
	if(reauth_event_code[0]!='\0')
	{
		sk_func_porting_params_set("tr069_event",reauth_event_code);
		evcpe_info(__func__ , "\sk_reauth_reset_event_code event :%s\n",reauth_event_code);
	}
	return 0;
}

DLLEXPORT_API int sk_tr069_get_urlchange(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[32] = {0};

    memset(local_buffer , 0 , sizeof(local_buffer));
    
	sk_func_porting_params_get("tr069_url_modify_flag",local_buffer,sizeof(local_buffer));

	*value = local_buffer;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_urlchange(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[32] = {0};
	int  length = sizeof(local_buffer);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	length = length>len?len:length-1;

	strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "sk_tr069_set_urlchange _buffer=%s,length=%d",local_buffer,length);

    sk_func_porting_params_set("tr069_url_modify_flag" , local_buffer);
    
	return 0;
}

DLLEXPORT_API int sk_tr069_get_app_auto_run_blacklist_flag(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[32] = {0};

    memset(local_buffer , 0 , sizeof(local_buffer));
    
	sk_func_porting_params_get("app_auto_run_blacklist_flag",local_buffer,sizeof(local_buffer));

	*value = local_buffer;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_app_auto_run_blacklist_flag(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[32] = {0};
	int  length = sizeof(local_buffer);
	
	if ( (buffer==NULL) || 	(len<=0) )
	{
		return -1;
	}
	length = length>len?len:length-1;

	strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "sk_tr069_set_app_auto_run_blacklist_flag _buffer=%s,length=%d",local_buffer,length);

    sk_func_porting_params_set("app_auto_run_blacklist_flag" , local_buffer);
    
	return 0;
}

DLLEXPORT_API int sk_tr069_get_app_auto_run_blacklist_nums(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[32] = {0};

    memset(local_buffer , 0 , sizeof(local_buffer));
    
	sk_func_porting_params_get("app_auto_run_blacklist_nums",local_buffer,sizeof(local_buffer));

	*value = local_buffer;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}


DLLEXPORT_API int sk_tr069_get_app_auto_run_blacklist_app_packagename(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[11][128] = {0};
    int index = 0;
    unsigned char param_name[64] = {0};
    if(NULL != attr)
    {   
       
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.%d.PackageName" ,&index);
            if(index > 0 && index <=10 )
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_packagename%d",index);
                memset(local_buffer[index] , 0 , sizeof(local_buffer[index]));
                sk_func_porting_params_get(param_name , local_buffer[index],sizeof(local_buffer[index]));
            }
        }
    } 
	*value = local_buffer[index];
	if(*value != NULL)
	{
		if(!strcmp(local_buffer[index],"AddObject")|| !strcmp(local_buffer[index],"DeleteObject"))
        {
            *len = 0;
        }
        else
        {
		    *len=strlen(*value);
	    }
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_app_auto_run_blacklist_app_packagename(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[128] = {0};
    
	int  length = sizeof(local_buffer);
	
	if((buffer==NULL)||(len <= 0))
	{
		//return -1;
		return 0;
	}
    
	length = length>len?len:length-1;
    
    strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "local_buffer=%s,length=%d",local_buffer,length);
    
    if(NULL != attr)
    {   
        int index = -1;
        unsigned char param_name[64] = {0};
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.%d.PackageName" ,&index);
            if(index > 0)
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_packagename%d",index);
                sk_func_porting_params_set(param_name , local_buffer);
            }
        }
    } 
	return 0;
}

DLLEXPORT_API int sk_tr069_get_app_auto_run_blacklist_app_classname(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[11][128] = {0};
    int index = 0;
    unsigned char param_name[64] = {0};
    if(NULL != attr)
    {   
       
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.%d.ClassName" ,&index);
            if(index > 0 && index <=10 )
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_classname%d",index);
                memset(local_buffer[index] , 0 , sizeof(local_buffer[index]));
                sk_func_porting_params_get(param_name , local_buffer[index],sizeof(local_buffer[index]));
            }
        }
    } 
	*value = local_buffer[index];
	if(*value != NULL)
	{
		if(!strcmp(local_buffer[index],"AddObject")|| !strcmp(local_buffer[index],"DeleteObject"))
        {
            *len = 0;
        }
        else
        {
		    *len=strlen(*value);
	    }
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_app_auto_run_blacklist_app_classname(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[128] = {0};
    
	int  length = sizeof(local_buffer);
	
	if((buffer==NULL)||(len <= 0))
	{
		//return -1;
		return 0;
	}
    
	length = length>len?len:length-1;
    
    strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "local_buffer=%s,length=%d",local_buffer,length);
    
    if(NULL != attr)
    {   
        int index = -1;
        unsigned char param_name[64] = {0};
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.%d.ClassName" ,&index);
            if(index > 0)
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_classname%d",index);
                sk_func_porting_params_set(param_name , local_buffer);
            }
        }
    } 
	return 0;
}


DLLEXPORT_API int sk_tr069_get_usb_permit_installed_app_nums(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[32] = {0};

    memset(local_buffer , 0 , sizeof(local_buffer));
    
	sk_func_porting_params_get("usb_permit_installed_app_nums",local_buffer,sizeof(local_buffer));

	*value = local_buffer;
	if(*value != NULL)
	{
		*len=strlen(*value);
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}


DLLEXPORT_API int sk_tr069_get_usb_permit_installed_app_packagename(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[11][128] = {0};
    int index = 0;
    unsigned char param_name[64] = {0};
    if(NULL != attr)
    {   
       
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.%d.PackageName" ,&index);
            if(index > 0 && index <=10 )
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_packagename%d",index);
                memset(local_buffer[index] , 0 , sizeof(local_buffer[index]));
                sk_func_porting_params_get(param_name , local_buffer[index],sizeof(local_buffer[index]));
            }
        }
    } 
	*value = local_buffer[index];
	if(*value != NULL)
	{
        if(!strcmp(local_buffer[index],"AddObject")|| !strcmp(local_buffer[index],"DeleteObject"))
        {
            *len = 0;
        }
        else
        {
		    *len=strlen(*value);
	    }
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_usb_permit_installed_app_packagename(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[128] = {0};
    
	int  length = sizeof(local_buffer);
	
	if((buffer==NULL)||(len <= 0))
	{
		//return -1;
        return 0;
    }
    
	length = length>len?len:length-1;
    
    strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "local_buffer=%s,length=%d",local_buffer,length);
    
    if(NULL != attr)
    {   
        int index = -1;
        unsigned char param_name[64] = {0};
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.%d.PackageName" ,&index);
            if(index > 0)
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_packagename%d",index);
                sk_func_porting_params_set(param_name , local_buffer);
            }
        }
    } 
	return 0;
}

DLLEXPORT_API int sk_tr069_get_usb_permit_installed_app_classname(struct evcpe_attr *attr,const char **value, unsigned int *len)
{
	static char local_buffer[11][128] = {0};
    int index = 0;
    unsigned char param_name[64] = {0};
    if(NULL != attr)
    {   
       
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.%d.ClassName" ,&index);
            if(index > 0 && index <=10 )
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_classname%d",index);
                memset(local_buffer[index] , 0 , sizeof(local_buffer[index]));
                sk_func_porting_params_get(param_name , local_buffer[index],sizeof(local_buffer[index]));
            }
        }
    } 
	*value = local_buffer[index];
	if(*value != NULL)
	{
        if(!strcmp(local_buffer[index],"AddObject")|| !strcmp(local_buffer[index],"DeleteObject"))
        {
            *len = 0;
        }
        else
        {
		    *len=strlen(*value);
	    }
	}
	else
	{
		*len = 0;
	}
	
	return 0;
}

DLLEXPORT_API int sk_tr069_set_usb_permit_installed_app_classname(struct evcpe_attr *attr,const char *buffer, unsigned int len)
{
	char local_buffer[128] = {0};
    
	int  length = sizeof(local_buffer);
	
	if((buffer == NULL)||(len <= 0))
	{
		//return -1;
        return 0;
    }
    
	length = length>len?len:length-1;
    
    strncpy(local_buffer,buffer,length);
	
	evcpe_info(__func__ , "local_buffer=%s,length=%d",local_buffer,length);
    
    if(NULL != attr)
    {   
        int index = -1;
        unsigned char param_name[64] = {0};
        evcpe_info(__func__ , "path = %.*s",attr->pathlen ,attr->path);
        if(strstr(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList."))
        {
            sscanf(attr->path , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.%d.ClassName" ,&index);
            if(index > 0)
            {
                snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_classname%d",index);
                sk_func_porting_params_set(param_name , local_buffer);
            }
        }
    } 
	return 0;
}

//add by lijingchao
//comment:针对江苏移动需求
/*
Device.X_CMCC_OTV.Extention. (object)
|_AppAutoRunBlackListFlag   boolean  RW 默认True
|_NumOfAppAutoRunBlackList  Int(32)　R 默认0（由机顶盒根据下挂节点数量进行自动及时更新、只读）
|_AppAutoRunBlackList.{i}. object   RW 
  |_PackageName   string     RW
  |_ClassName    string     RW
      
Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP object RW
|_NumofAPP  Int(32)　R 默认0（由机顶盒根据下挂节点数量进行自动及时更新，只读）
|_UsbInstalledAppList.{i}.  object  
  |_PackageName  string(256)
  |_ClassName    string(256)     
*/
int sk_tr069_add_multiple_object_data(struct evcpe_attr *attr, char*object_name , int object_index)
{
    int index = object_index;
    unsigned char param_name[64] = {0};
    char local_buffer[128] = {0};   

    evcpe_info(__func__ , "[lijc for debug]object_name=%s,object_index =%d" , object_name,object_index);

    if(NULL != object_name)
    {
        if(strstr(object_name , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList."))
        {
            strcpy(local_buffer , "AddObject");
            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_packagename%d",index);
            sk_func_porting_params_set(param_name , local_buffer);


            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_classname%d",index);
            sk_func_porting_params_set(param_name , local_buffer);
        }
        else if(strstr(object_name , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList."))
        {
            strcpy(local_buffer , "AddObject");
            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_packagename%d",index);
            sk_func_porting_params_set(param_name , local_buffer);


            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_classname%d",index);
            sk_func_porting_params_set(param_name , local_buffer);
        }
    }
  
    return 0;
}



int sk_tr069_delete_multiple_object_data(struct evcpe_attr *attr, char*object_name)
{
    int index = 0;
    unsigned char param_name[64] = {0};
    char local_buffer[128] = {0};   

    evcpe_info(__func__ , "[lijc for debug]object_name=%s",object_name);

    if(NULL != object_name)
    {
        if(strstr(object_name , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList."))
        {
            sscanf(object_name , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.%d.",&index);
            strcpy(local_buffer , "DeleteObject");
            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_packagename%d",index);
            sk_func_porting_params_set(param_name , local_buffer);


            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_classname%d",index);
            sk_func_porting_params_set(param_name , local_buffer);
        }
        else if(strstr(object_name , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList."))
        {
            sscanf(object_name , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.%d.",&index);         
            strcpy(local_buffer , "DeleteObject");
            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_packagename%d",index);
            sk_func_porting_params_set(param_name , local_buffer);


            memset(param_name , 0  , sizeof(param_name));
            snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_classname%d",index);
            sk_func_porting_params_set(param_name , local_buffer);
        }
    }
    return 0;
}

int sk_tr069_preload_multipleObject_UsbInstalledAppList_construct(struct evcpe_repo *repo ,struct evcpe_obj *obj)
{
    int rc=0;
	unsigned int index;
    unsigned char tmp_buffer[128] = {0};
    unsigned char object_name[128] = {0};
    unsigned char param_name[64] = {0};
    int i = 0;
    int count = 0;
    int usb_app_nums = 0;
    int auto_app_nums = 0;

    memset(tmp_buffer , 0 , sizeof(tmp_buffer));
	sk_func_porting_params_get("usb_permit_installed_app_nums",tmp_buffer,sizeof(tmp_buffer));

    usb_app_nums = strtol(tmp_buffer , NULL , 10);
    evcpe_info(__func__ , "usb_permit_installed_app_nums = %d", usb_app_nums);

    i = 1 ;
    count = 1;
    while(usb_app_nums> 0 && count++ <= usb_app_nums && i++ <= 10)
    { 
       
        memset(object_name , 0 , sizeof(object_name));
        strcpy(object_name , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.");
        if ((rc = evcpe_repo_add_obj(repo, object_name , &index)))
    	{
    		evcpe_error(__func__, "failed to add object: %s", object_name);
    		break;
    	}
        
        memset(param_name , 0 , sizeof(param_name));
        snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_packagename%d", i);
       
        memset(tmp_buffer , 0 , sizeof(tmp_buffer));
        sk_func_porting_params_get(param_name , tmp_buffer ,sizeof(tmp_buffer));

        if(!strcmp(tmp_buffer , "DeleteObject"))
        {
            count--;    
        }
    }
    count = i - 1;
    for(i = 1 ; i < count ; i++)
    {

        memset(param_name , 0 , sizeof(param_name));
        snprintf(param_name ,sizeof(param_name)-1 ,"usb_install_packagename%d", i);
       
        memset(tmp_buffer , 0 , sizeof(tmp_buffer));
        sk_func_porting_params_get(param_name , tmp_buffer ,sizeof(tmp_buffer));

        if(!strcmp(tmp_buffer , "DeleteObject"))
        {
            memset(object_name , 0 , sizeof(object_name));
            snprintf(object_name , sizeof(object_name)-1 , "Device.X_CMCC_OTV.ServiceInfo.USBPermitInstalledAPP.UsbInstalledAppList.%d.",i);
            if ((rc = evcpe_repo_del_obj(repo, object_name)))
        	{
        		evcpe_error(__func__, "failed to delete object: %s", object_name);
        		break;
        	} 
        }
        
    }
    
 finally:       
    return rc;
}

int sk_tr069_preload_multipleObject_AppAutoRunBlackList_construct(struct evcpe_repo *repo ,struct evcpe_obj *obj)
{
    int rc=0;
	unsigned int index;
    unsigned char tmp_buffer[128] = {0};
    unsigned char object_name[128] = {0};
    unsigned char param_name[64] = {0};
    int i = 0;
    int count = 0;
    int auto_app_nums = 0;

    memset(tmp_buffer , 0 , sizeof(tmp_buffer));
	sk_func_porting_params_get("app_auto_run_blacklist_nums",tmp_buffer,sizeof(tmp_buffer));

    auto_app_nums = strtol(tmp_buffer , NULL , 10);
    evcpe_info(__func__ , "app_auto_run_blacklist_nums = %d", auto_app_nums);

    i = 1 ;
    count = 1;
    while(auto_app_nums> 0 && count++ <= auto_app_nums && i++ <= 10)
    { 
       
        memset(object_name , 0 , sizeof(object_name));
        strcpy(object_name , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.");
        if ((rc = evcpe_repo_add_obj(repo, object_name , &index)))
    	{
    		evcpe_error(__func__, "failed to add object: %s", object_name);
    		break;
    	}
        
        memset(param_name , 0 , sizeof(param_name));
        snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_packagename%d", i);
       
        memset(tmp_buffer , 0 , sizeof(tmp_buffer));
        sk_func_porting_params_get(param_name , tmp_buffer ,sizeof(tmp_buffer));

        if(!strcmp(tmp_buffer , "DeleteObject"))
        {
            count--;    
        }
    }
    count = i - 1;
    for(i = 1 ; i < count ; i++)
    {

        memset(param_name , 0 , sizeof(param_name));
        snprintf(param_name ,sizeof(param_name)-1 ,"autorun_app_packagename%d", i);
       
        memset(tmp_buffer , 0 , sizeof(tmp_buffer));
        sk_func_porting_params_get(param_name , tmp_buffer ,sizeof(tmp_buffer));

        if(!strcmp(tmp_buffer , "DeleteObject"))
        {
            memset(object_name , 0 , sizeof(object_name));
            snprintf(object_name , sizeof(object_name)-1 , "Device.X_CMCC_OTV.Extention.AppAutoRunBlackList.%d.",i);
            if ((rc = evcpe_repo_del_obj(repo, object_name)))
        	{
        		evcpe_error(__func__, "failed to delete object: %s", object_name);
        		break;
        	} 
        }
        
    }
    
 finally:       
    return rc;
}
//add end

//add by lijingchao
//检测IP字符串的合法性
//return->	-1:invalid;	0:valid
//标准输入为"192.168.28.61"
int sk_network_check_ip(const char* IPString)
{
	int iLen=0,idotNum=0,iNumber=0,i=0;
	char arrayIP[32]={0};
	char arrayNumber[4]={0};
	char* pBegin=NULL,*pEnd=NULL;
	char cSymbol=0;
	
	if (NULL == IPString)
	{
		LOGI("[ERROR]ip is NULL!!\n");
		goto ERROR_EXIT;
	}

    if(strstr(IPString , ":"))
    {
        //如果是ipv6地址的话，目前暂时不需要检查，直接返回ip地址有效
        goto OK_EXIT;
    }
	
	iLen = strlen(IPString);
	if (iLen<=0 || iLen>15) 
	{
		LOGI("[ERROR]ip length is wrong!!!\n");
		goto ERROR_EXIT;
	}
	
	strcpy(arrayIP,IPString);
	for (i=0;i<iLen;i++)
	{
		cSymbol = arrayIP[i];
		if (cSymbol<'.' || cSymbol>'9')
		{
			LOGI("[ERROR]ip isn't numeric!!!\n");
			goto ERROR_EXIT;
		}
	}
	
	pBegin = (char *)arrayIP;
	pEnd = (char *)strstr((char *)pBegin,".");

	while( pEnd != NULL )
	{
		idotNum++;
		strncpy(arrayNumber,pBegin,pEnd-pBegin);
		iNumber = atoi(arrayNumber);
        
		if (iNumber>255)
		{
			LOGI("[ERROR]ip bigger than 255!!!\n");
		    goto ERROR_EXIT;
		}
        
		memset(arrayNumber,0,4);
		pBegin=pEnd+1;
		pEnd = (char *)strstr((char *)pBegin,".");
	}
	if (idotNum!=3) 
	{
		LOGI("[ERROR]ip dot wrong!!!\n");
		goto ERROR_EXIT;
	}
	//检验最后一段
	strncpy(arrayNumber,pBegin,3);
	iNumber = atoi(arrayNumber);
	if (iNumber>255)
	{
		LOGI("[ERROR]ip bigger than 255!!!\n");
		goto ERROR_EXIT;
	}
    
OK_EXIT:
	return 0;
ERROR_EXIT:
    return -1;
}




int sk_get_nat_stun_flag(void)
{
	return g_nat_stun_flag;
}

int  sk_set_nat_stun_flag(int nat_stun_flag)
{
    LOGI("[accessor][sk_set_nat_stun_flag] nat_stun_flag = %d \n",nat_stun_flag);
    g_nat_stun_flag  = nat_stun_flag;
	return 0;
}
 

int sk_get_value_reset_acs_url_flag()
{
    return reset_acs_url_flag;
}

void sk_set_value_reset_acs_url_flag(int flag)
{
    if(reset_acs_url_flag != flag)
    {
        evcpe_info(__func__ , "sk_set_value_reset_acs_url_flag  flag = %d\n",flag);
        printf("sk_set_value_reset_acs_url_flag  flag = %d\n",flag);
    }
    reset_acs_url_flag = flag;
}

//add by lijingchao
int sk_tr069_start_reauth(struct evcpe * cpe)
{
	int rc = 0 ;
    char *value = NULL;
    int len = 0;
    
    unsigned char tr069_acs_buf[512] = {0};
    unsigned char tr069_bak_acs_buf[512] = {0};
    #if 1
    unsigned char *buffer = sk_tr069_get_reauth_tr069_acs();
    if(strlen(buffer) > 0 )
    {
        sk_set_acs_dns(cpe , buffer);
    }
    
    if(0 == sk_get_value_reset_acs_url_flag())
    {
        sk_set_value_reset_acs_url_flag(1); 
    }
	sk_tr069_get_boot_status(&value,&len);
    
    sk_func_porting_params_get("tr069_acs" ,tr069_acs_buf , sizeof(tr069_acs_buf));
    sk_func_porting_params_get("tr069_bak_acs" ,tr069_bak_acs_buf , sizeof(tr069_bak_acs_buf));

   

    
    if('\0' != tr069_bak_acs_buf[0] && (0!=strcmp(tr069_acs_buf , tr069_bak_acs_buf)))
    {
         sk_tr069_add_boot_status(E_TR069_BOOTSTRAP);
         cpe->event_flag = EVCPE_NO_BOOT;
    }
    else
    {
        if(NULL != value && ('\0' == value[0] || !strcmp(value , "none") || NULL != strchr(value , '0')))
        {
            cpe->event_flag = EVCPE_NO_BOOT;
        }
        else
        {
            cpe->event_flag = EVCPE_BOOTING;
        }
    }
                   
   
    evcpe_handle_del_event(cpe);
  
	rc = evcpe_start_session(cpe);		
        
    #else
    sk_reauth_reset_event_code();
	rc = evcpe_start_session(cpe);							
    #endif
	return rc;
}


int sk_tr069_start_awaken(struct evcpe * cpe)
{
	int rc = 0 ;
    char *value = NULL;
    int len = 0;
    sk_tr069_get_acsurl(NULL ,&value , &len);
    
    if(strlen(value)> 0 )
    {
        sk_set_acs_dns(cpe , value);
    }
    
    sk_set_value_reset_acs_url_flag(1); 
   
    evcpe_handle_del_event(cpe);
    
    value = NULL;
    len = 0;
    sk_tr069_get_boot_status(&value , &len);

    //如果上一次是0 BOOT事件，就不应该添加1 BOOT。
    if(NULL != value && ('\0' == value[0] || !strcmp(value , "none") || NULL != strchr(value , '0')))
    {
        cpe->event_flag = EVCPE_NO_BOOT;
    }
    else
    {
        cpe->event_flag = EVCPE_BOOTING;
    }
    
	rc = evcpe_start_session(cpe);		

    return rc;
}

int sk_tr069_start_6_connection_request(struct evcpe * cpe)
{
    int rc = 0;
    evcpe_handle_del_event(cpe);
	cpe->event_flag=EVCPE_NO_BOOT;
    
    evcpe_info(__func__ , "6 CONNECTION REQUEST begin...");
	rc = evcpe_repo_add_event(cpe->repo, "6 CONNECTION REQUEST", "");     
	if (!cpe->session && (rc = evcpe_start_session(cpe))) 
	{
		evcpe_info(__func__ , "failed to start session 6 CONNECTION REQUEST");
		goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    return -1;
}

int sk_tr069_start_4_value_change(struct evcpe * cpe)
{
    int rc = 0;
    evcpe_handle_del_event(cpe);
	cpe->event_flag=EVCPE_NO_BOOT;
    
    evcpe_info(__func__ , "4 VALUE CHANGE  begin...");
	rc = evcpe_repo_add_event(cpe->repo, "4 VALUE CHANGE", "");     
	if (!cpe->session && (rc = evcpe_start_session(cpe))) 
	{
		evcpe_info(__func__ , "failed to start session 4 VALUE CHANGE");
		goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    return -1;
}

int sk_tr069_start_shutdown(struct evcpe * cpe)
{
    int rc = 0;
    evcpe_handle_del_event(cpe);
	cpe->event_flag=EVCPE_NO_BOOT;
    
    evcpe_info(__func__ , "X CMCC Shutdown begin...");
	rc = evcpe_repo_add_event(cpe->repo, "X CMCC Shutdown", "");     
	if (!cpe->session && (rc = evcpe_start_session(cpe))) 
	{
		evcpe_info(__func__ , "failed to start session X CMCC Shutdown");
		goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    return -1;
}


int sk_tr069_start_2_periodic(struct evcpe * cpe)
{
    int rc = 0;
    evcpe_handle_del_event(cpe);
    cpe->event_flag = E_TR069_PERIODIC;
     evcpe_info(__func__ , "2 PERIODIC begin...");
	if (!cpe->session && (rc = evcpe_start_session(cpe))) 
	{
		evcpe_info(__func__ , "evcpe_start_session_cb failed to start session");
        goto LAB_ERR;
	}
    return 0;
    
LAB_ERR:
    return -1;
}


//Date:2014-04-14
//江苏移动项目，需要判断一下升级异常情况下，是否需要发送Transportation Complete事件
int sk_tr069_check_upgrade_successful_or_not(void)
{

    char upgrade_flag[32]={0};  
    sk_api_params_get("tr069_upgrade_flag", upgrade_flag , sizeof(upgrade_flag));
    if(!strcmp(upgrade_flag , "1"))
    {
       char system_build_time[128] = {0};
       char bak_system_build_time[128] = {0};
       
       sk_func_porting_params_get("tr069_system_build_time" , system_build_time , sizeof(system_build_time));
       sk_func_porting_params_get("tr069_bak_system_build_time" , bak_system_build_time , sizeof(bak_system_build_time));
       if(strcmp(system_build_time , bak_system_build_time))
       {
           sk_tr069_add_boot_status(E_TR069_TRANSFER_COMPLETE);
           sk_api_params_set("tr069_upgrade_flag","0");
           sk_func_porting_params_set("tr069_bak_system_build_time" , system_build_time );
           evcpe_info(__func__ , "[cpe] oh yeal ! upgrade OK !\n");
           return 0;
       }
       else
       {
            evcpe_info(__func__ , "[cpe] last upgrade failed! fuck you!\n");
       }
    }
  
    return -1;   
}



//add end

	
