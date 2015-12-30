// $Id: evcpe.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#include <signal.h>

#ifndef WIN32
#include <mcheck.h>
#include <getopt.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <evcpe.h>
#include <evdns.h>

#include <cutils/properties.h>
#include "evcpe-config.h"
#include "accessor.h"


evcpe_t this = {0};

#ifdef WIN32
//用于本地调试打印
static FILE *fp_debug = NULL;
#endif//WIN32
/*
标注 网管是否启动 状态 
0:表示网管没有启动
1:表示网管启动
*/
static int m_init_ok = 0;


static int m_shutdown_flag = 0;


//特殊处理:目前发现amLogic芯片，频繁创建线程和销毁线程会引起死机。
static int g_call_shutdown = 0;


static void sig_handler(int signum);

static void error_cb(struct evcpe *cpe,
					 enum evcpe_error_type type, int code, const char *reason, void *cbarg);



static int sk_tr069_get_shutdown_flag();
static void sk_tr069_set_shutdown_flag(int status);

static char *sk_tr069_get_buildtime(void);

static int _shutdown(int code);


extern int sk_tr069_get_call_shutdown_flag();

extern void sk_tr069_set_call_shutdown_flag(int flag);


static int _shutdown(int code)
{
	evcpe_info(__func__ ,"shuting down  start \n");
	evcpe_info(__func__, "shuting down with code: %d (%s)", code, strerror(code));
	event_base_loopbreak(this.evbase);
	evcpe_free(this.cpe);
	evdns_shutdown(0);
	evcpe_persister_free(this.persist);
	event_base_free(this.evbase);
	evcpe_repo_free(this.repo);
	evcpe_obj_free(this.obj);
	evcpe_class_free(this.cls);

	//muntrace();
	evcpe_info(__func__ , "shuting down  exit \n");
	return 0;
}

evcpe_t *get_evcpe_object()
{
	return &this;
}

int load_file(const char *filename, struct evbuffer *buffer)
{
	FILE *file;
	int rc, fd, len;
   	evcpe_info(__func__ , "filename:%s\n",filename);
	if (!(file = fopen(filename, "r")))
	{
		rc = errno;
		goto finally;
	}
	fd = fileno(file);
	evbuffer_drain(buffer, EVBUFFER_LENGTH(buffer));
	do
	{
        #ifdef WIN32
        len = debug_evbuffer_read(buffer, fd, -1);
        #else
		len = evbuffer_read(buffer, fd, -1);
        #endif
    }
	while (len > 0);

	rc = len < 0 ? -1 : 0;
	fclose(file);

finally:
		return rc;
}

int load_data_to_buffer(struct evbuffer *buffer)
{

	int 	len = 0 ;
	char	*value = NULL;
	
	evbuffer_drain(buffer, EVBUFFER_LENGTH(buffer));
    len = EVBUFFER_LENGTH(buffer);
	
	sk_tr069_get_boot_status(&value,&len);
	if(value==NULL)
	{
		return 0;
	}

	if(strlen(value)<=0)
	{
		strcpy(value,"none");
	}
    
	evcpe_info(__func__ , "[load_data_to_buffer] tr069_boot=%s\n",value);
		

	if(!strcmp(value,"none"))
	{
		if (evcpe_add_buffer(buffer, "<?xml version=\"1.0\"?><Device>\
		<Event><EventCode><Value>0 BOOTSTRAP</Value></EventCode>\
		<CommandKey><Value></Value></CommandKey></Event>\
			</Device>"))
		{
			evcpe_error(__func__, "failed to load data!");
			return -1;
		}
	}
	else
	{
		if (evcpe_add_buffer(buffer, "<?xml version=\"1.0\"?><Device>\
		<Event><EventCode><Value>1 BOOT</Value></EventCode>\
		<CommandKey><Value></Value></CommandKey></Event>\
			</Device>"))
		{
			evcpe_error(__func__, "failed to load data!");
			return -1;
		}
	}

	return 0;

}


void sig_handler(int signal)
{
    
	evcpe_info(__func__, "signal caught: %d", signal);
	if (signal == SIGINT)
	{
        printf("you input Ctrl + C !\n");
        _shutdown(0);
	}
}


void error_cb(struct evcpe *cpe,
			  enum evcpe_error_type type, int code, const char *reason, void *cbarg)
{
	evcpe_error(__func__, "type: %d, code: %d, reason: %s", type, code, reason);
	_shutdown(code);
}

static int _syslog_start_and_stop(sk_tr069_params_t *param)
{
	static int			start_syslog=0;   //syslog还没开始

	//printf("_syslog_start_and_stop enter!\n");
	if(param==NULL)
	{
		return 0;
	}
    
    if(param->syslog.put_type > 0)
	{
		int now;
		int start_time;
		int end_time;
		
		now=sk_tr069_gettickcount();
		
		start_time=atoi(param->syslog.upload_starttime);
		end_time=start_time+param->syslog.continue_time*60;
		
		//printf("start_time=%d,now=%d,end_time=%d\n",start_time,now,end_time);
		
		if(!start_syslog)  //如果没有启动，则判断并启动
		{
			if((now>=start_time) && (now<end_time))
			{
				//printf("[_syslog_start_and_stop] _syslog_start!\n\n\n");
				sk_tr069_syslog_start(param->syslog.log_server);
				start_syslog=1;
			}
		}
		else  //如果已经启动，则判断并停止
		{
			
			//if(now>start_time+param->syslog.continue_time*60)
			if(now>start_time+param->syslog.continue_time)
			{
				//printf("[_syslog_start_and_stop] _syslog_stop!\n\n\n");
				sk_tr069_syslog_stop();
				start_syslog=0;
				param->syslog.put_type = 0;
			}
			
		}
	}

	
	return 0;
	
				
}



//上海log
static int _shlog_start_and_stop(sk_tr069_params_t 	*param)
{
	static int			start_syslog=0;   //syslog还没开始


	//printf("_syslog_start_and_stop enter!\n");

	if(param->shlog.log_enable>0)
	{
		int now;
		int start_time;
		int end_time;
		
		now=sk_tr069_gettickcount();
		
		start_time=param->shlog.uplog_starttime;
		end_time=start_time+param->syslog.continue_time;
		
		//printf("start_time=%d,now=%d,end_time=%d\n",start_time,now,end_time);
		
		if(!start_syslog)  //如果没有启动，则判断并启动
		{
			if((now>=start_time) && (now<end_time))
			{
		
				//printf("[_syslog_start_and_stop] _syslog_start!\n\n\n");
				//sk_tr069_logmsg_listen_start();   
				start_syslog=1;
			}
		}
		else  //如果已经启动，则判断并停止
		{
			if((now>=start_time) && (now<end_time))
			{
				sk_tr069_log_proc();
			}	
			//printf("now=%d,end_time=%d\n",now,end_time);
			if(now>end_time)
			{
				sk_tr069_write_logfile();
				//sk_tr069_log_proc();
				param->shlog.log_enable = 0;
				
				//printf("[_syslog_start_and_stop] _syslog_stop!\n\n\n");
				start_syslog=0;
			}
			
		}
	}

	
	return 0;
	
				
}


int sk_tr069_callback()
{
	int 					rc;
	enum evcpe_log_level 	level;
	struct evbuffer 		*buffer = NULL;
	int						i = 0;

	evcpe_info(__func__ , "enter ......\n");
    pthread_detach(pthread_self());
	//level = EVCPE_LOG_TRACE;
	//level = EVCPE_LOG_DEBUG;
	level = EVCPE_LOG_INFO;
	//level = EVCPE_LOG_WARN;
	//level = EVCPE_LOG_ERROR;
	if (level <= EVCPE_LOG_FATAL)
	{
		evcpe_add_logger("stderr", level, EVCPE_LOG_FATAL, NULL, evcpe_file_logger, stdout);
	}
    while(1)
   	{
        //用户有可能在这个时候，按了待机按键
        if(sk_tr069_get_init_status() == 0 || sk_tr069_get_shutdown_flag() == 1)
        {
            LOGI("[sk_tr069_start] may be  call shutdown !\n");
            goto finally;
        }
         
		if(0 == sk_net_check())
		{
			break;
		}
        else
        {
			usleep(1000*2000);
        }
   	}
    
	evcpe_info(__func__ ,"net check is ok !\n");

    sk_tr069_check_upgrade_successful_or_not();
    
    
	if (!(buffer = evbuffer_new()))
	{
		evcpe_error(__func__, "failed to create evbuffer");
		rc = ENOMEM;
		goto finally;
	}

	memset(&this ,  0  ,sizeof(this));

    #ifdef TR069_ANDROID
	if ((rc = load_file(sk_get_config_path(), buffer)))
	{
		evcpe_error(__func__, "failed to load data model!");
		goto finally;
	}
    #else
    if ((rc = load_file(SK_TR069_MODEL , buffer)))
	{
		evcpe_error(__func__, "failed to load data model!");
		goto finally;
	}
    #endif

    //创建数据模型类
	if (!(this.cls = evcpe_class_new(NULL)))
	{
		evcpe_error(__func__, "failed to create evcpe_class");
		rc = ENOMEM;
		goto finally;
	}
    
    //解析xml里面的schema 和 extension 标签，构建 类模型
	if ((rc = evcpe_class_from_xml(this.cls, buffer)))
	{
		evcpe_error(__func__, "failed to parse data model!");
		goto finally;
	}

    //基于类 ，创建具体的 类对象
	if (!(this.obj = evcpe_obj_new(this.cls, NULL)))
	{
		evcpe_error(__func__, "failed to create root object");
		rc = ENOMEM;
		goto finally;
	}

    //根据之前类模型中保存信息，初始化具体的类对象。构建 对象 中数据单元树模型
	if ((rc = evcpe_obj_init(this.obj)))
	{
		evcpe_error(__func__, "failed to init root object");
		goto finally;
	}
	if ((this.evbase = event_init()) == NULL)
	{
		evcpe_error(__func__, "failed to init event");
		rc = ENOMEM;
		goto finally;
	}

	if ((rc = evdns_init()))// 另外一种调用方式 evdns_base_new(this.evbase, 0)
	{
		evcpe_error(__func__, "failed to initialize DNS");
		goto finally;
	}
	if ((this.cpe = evcpe_new(this.evbase, NULL, error_cb, NULL)) == NULL)
	{
		evcpe_error(__func__, "failed to create evcpe");
		rc = ENOMEM;
		goto finally;
	}
    
    evcpe_worker_msg_queue_init();
	this.cpe->transfer_flag = 0;
	
	if (load_data_to_buffer(buffer) < 0)
	{
		goto finally;
	}

    //把xml数据内容，赋值给对象。这里有一个地方比较诡异，碰到multipleObject，会重新创建multipleObject对象元素
	if ((rc = evcpe_obj_from_xml(this.obj, buffer)))  
	{
		evcpe_error(__func__, "failed to parse data!");
		goto finally;
	}
    
 
    //创建一个repo对象
	if (!(this.repo = evcpe_repo_new(this.obj)))
	{
		evcpe_error(__func__, "failed to create repo");
		rc = ENOMEM;
		goto finally;
	}

	//add by lijingchao
    //针对江苏移动项目需要，读取多对象节点参数
    sk_tr069_preload_multipleObject_UsbInstalledAppList_construct(this.repo , this.obj);

    sk_tr069_preload_multipleObject_AppAutoRunBlackList_construct(this.repo , this.obj);
   	//add end
	
    #if 0   //定时写数据到文件中
	if (!(this.persist = evcpe_persister_new(this.evbase)))
	{
		evcpe_error(__func__, "failed to create persister");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_persister_set(this.persist, this.repo, repo_data)))
	{
		evcpe_error(__func__, "failed to set persister");
		goto finally;
	}

    #endif

 	if ((rc = evcpe_set(this.cpe, this.repo)))  //将repo的值设置到cpe中
	{
		evcpe_error(__func__, "failed to set evcpe");
		goto finally;
	}

	evcpe_info(__func__, "configuring signal action");
	
	#ifndef WIN32
	{
        struct sigaction action = {0};
    	action.sa_handler = sig_handler;
    	sigemptyset (&action.sa_mask);
    	action.sa_flags = 0;
    	if ((rc = sigaction(SIGINT, &action, NULL)) != 0)
    	{
    		evcpe_error(__func__, "failed to configure signal action");
    		goto finally;
    	}
	}
	#endif
    
	evcpe_info(__func__, "starting evcpe");
	if ((rc = evcpe_start(this.cpe)))   //开启evcpe
	{
		evcpe_error(__func__, "failed to start evcpe");
		goto finally;
	}
	

	evcpe_info(__func__, "dispatching event base");
	if ((rc = event_dispatch()) != 0)
	{
		evcpe_error(__func__, "failed to dispatch event base");
		goto finally;
	}

finally:

    if (buffer) evbuffer_free(buffer);

	return rc;
}


static void * sk_tr069_shutdown_task(void *arg)
{
    int rc = 0;
    struct evcpe *cpe = (struct evcpe *)arg;
    time_t tm = {0};
	evcpe_info(__func__ , "sk_tr069_shutdown_task enter\n");

    sk_tr069_set_call_shutdown_flag(1);
    
    pthread_detach(pthread_self());
    if(NULL != cpe)
    {
    	//if ((rc = evcpe_repo_add_event(this.cpe->repo, "M X_CTC_SHUT_DOWN", "")))
        if(sk_net_check()==0)
        {
            if(EVCPE_SESSION_STATUS_BUSY == evcpe_get_session_status(cpe))
            {
               //如果前面有消息的话，就延时多1秒
               sleep(1);
            }
            
            evcpe_worker_add_msg(EVCPE_WORKER_SHUTDOWN);
            sleep(2);
        }
		else
        {
            evcpe_info(__func__ , "network is not work!\n");
        }      
    }
finally:    
    //sk_tr069_add_boot_status(E_TR069_M_REBOOT);
    //_shutdown(0);
	//sk_tr069_set_init_status(0);
    property_set("service.tr069.state" , "stop");
    LOGI("sk_tr069_shutdown_task exit\n");

    return NULL;
}

void sk_band_width_test_response_start()
{
	band_width_test_response_start();
}


//机顶盒关闭时应该调用本方法，告知服务端STB已经shutdown了
int sk_tr069_shutdown()
{
	int rc = 0;
    struct evcpe *cpe = this.cpe;
    static	pthread_t  pid = {0};
	evcpe_info(__func__ , "sk_tr069_shutdown() call\n");
    
	if(1 != sk_tr069_get_init_status())
	{
        //网管都没有启动，直接返回。
		evcpe_error(__func__ , "sk_tr069_shutdown error , not init!\n");
		return 0;
	}

    sk_tr069_set_shutdown_flag(1);
    rc = pthread_create(&pid , NULL , sk_tr069_shutdown_task , cpe);
	if (rc != 0)
	{
		evcpe_error(__func__ , "pthread_create sk_tr069_callback  error\n");
	}

    return rc;
}


//上传日志
int sk_tr069_log_periodic()
{
	int rc;
	evcpe_handle_del_event(this.cpe);
	if ((rc = evcpe_repo_add_event(this.cpe->repo, "M CTC LOG_PERIODIC", "")))
	{
		evcpe_error(__func__, "failed to add boot event");
		goto finally;
	}
	this.cpe->event_flag=EVCPE_NO_BOOT;
	
	//sk_tr069_set_logmsg_event(1);
	
	if ((rc = evcpe_start_session(this.cpe))) //建立会话，并组装inform包，发送出去
	{
		evcpe_error(__func__, "failed to start session");
		goto finally;
	}

finally:
		return rc;
}

int sk_tr069_awakening()
{
    int rc = 0;
    evcpe_t * evcpe = get_evcpe_object();
    struct evcpe *cpe = evcpe->cpe;
    pthread_detach(pthread_self());
    if(NULL != cpe)
    {

        while(1)
       	{
            //用户有可能在这个时候，按了待机按键
            if(sk_tr069_get_init_status() == 0 || sk_tr069_get_shutdown_flag() == 1)
            {
                LOGI("[sk_tr069_awakening] may be  call shutdown !\n");
                goto fially;
            }
       
            if(0 == sk_net_check())
    		{
    			break;
    		}
            else
            {
    			//usleep(1000*2);
    			//usleep(1000*500);
    			//usleep(1000*5000);
    			usleep(1000*2000);
            }
       	}

        sk_tr069_check_upgrade_successful_or_not();
        
        evcpe_worker_add_msg(EVCPE_WORKER_AWAKEN);
    	
    }

fially:
    return 0;
}


//本函数完成tr069的启动
int sk_tr069_start()
{
	static  pthread_t  g_tr069_pid ;
    static  pthread_t  m_awakening_pid;
	int 	rc = -1;

    LOGI("===============================================\n");
    LOGI("*           welcome skyworth tr069            *\n");
    LOGI("*\t build time:%s\n", sk_tr069_get_buildtime());
    LOGI("===============================================\n");
    
    LOGI("[sk_tr069_start]enter!");
	sk_tr069_print_time("sk_tr069_start");
    LOGI("[sk_tr069_start] begin...net check...!\n");



    if(0 != sk_tr069_get_init_status())
    {
       LOGI("[sk_tr069_start] tr069 have been start! run.....\n");
        //针对江苏移动 网管待机后，重新唤醒，发1 BOOT事件，特殊处理
        property_set("service.tr069.state" , "running");
        sk_tr069_set_shutdown_flag(0);
        rc = pthread_create(&m_awakening_pid , NULL, sk_tr069_awakening, NULL);
    	if (rc != 0)
    	{
    		LOGI("pthread_create sk_tr069_awakening  error\n");
    	}
       return 0;
    }

    sk_tr069_set_init_status(1);
    sk_tr069_set_shutdown_flag(0);

    property_set("service.tr069.state" , "running");

	sk_tr069_init();
   
    sk_set_reauth_event_code();
  

	rc = pthread_create(&g_tr069_pid, NULL, sk_tr069_callback, NULL);
	if (rc != 0)
	{
		evcpe_error(__func__ , "pthread_create sk_tr069_callback  error\n");
	}

   
    evcpe_info(__func__ , "[sk_tr069_start]OK exit!rc = %d",rc);
	return rc;
}


extern  sk_tr069_params_t			m_tr069_params;					//所有局部静态参数都通过这个结构来管理


int sk_tr069_xinneng_reportaction(struct evcpe *mycpe)
{
	int rc; 

      evcpe_handle_del_event(mycpe);      
   		
	if ((rc = evcpe_repo_add_event(mycpe->repo, "X CTC MONITOR", "")))
	{
		evcpe_info(__func__ , "failed to add boot event");
		goto finally;
	}
   	
	
	mycpe->event_flag=EVCPE_NO_BOOT;

	if ((rc = evcpe_start_session(mycpe))) //建立会话，并组装inform包，发送出去
	{
		evcpe_info(__func__ , "failed to start session");
		goto finally;
	}

finally:
		return rc;
}



int sk_tr069_get_init_status()
{
    return m_init_ok ;
}

void sk_tr069_set_init_status(int status)
{
    m_init_ok = status;
}


static int sk_tr069_get_shutdown_flag()
{
    return m_shutdown_flag ;
}

static void sk_tr069_set_shutdown_flag(int status)
{
    m_shutdown_flag = status;
}


extern int sk_tr069_get_call_shutdown_flag()
{
    return g_call_shutdown;
}

extern void sk_tr069_set_call_shutdown_flag(int flag)
{
    g_call_shutdown = flag;
}


static char *sk_tr069_get_buildtime(void)
{
    return __DATE__" "__TIME__;
}


#ifdef WIN32

//add by lijc 用于内存泄露打印
#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_CLIENTBLOCK
#endif  // _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif  // _DEBUG
//end

static void * input_proc(void *arg)
{
   unsigned char input_data[256] = {0};
   int flag_quit = 0;
   pthread_detach(pthread_self());

   do
   {
       memset(input_data , 0 , sizeof(input_data));
       gets(input_data);
       printf("input_data = %s\n",input_data);
       if(!strcmp(input_data , "quit"))
       {
           flag_quit = 1 ;
       }
       else if(!strcmp(input_data , "shutdown"))
       {
            sk_tr069_shutdown();
            sleep(5);
            sk_func_porting_params_exit();
            if(fp_debug)
            {
               fflush(fp_debug);
            }
    
       }
       else if(!strcmp(input_data , "start"))
       {
            sk_func_porting_params_init();
            sk_tr069_start();
       }
       else if(!strcmp(input_data , "setip"))
       {
            sk_func_porting_params_set("lan_ip","172.28.17.244");
       }
       else if(!strcmp(input_data , "help"))
       {
            printf("*\t start\n");
            printf("*\t shutdown\n");
            printf("*\t setip\n");
            printf("*\t quit\n");
            printf("*\t help\n");
       }
       
   }while(!flag_quit);

}

static int debug_evbuffer_read(struct evbuffer *buf, evutil_socket_t fd, ev_ssize_t howmuch)
{
	int result;
	int nchains, n;
	struct evbuffer_iovec v[2];

	//EVBUFFER_LOCK(buf);
	/*
	if (buf->freeze_end) {
	result = -1;
	goto done;
	}
	*/
	if (howmuch < 0)
		howmuch = 16384;


	/* XXX we _will_ waste some space here if there is any space left
	* over on buf->last. */
	nchains = evbuffer_reserve_space(buf, howmuch, v, 2);
	if (nchains < 1 || nchains > 2) {
		result = -1;
		goto done;
	}
	n = read((int)fd, v[0].iov_base, (unsigned int)v[0].iov_len);
	if (n <= 0) {
		result = n;
		goto done;
	}
	v[0].iov_len = (size_t) n; /* XXXX another problem with big n.*/
	if (nchains > 1) {
		n = read((int)fd, v[1].iov_base, (unsigned int)v[1].iov_len);
		if (n <= 0) {
			result = (unsigned long) v[0].iov_len;
			evbuffer_commit_space(buf, v, 1);
			goto done;
		}
		v[1].iov_len = n;
	}
	evbuffer_commit_space(buf, v, nchains);

	result = n;
done:
	//EVBUFFER_UNLOCK(buf);
	return result;
}

int main(void)
{
    static  pthread_t  sk_input_pid = {0};
    int rc = 0;

	WSADATA wsaData;  
	DWORD Ret;

    printf("[lijc for debug]main start .....................!\n");
    fp_debug = fopen("./debug_file","wb");
    if(NULL == fp_debug)
    {
        printf("fopen debug_file failed!\n");
        goto LAB_ERROR;
    }
    
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) 
	{  
		printf("WSAStartup failed with error %d\n", Ret);  
	    goto LAB_ERROR;
	}  
 
    rc = sk_func_porting_params_init();
    if(0 != rc)
    {
        printf("sk_func_porting_params_init error\n");  
	    goto LAB_ERROR;
    }
    
    sk_tr069_start();

	input_proc(NULL);
    
    if(NULL != fp_debug)
    {
        fflush(fp_debug);
        fclose(fp_debug);
        fp_debug = NULL;
    }
    sk_func_porting_params_exit();
    printf("main OK exit .....................!\n");
    return 0;
    
LAB_ERROR:
    printf("main Error exit .....................!\n");
    return -1;
}
#endif//WIN32
