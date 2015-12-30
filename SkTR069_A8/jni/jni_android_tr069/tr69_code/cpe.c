// $Id: cpe.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <time.h>

#include <evdns.h>

#include "log.h"
#include "util.h"
#include "msg.h"
#include "data.h"
#include "fault.h"
#include "inform.h"
#include "get_rpc_methods.h"
#include "get_param_names.h"
#include "get_param_values.h"
#include "get_param_attrs.h"
#include "add_object.h"
#include "delete_object.h"
#include "set_param_values.h"

#include "http_author_util.h"
#include "http_digest_calc.h"


#include "cpe.h"
#include "sk_tr069.h"


//+ by aobai
#include "download.h"
#include "upload.h"
#include "reboot.h"
#include "factory_reset.h"
#include "evcpe.h"  // use shutdonw()
#include "accessor.h"



static void evcpe_send_error(struct evcpe *cpe, enum evcpe_error_type type,
		int code, const char *reason);

static void evcpe_creq_cb(struct evhttp_request *req, void *arg);

static void evcpe_session_message_cb(struct evcpe_session *session,
		enum evcpe_msg_type type, enum evcpe_method_type method_type,
				void *request, void *response, void *arg);

static void evcpe_session_terminate_cb(struct evcpe_session *session,
		int code, void *arg);

static void evcpe_dns_cb(int result, char type, int count, int ttl,
	    void *addresses, void *arg);

static inline void evcpe_dns_timer_cb(int fd, short event, void *arg);

static int evcpe_dns_entry_resolve(struct evcpe_dns_entry *entry,
		const char *hostname);

static int evcpe_dns_add(struct evcpe *cpe, const char *hostname);

static inline int evcpe_handle_request(struct evcpe *cpe,
		struct evcpe_session *session,
		enum evcpe_method_type method_type, void *request);

static inline int evcpe_handle_response(struct evcpe *cpe,
		enum evcpe_method_type method_type, void *request, void *response);

static inline int evcpe_handle_get_rpc_methods(struct evcpe *cpe,
		struct evcpe_get_rpc_methods *req, struct evcpe_msg *msg);

static inline int evcpe_handle_get_param_names(struct evcpe *cpe,
		struct evcpe_get_param_names *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_get_param_values(struct evcpe *cpe,
		struct evcpe_get_param_values *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_get_param_attrs(struct evcpe *cpe,
		struct evcpe_get_param_attrs *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_add_object(struct evcpe *cpe,
		struct evcpe_add_object *req,
		struct evcpe_msg *msg);


static inline int evcpe_handle_delete_object(struct evcpe *cpe,
		struct evcpe_delete_object *req,
		struct evcpe_msg *msg);

static inline int evcpe_handle_set_param_values(struct evcpe *cpe,
		struct evcpe_set_param_values *req,
		struct evcpe_msg *msg);




//+ by aobai
static inline int evcpe_handle_download(struct evcpe *cpe,
										struct evcpe_download *req,
										struct evcpe_msg *msg);

static inline int evcpe_handle_upload(struct evcpe *cpe,
									  struct evcpe_upload *req,
									  struct evcpe_msg *msg);
static int evcpe_handle_reboot(struct evcpe *cpe,
									  struct evcpe_reboot *req, 
									  struct evcpe_msg *msg);
//+

static inline int evcpe_handle_inform_response(struct evcpe *cpe,
		struct evcpe_inform *req, struct evcpe_inform_response *resp);

static int evcpe_retry_session(struct evcpe *cpe);
static void evcpe_start_session_cb(int fd, short event, void *arg);
static int evcpe_send_transfer_complete(struct evcpe *cpe);

static void evcpe_worker_start_session_cb(int fd, short event, void *arg);

static int  informtime = 0;





struct evcpe *evcpe_new(struct event_base *evbase,
		evcpe_request_cb cb, evcpe_error_cb error_cb, void *cbarg)
{
	struct evcpe *cpe;

	evcpe_debug(__func__, "constructing evcpe");

	if ((cpe = calloc(1, sizeof(struct evcpe))) == NULL) {
		evcpe_error(__func__, "failed to calloc evcpe");
		return NULL;
	}
	RB_INIT(&cpe->dns_cache);
	cpe->evbase = evbase;
	cpe->cb = cb;
	cpe->error_cb = error_cb;
	cpe->cbarg = cbarg;

	return cpe;
}

void evcpe_free(struct evcpe *cpe)
{
	if (cpe == NULL) return;

	evcpe_debug(__func__, "destructing evcpe");

	if (event_initialized(&cpe->retry_ev) &&
			evtimer_pending(&cpe->retry_ev, NULL)) {
		event_del(&cpe->retry_ev);
	}
	if (event_initialized(&cpe->periodic_ev) &&
			evtimer_pending(&cpe->periodic_ev, NULL)) {
		event_del(&cpe->periodic_ev);
	}

    if (event_initialized(&cpe->session_ev) &&
			evtimer_pending(&cpe->session_ev, NULL)) {
		event_del(&cpe->session_ev);
	}

    if (event_initialized(&cpe->worker_loop_ev) &&
			evtimer_pending(&cpe->worker_loop_ev, NULL)) {
		printf("event_del worker_loop_ev\n");
		event_del(&cpe->worker_loop_ev);
	}
      
	if (cpe->session) 
	{
        evcpe_session_free(cpe->session);
	}
	if (cpe->http) 
    {
	    evhttp_free(cpe->http);
	}
        
	if (cpe->acs_url) 
    {   
		evcpe_url_free(cpe->acs_url);
	}
	
	if (cpe->proxy_url) 
	{
		evcpe_url_free(cpe->proxy_url);
	}
	
	if (cpe->creq_url)
	{
		evcpe_url_free(cpe->creq_url);
	}
	evcpe_dns_cache_clear(&cpe->dns_cache);

    evcpe_worker_msg_queue_clear(&cpe->worker_queue);

	free(cpe);
}

int evcpe_set(struct evcpe *cpe,struct evcpe_repo *repo)
{
	int rc = 0;
    int periodic_tv = 0 ;
    int inform_enable = 0;
	char 	*value=NULL;
	char	szBuffer[SK_BUFFER_SIZE]={0};
	
	unsigned int len=SK_BUFFER_SIZE;
	evcpe_info(__func__ , "android tr069 evcpe_set \n");

	len=SK_BUFFER_SIZE;
	if(sk_tr069_get_authtype(NULL,&value,&len)!=0)
	{
		evcpe_error(__func__, "failed to get ACS authentication type");
		goto finally;
	}

	strcpy(szBuffer,value);
	
	if (!strcmp("NONE", szBuffer))			//无认证,没有做任何认证
		cpe->acs_auth = EVCPE_AUTH_NONE;
	else if (!strcmp("BASIC", szBuffer))	//基本认证
		cpe->acs_auth = EVCPE_AUTH_BASIC;
	else if (!strcmp("DIGEST", szBuffer))	//加密认证
		cpe->acs_auth = EVCPE_AUTH_DIGEST;
	else 
	{
		evcpe_error(__func__, "invalid authentication value: %s", value);
		rc = EINVAL;
		goto finally;
	}

	
	len=SK_BUFFER_SIZE;
	if(sk_tr069_get_acsurl(NULL,&value,&len)!=0)
	{
		evcpe_error(__func__, "failed to get ACS authentication url");
		goto finally;
	}
 
	strcpy(szBuffer,value);
	if (!(cpe->acs_url = evcpe_url_new())) 
	{
		evcpe_error(__func__, "failed to create evcpe_url");
		rc = ENOMEM;
		goto finally;
	}

	 
	
	if ((rc = evcpe_url_from_str(cpe->acs_url, szBuffer))) 
	{
		evcpe_error(__func__, "failed to parse ACS URL: %s", szBuffer);
		goto finally;
	}
	
   	evcpe_info(__func__ , "acs_url=%s\n\n",szBuffer);

	evcpe_info(__func__ , "evcpe_dns_add:%s\n",cpe->acs_url->host);
	if ((rc = evcpe_dns_add(cpe, cpe->acs_url->host))) 
	{
		evcpe_error(__func__, "failed to resolve ACS hostname");
		goto finally;
	}


	
	//acs用户名和密码
	len=sizeof(cpe->acs_username);
	sk_tr069_get_acsuser(NULL,&value,&len);
	strcpy(cpe->acs_username,value);
	
	len=sizeof(cpe->acs_password);
	sk_tr069_get_acspass(NULL,&value,&len);
	strcpy(cpe->acs_password,value);
	
	cpe->acs_timeout = EVCPE_ACS_TIMEOUT;

	len = SK_BUFFER_SIZE;
	if(sk_tr069_get_request_url(NULL,&value,&len)!=0)
	{
		evcpe_error(__func__, "failed to get ACS authentication");
		goto finally;
	}
	strcpy(szBuffer,value);
	
	evcpe_info(__func__ , "android tr069 request_url=%s\n",szBuffer);
	
	if (!(cpe->creq_url = evcpe_url_new())) 
	{
		evcpe_error(__func__, "failed to create evcpe_url");
		rc = ENOMEM;
		goto finally;
	}
	//从value里面查找url包括地址和端口,如果没有找到端口,则使用默认端口
	if ((rc = evcpe_url_from_str(cpe->creq_url, szBuffer))) 
	{
		evcpe_error(__func__, "failed to parse CReq URL");
		goto finally;
	}

	//acs用户名和密码
	len=sizeof(cpe->acs_username);
	sk_tr069_get_requser(NULL,&value,&len);
	strcpy(cpe->creq_username,value);
	len=sizeof(cpe->acs_password);
	sk_tr069_get_reqpass(NULL,&value,&len);
	strcpy(cpe->creq_password,value);
	
	cpe->creq_interval = EVCPE_CREQ_INTERVAL;

	evcpe_info(__func__ , "kangzh android tr069 evcpe_set 3\n");

    sk_tr069_get_inform_enable(NULL , &value , &len);	

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
	    evcpe_enable_periodic_inform(cpe);
    }
    
	cpe->repo = repo;
    evutil_timerclear(&cpe->worker_loop_tv);
    cpe->worker_loop_tv.tv_sec = 0 ;
    cpe->worker_loop_tv.tv_usec = 100000;
    event_assign(&cpe->worker_loop_ev , cpe->evbase , -1 , EV_PERSIST , evcpe_worker_loop , cpe);
    event_add(&cpe->worker_loop_ev , &cpe->worker_loop_tv);


    evtimer_set(&cpe->session_ev, evcpe_worker_start_session_cb, cpe);
	if ((rc = event_base_set(cpe->evbase, &cpe->session_ev))) 
	{
		evcpe_error(__func__, "failed to set event base");
		goto finally;
	}

finally:
	return rc;
}


static int evcpe_get_auth_str(char *auth_str,struct evhttp_request *req)
{
	char auth_str2[1024]={0};
	HTTPDigest_Fields digestFields;
	HTTPAuth_Config config={0};
	char method[20]="GET";
	char requser[64]={0};
	char reqpwd[64]={0};
	char requrl[256]={0};
	char response_user[64]={0};
	char response_str[64]={0};
	char response_cnonce[64]={0};
	
/*	
	char algorithm[32]={0};
	char realm[32]={0};
	char nonce[32]={0};
	char qop[32]={0};
*/	
	//
	char *value=NULL;
	char *response=NULL;
	int size;
	int i;
	int len=0;
	
	strcpy(auth_str2,auth_str);
	//printf("auth_str2=%s\n",auth_str2);
	config.eAbility=HTTPDigest_Config_Algorithm_MD5;
	config.eAuthType=E_HTTPAUTH_TYPE_DIGEST;
	config.lNonceCount=1;
	config.pcMethod=method;
	
	sk_tr069_get_requser(NULL,&value,&len);
	strcpy(requser,value);
	config.pcUserName=requser;

	sk_tr069_get_reqpass(NULL,&value,&len);
	strcpy(reqpwd,value);
	config.pcPassword=reqpwd;
	
	sk_tr069_get_request_url(NULL,&value,&len);
	strcpy(requrl,value);
	config.pcUri=req->uri;

	memset(digestFields, 0, HTTPDigest_Field_End*sizeof(CHAR *));

	
	value=strstr(auth_str2,"username=");
	if(value!=NULL)
	{
		i=0;
		value+=strlen("username=")+1;
		size=sizeof(response_user);
		while((value[i]!='\"') && (i<size) && (value[i]!='\0'))
		{
			i++;
			//printf("i=[%d]\n",i);
		}	
		
		strncpy(response_user,value,i);
		response_user[i]='\0';
		//printf("response_user=%s\n",response_user);
	}
	//判断上报用户名是否正确
	if(strcmp(response_user,requser))
	{
		//printf("[evcpe_get_auth_str] username is not match!,response_user=%s,requser=%s\n",response_user,requser);
		return -1;
	}

	value=strstr(auth_str2,"cnonce=");
	if(value!=NULL)
	{
		i=0;
		value+=strlen("cnonce=")+1;
		size=sizeof(response_cnonce);
		while((value[i]!='\"') && (i<size) && (value[i]!='\0'))
		{
			i++;
//			printf("i=[%d]\n",i);
		}	
		//printf("evcpe_get_auth_str i=%d\n",i);
		strncpy(response_cnonce,value,i);
		response_cnonce[i]='\0';
		//printf("response_cnonce=%s\n",response_cnonce);
	}
	config.pcNonce=response_cnonce;



	
	//printf("abc\n");
	value=strstr(auth_str2,"response=");
	
	if(value!=NULL)
	{
		i=0;
		
		value+=strlen("response=")+1;
		size=sizeof(response_str);
		while((value[i]!='\"') && (i<size) && (value[i]!='\0'))
		{
			i++;
			
		}
		strncpy(response_str,value,i);
		response_str[i]='\0';
		//printf("response_str=%s\n",response_str);
	}	
	//printf("abc2\n");

	digestFields[HTTPDigest_Field_algorithm] = "MD5";
	digestFields[HTTPDigest_Field_realm] = "testrealm@host.com";
	digestFields[HTTPDigest_Field_nonce] = "abcdef";
	digestFields[HTTPDigest_Field_qop] = "auth";
	
	
	HTTPAuth_GetAuthResponse(&config,digestFields,&response);
	if(response!=NULL)
	{
		//判断上报加密码是否正确
		if(strcmp(response,response_str))
		{
			free(response);
			response=NULL;
			//printf("[evcpe_get_auth_str] response is not match!response=%s,response_str=%s\n",response,response_str);
			return -1;
		}
		free(response);
		response=NULL;
	}
	
	//printf("[evcpe_get_auth_str] success!\n");
	return 0;

	
		
}
void evcpe_creq_resonse_auth(struct evhttp_request *req)
{
	evhttp_add_header(req->output_headers,"WWW-Authenticate","Digest realm=\"testrealm@host.com\", algorithm=\"MD5\", qop=\"auth\", nonce=\"abcdef\"");
	evhttp_send_reply(req, 401, "Unauthorized", NULL);
}

void evcpe_creq_cb(struct evhttp_request *req, void *arg)
{
	time_t curtime;
	struct evcpe *cpe = arg;
	int duration;
	char *authentication=NULL;
	evcpe_info(__func__,"evcpe_creq_cb enter!\n\n");

	if (req->type != EVHTTP_REQ_GET || (cpe->creq_url->uri &&
										strncmp(cpe->creq_url->uri,
										evhttp_request_uri(req), strlen(cpe->creq_url->uri))))
	{
        evcpe_error(__func__ , "evcpe_creq_cb evhttp_send_reply 404!\n\n");
		evhttp_send_reply(req, 404, "Not Found", NULL);
		return;
	}
	else	//如果接收到请求数据
	{
		//如果当前正在下载，则不做任何处理，直接丢弃该链接请求
		if(sk_tr069_get_upgrade_status()==EVCPE_DOWNLOADING)
			return;

		//改变第一次连接状态
		sk_tr069_set_first_connect_status(0);
		
        #if 1
		//WWW-Authenticate									   Authorization
		//对于新的请求，则入队列,进行集中处理
		authentication=evhttp_find_header(req->input_headers, "Authorization");
		if(authentication==NULL)
		{
			evcpe_info(__func__,"evcpe_creq_cb enter! evhttp_find_header return authentication==NULL \n");
			evcpe_creq_resonse_auth(req);
		}
		else
		{

			#if 0
			if(evcpe_get_auth_str(authentication,req)<0)
				evcpe_creq_resonse_auth(req);
			else
				evhttp_send_reply(req, 200, "OK", NULL);
            #else
            evhttp_send_reply(req, 200, "OK", NULL);
            evcpe_worker_add_msg(EVCPE_WORKER_CONNECTION_REQUEST);
            #endif

		}
        #else
    	//如果浙江需求不能满足，则打开这里
		int duration;
		curtime = time(NULL);
		duration=difftime(curtime, cpe->creq_last);

		if (cpe->creq_last && duration< cpe->creq_interval)
		{
		    printf("evcpe_creq_cb[] duration=%d\n",duration);
			evhttp_send_reply(req, 503, "Service Unavailable", NULL);
			return;
		}
		else
		{
			if (cpe->session)  //如果已经建立会话
			{
				evhttp_send_reply(req, 503, "Session in Progress", NULL);
				return;
			}
			
			if (evcpe_repo_add_event(cpe->repo, "6 CONNECTION REQUEST", ""))
			{
				evcpe_error(__func__, "failed to add connection request event");
				evhttp_send_reply(req, 501, "Internal Server Error", NULL);
				return;
			}


			//kangzh:这里应有对来自管理平台的认证处理,
			//机顶盒对终端管理平台认证通过后，要求机顶盒重新对终端管理平台发起
			//HTTP Digest Authentication认证请求。


			//要求客户端去建立连接
			cpe->event_flag=EVCPE_NO_BOOT;
			if (evcpe_start_session(cpe)) //如果会话已经建立，或者由于其他原因返回
			{
				evcpe_error(__func__, "failed to start session");
				evhttp_send_reply(req, 501, "Internal Server Error", NULL);
				return;
			}
			else
			{
				cpe->creq_last = curtime;
				evhttp_send_reply(req, 200, "OK", NULL);
				return;
			}
			
		}
    #endif
	}
}

int evcpe_bind(struct evcpe *cpe)  //建立http连接
{
	int rc;
	struct evhttp *http;

	evcpe_info(__func__, "binding %s on port: %d",
			cpe->creq_url->protocol, cpe->creq_url->port);

	// TODO: SSL

	if (!(http = evhttp_new(cpe->evbase)))
	{
		evcpe_error(__func__, "failed to create evhttp");
		rc = ENOMEM;
		goto finally;
	}
	
  	if ((rc = evhttp_bind_socket(http, "0.0.0.0", cpe->creq_url->port)) != 0)
	{
		evcpe_error(__func__, "failed to bind evhttp on port:%d",
				cpe->creq_url->port);
		evhttp_free(http);
		goto finally;
	}

	if (cpe->http) evhttp_free(cpe->http);
	cpe->http = http;
	evcpe_info(__func__, "accepting connection request: %s", cpe->creq_url->uri);
	evhttp_set_gencb(cpe->http, evcpe_creq_cb, cpe);  //设置请求回调

finally:
	return rc;
}

void evcpe_send_error(struct evcpe *cpe, enum evcpe_error_type type,
		int code, const char *reason)
{
	evcpe_error(__func__, "%d type error: %d %s", type, code, reason);
	if (cpe->error_cb)
		(*cpe->error_cb)(cpe, type, code, reason, cpe->cbarg);
}


//重试会话
int evcpe_retry_session(struct evcpe *cpe)
{
	int rc, base, secs;

	if (evtimer_pending(&cpe->retry_ev, NULL))
	{
		evcpe_error(__func__, "another session retry has been scheduled");
		rc = EINVAL;
		goto finally;
	}

	if (cpe->retry_count == 0)
	{
		secs = 0;
	}
	else
	{
		switch (cpe->retry_count)
		{
		case 1:
			base = 5;
			break;
		case 2:
			base = 10;
			break;
		case 3:
			base = 20;
			break;
		case 4:
			base = 40;
			break;
		case 5:
			base = 80;
			break;
		case 6:
			base = 160;
			break;
		case 7:
			base = 320;
			break;
		case 8:
			base = 640;
			break;
		case 9:
			base = 1280;
			break;
		default:
			base = 2560;
			break;
		}
		secs = base + rand() % base;
	}

	evcpe_info(__func__, "scheduling session retry in %d second(s)", secs);

	evutil_timerclear(&cpe->retry_tv);
	cpe->retry_tv.tv_sec = secs;
	if ((rc = event_add(&cpe->retry_ev, &cpe->retry_tv)))
		evcpe_error(__func__, "failed to add timer event");

finally:
	return rc;
}


int evcpe_add_to_event(int value,struct evcpe *cpe)
{
	int 	rc=-1;
	int 	number=0;
	char	buffer[100]={0};

	//printf("evcpe_add_to_event,enter!value=%d\n",value);
	
	if( cpe==NULL)
		goto finally;
	
	//printf("evcpe_add_to_event,value=%d,is_booting=%d",value,is_booting);
	evcpe_info(__func__, "enter!value=%d\n",value);
	
	{
		number=value;
	
		switch(number)
		{
			case E_TR069_BOOTSTRAP:
				strcpy(buffer,"0 BOOTSTRAP");
				break;
			case E_TR069_BOOT:
				strcpy(buffer,"1 BOOT");
				break;
			case E_TR069_PERIODIC:
				strcpy(buffer,"2 PERIODIC");
				break;
			case E_TR069_VALUE_CHANGED:
				strcpy(buffer,"4 VALUE CHANGE");
				break;
			case E_TR069_TRANSFER_COMPLETE:
				cpe->transfer_flag=1;
				strcpy(buffer,"7 TRANSFER COMPLETE");
				break;
			case E_TR069_DIAGNOSTICS_COMPLETE:
				strcpy(buffer,"8 DIAGNOSTICS COMPLETE");
				break;
			case E_TR069_M_REBOOT:
				strcpy(buffer,"M Reboot");
				break;
			case E_TR069_LOG_PERIODIC:
				strcpy(buffer,"M CTC LOG_PERIODIC");
				break;
			case E_TR069_SHUT_DOWN:
				//strcpy(buffer,"M X_CTC_SHUT_DOWN");
				strcpy(buffer, "X CMCC Shutdown");
				break;
			default:
			{
				evcpe_error(__func__, "no surpport event!");
			}
			break;		
		}
	}	
	if ((rc = evcpe_repo_add_event(cpe->repo, buffer, ""))) 
	{
		evcpe_error(__func__, "failed to add boot event");
		goto finally;
	}
	
	return 0;
	
finally:
	return rc;
	
}
int evcpe_get_boot_status(struct evcpe *cpe,int event_flag)
{
	int 	rc=0;
	int 	len;
	int     boot_status;
		
	char	str_number[20]={0};
	
	char	*value=NULL;
	char	*p_start=NULL;
	char	*p_end=NULL;
	int		value_int=0;

	//printf("enter!event_flag=%d\n",event_flag);
	evcpe_info(__func__, "enter!event_flag=%d",event_flag);
	
	sk_tr069_get_boot_status(&value,&len);
	if(value==NULL)
	{
		goto finally;
	}
    
	if(strlen(value)<=0)
	{
		strcpy(value,"none");
	}
    
	evcpe_info(__func__, "enter!tr069_boot=%s",value);
    //printf("[tr069] tr069_boot=%s\n",value);
    
	if(!strcmp(value,"0") || !strcmp(value,"none"))
	{
        //E_TR069_BOOTSTRAP
		return evcpe_add_to_event(E_TR069_BOOTSTRAP,cpe);
	}
	else
	{
		if(event_flag==E_TR069_PERIODIC)
		{
			return evcpe_add_to_event(E_TR069_PERIODIC,cpe);
		}
		
		if(event_flag==E_TR069_BOOT)
		{
			evcpe_add_to_event(E_TR069_BOOT,cpe);
		}
		p_start=value;
		p_end=value;
		while((*p_end!='\0') && (*p_end!=' '))
		{
			if(*p_end==',')
			{
				len=p_end-p_start;
				memset(str_number,0,sizeof(str_number));
				strncpy(str_number,p_start,len);
				
				value_int=atoi(str_number);
				if(E_TR069_BOOT != value_int)
				{
				    //printf("evcpe_get_boot_status,str_number=%s,value_int=%d\n",str_number,value_int);
                   evcpe_add_to_event(value_int,cpe);
				}
            
				if((*p_end+1)!='\0')
				{
					p_start=p_end+1;
				}
				p_end++;
				
			}
			else
			{
				p_end++;
			}
		}
        
		if(p_start!=NULL)
		{
			value_int=atoi(p_start);
			if(E_TR069_BOOT != value_int)
			{
				//printf("evcpe_get_boot_status,p_start=%s,value_int=%d\n",p_start,value_int);
				evcpe_add_to_event(value_int,cpe);
			}
      
		}

      
	}
//	evcpe_listen_params_value_change(cpe);
//	evcpe_log_periodic(cpe);
finally:
	return rc;	
}


//evcpe启动
int evcpe_start(struct evcpe *cpe)
{
	int 	rc;
	
	evcpe_info(__func__ , " evcpe_start  enter!\n");
	
	if (!cpe->repo)
	{
		evcpe_error(__func__, "evcpe is not initialized");
		rc = EINVAL;
		goto finally;
	}

	evcpe_info(__func__, "starting evcpe");

	//绑定本地socket，并监听来自server的主动请求,如果server有请求，则判断会话是否已经建立，如果已经建立，则不启动
	if (cpe->creq_url && (rc = evcpe_bind(cpe)))   
	{
		evcpe_error(__func__, "failed to bind for connection request!!!! uri:%s  error:%d\n",cpe->creq_url->uri,errno);
		goto finally;
	} 
	//kangzh,去掉重试会话，定时器
	//evtimer_set(&cpe->retry_ev, evcpe_start_session_cb, cpe);
	cpe->event_flag = E_TR069_BOOT;
	if ((rc = evcpe_start_session(cpe))) //建立会话，并组装第一个inform包，发送出去
	{
		evcpe_error(__func__, "failed to start session");
		goto finally;
	}

		
	rc = 0;

finally:
	evcpe_info(__func__ , "evcpe_start return rc = %d\n",rc);
	return rc;
}


//定时器回调，建立会话，并发送第一个inform包
void evcpe_start_session_cb(int fd, short ev, void *arg)
{
	int rc = 0;
	struct evcpe *cpe = arg;
    char 	*value = NULL;
	unsigned int len =  0;
    int inform_enable = 0;
    int periodic_sec = 0 ;
  

    if(NULL == cpe)
    {
        goto finally;
    }

    sk_tr069_get_inform_enable(NULL , &value , &len);	

	if(NULL != value && len > 0)
	{
		sscanf(value ,"%d" , &inform_enable);
	}
    else
    {
		inform_enable = 1;
    }

    if(inform_enable && (sk_tr069_get_upgrade_status() !=  EVCPE_DOWNLOADING) && 0 == sk_tr069_get_first_connect_status())
    {
        //struct evcpe_worker_msg * msg = evcpe_worker_msg_queue_find_msg(&cpe->worker_queue , EVCPE_WORKER_2_PERIODIC);
        //避免周期时间过短，重复添加多次周期事件。
        struct evcpe_worker_msg * msg = TAILQ_FIRST(&cpe->worker_queue);
        
        if(NULL == msg)
        {
            evcpe_worker_add_msg(EVCPE_WORKER_2_PERIODIC);
        }
    }

    value = NULL;
    sk_tr069_get_period_inform_interval(NULL , &value , &len);	
    if(NULL != value && len > 0)
	{
		sscanf(value ,"%d" , &periodic_sec);
	}
    else
    {
		periodic_sec = TR069_INFORM_INTERVAL;
    }
    
	cpe->periodic_tv.tv_sec = periodic_sec;
 

	//event_initialized()用于检测event结构体变量是否已经初始化.
	if (event_initialized(&cpe->periodic_ev) &&	!evtimer_pending(&cpe->periodic_ev, NULL)) 
	{
		evcpe_info(__func__, "scheduling periodic inform in %ld second(s)",	cpe->periodic_tv.tv_sec);
		if ((rc = event_add(&cpe->periodic_ev, &cpe->periodic_tv)))
			evcpe_error(__func__, "failed to schedule periodic inform");
   
	}
	
finally:
	return rc;	
}

int evcpe_create_inform(struct evcpe *cpe)
{
	struct evcpe_inform *inform=NULL;
	struct evcpe_msg *msg=NULL;
	int rc=0;

   	evcpe_info(__func__ , "[tr069] evcpe_create_inform \n");

	if (!(msg = evcpe_msg_new()))  //创建消息体结构
	{
		evcpe_error(__func__, "failed to create evcpe_msg");
		rc = ENOMEM;
		goto exception;
	}
	if (!(msg->session = evcpe_ltoa(random()))) 
	{
		evcpe_error(__func__, "failed to create session string");
		rc = ENOMEM;
		goto exception;
	}
	msg->major = 1;
	msg->minor = 0;
	if (!(msg->data = inform = evcpe_inform_new()))   //创建inform请求
	{
		evcpe_error(__func__, "failed to create inform request");
		rc = ENOMEM;
		goto exception;
	}

	evcpe_info(__func__ , "[tr069] evcpe_create_inform 2\n");

	msg->type = EVCPE_MSG_REQUEST;
	msg->method_type = EVCPE_INFORM;
	
	inform->retry_count = cpe->retry_count;

	if (evcpe_repo_to_inform(cpe->repo, inform)) //准备inform请求数据
	{
		evcpe_error(__func__, "failed to prepare inform message");
		rc = ENOMEM;
		goto exception;
	}

	evcpe_info(__func__ , "[tr069]evcpe_create_inform 3\n\n");
	if ((rc = evcpe_session_send(cpe->session, msg)))   //将inform包放入发送队列中
	{
		evcpe_error(__func__, "failed to send inform message");
		goto exception;
	}
	evcpe_info(__func__ , "[tr069] evcpe_create_inform 4\n");
	//会话开始,从队列中取得第一个排队包，发送出去，并将对头数据从等待队列中删除
	if ((rc = evcpe_session_start(cpe->session)))   //
	{
		evcpe_info(__func__ , "[tr069] evcpe_create_inform 5,back\n");
		evcpe_error(__func__, "failed to start session");
		goto finally;
	}
	
	evcpe_info(__func__ , "[tr069] evcpe_create_inform 5\n");
	rc=0;
	goto finally;	
exception:
	if (msg) 
	{
		evcpe_msg_free(msg);

	}	
	//printf("evcpe_create_inform 6\n");
finally:
	//printf("evcpe_create_inform 7\n");
	return rc;
}

//监听本地参数值变化情况
int evcpe_listen_params_value_change(struct evcpe *cpe)
{
	sk_tr069_params_t *tr069_param=sk_tr069_get_param_object();
	
	if(tr069_param->value_changed)
	{
		evcpe_add_to_event(E_TR069_VALUE_CHANGED,cpe);
		tr069_param->value_changed=0;
	}	
	return 0;
}


int evcpe_start_session(struct evcpe *cpe)
{
	int rc=1;
	char ipaddr[32]={0};
	int ulport=0;
	const char *hostname, *address;
	struct evcpe_msg *msg=NULL;
	u_short port;
	struct evhttp_connection *conn = NULL;

    evcpe_info(__func__ , "[evcpe_start_session] enter test1!\n");

	//kangzh,删除所有事件之后重新添加,测试看看
	evcpe_repo_del_event(cpe->repo, "1 BOOT");
	//evcpe_handle_del_event(cpe);
	
	evcpe_get_boot_status(cpe,cpe->event_flag);	


    if(sk_tr069_get_power_down_status())
	{
		return rc;
    }
	       
	evcpe_info(__func__, "starting session");
    
	conn = NULL;
	hostname = cpe->proxy_url ? cpe->proxy_url->host : cpe->acs_url->host;
	port = cpe->proxy_url ? cpe->proxy_url->port : cpe->acs_url->port;
	evcpe_info(__func__ , "[evcpe_start_session] url:%s port:%d\n",hostname,port);
	
	if (!(address = evcpe_dns_cache_get(&cpe->dns_cache, hostname))) //取得远程主机地址
	{
	
		//printf("[kangzh] evcpe_start_session\n");	
		evcpe_info(__func__, "hostname not resolved: %s", hostname);
		cpe->retry_count ++;
        #if 0
		if ((rc = evcpe_retry_session(cpe)))
			evcpe_error(__func__, "failed to schedule session retry");
        #else
        rc = 0;
        #endif
		goto finally;
	}


	
	evcpe_info(__func__ , "[evcpe_start_session]  begin connect 000\n");
	if (!(conn = evhttp_connection_new(address, port))) 	//连接远程主机指定端口
	{
		evcpe_error(__func__, "failed to create evhttp_connection");
		rc = ENOMEM;
		goto exception;
	}
 
    evcpe_info(__func__ , "[evcpe_start_session]  begin connect 1111\n");

    evhttp_connection_set_base(conn, cpe->evbase);
    
	evhttp_connection_set_timeout(conn, cpe->acs_timeout);

	//建立会话,并设置会话处理回调	
	evcpe_info(__func__ , "[evcpe_start_session]  begin connect 3333\n");
	if (!(cpe->session = evcpe_session_new(conn, cpe->acs_url, evcpe_session_message_cb, cpe)))
	{
		evcpe_error(__func__, "failed to create evcpe_session");
		rc = ENOMEM;
		goto exception;
	}

    cpe->session->retry_cnt = 0;

    cpe->session->conn = conn;
    
    evcpe_info(__func__ , "[evcpe_start_session]  begin connect 4444\n");
	//设置终止回调
	evcpe_session_set_close_cb(cpe->session, evcpe_session_terminate_cb, cpe);	
    
    evcpe_info(__func__ , "[evcpe_start_session]  begin connect 5555\n");
    
	rc = evcpe_create_inform(cpe);
	if(0 != rc)
	{
		goto finally;
	}
    
    evcpe_info(__func__ , "[evcpe_start_session]  begin connect 66666\n");

    //设置状态
    //date:2014-06-25
    //modify by lijingchao
    //在这里设置标志不太合适，因为服务器有可能没有启动，那么发送infrom包就没有成功。
    //sk_tr069_set_boot_status(1); //定期

	rc = 0;
	goto finally;

exception:

	if (conn) evhttp_connection_free(conn);
	if (cpe->session) 
	{
		evcpe_session_free(cpe->session);
		cpe->session = NULL;
	}

finally:
	return rc;
}

                    
//终止会话时须释放占用资源
void evcpe_session_terminate_cb(struct evcpe_session *session, int rc,
		void *arg)
{
    static int flag_acs_url = 1;

    static int flag_start_stun = 1;
    
	struct evcpe *cpe = arg;
    
    evcpe_info(__func__ , "[evcpe_session_terminate_cb]i'll terminate! rc = %d\n",rc);
	
	//清除日志发送状态 
	sk_tr069_reset_logmsg_flag();

	//清除valuechange发送状态
	sk_tr069_reset_valuechange_flag();
	

	if (rc) 
	{
		
		evcpe_error(__func__, "session failed: %d - %s", rc, strerror(rc));
		cpe->retry_count ++;
		
		#if 0		//kangzh,需要时再打开
		if (evcpe_retry_session(cpe)) 
		{
			evcpe_error(__func__, "failed to schedule session retry");
		}
		#endif	
            //add by lijingchao
            /*
            date:2014-06-25
            func:江苏移动要求
            3.	开放平台主备地址切换原则：
            开机首先连接主地址，如果主地址可用，则使用住地址，下次开机继续用主地址；
            如主地址不可用，切换备地址可用，则备地址切成主地址，主备地址互相切换，
            下次开机时使用备地址作为主地址；失败超时时间默认10S。
            以上原则适用于家庭业务开放平台主备地址和终端网管平台主备地址，分别独立进行主备切换。
            */
            do{
                if(1 == flag_acs_url || 1 == sk_get_value_reset_acs_url_flag())
                {
                
                    unsigned char tr069_acs_buf[512] = {0};
                    unsigned char tr069_bak_acs_buf[512] = {0};

                    evcpe_info(__func__ ,"flag_acs_url = 0 \n");
                        
                    sk_func_porting_params_get("tr069_acs" ,tr069_acs_buf , sizeof(tr069_acs_buf));
                    sk_func_porting_params_get("tr069_bak_acs" ,tr069_bak_acs_buf , sizeof(tr069_bak_acs_buf));
                    if('\0' == tr069_bak_acs_buf[0])
                    { 
                        evcpe_info(__func__ ,"tr069_acs_buf = %s\n",tr069_acs_buf);
                        sk_tr069_set_reauth_tr069_acs(tr069_acs_buf);
                    }
                    else
                    {
                        evcpe_info(__func__ , "tr069_bak_acs_buf = %s\n",tr069_bak_acs_buf);
                        sk_tr069_set_reauth_tr069_acs(tr069_bak_acs_buf);
                    }
                    //sk_func_porting_params_set("tr069_acs" , tr069_bak_acs_buf);
                    //sk_func_porting_params_set("tr069_bak_acs" , tr069_acs_buf);

   
                    sleep(5);
                    evcpe_worker_add_msg(EVCPE_WORKER_REAUTH); 
                    
                    sk_set_value_reset_acs_url_flag(2);
                    flag_acs_url = 2;
                }
                else if(2 == flag_acs_url || 2 == sk_get_value_reset_acs_url_flag())
                {
                    /*
                    unsigned char tr069_acs_buf[512] = {0};
                    unsigned char tr069_bak_acs_buf[512] = {0};

                  
                    sk_func_porting_params_get("tr069_acs" ,tr069_acs_buf , sizeof(tr069_acs_buf));
                    sk_func_porting_params_get("tr069_bak_acs" ,tr069_bak_acs_buf , sizeof(tr069_bak_acs_buf));
                   
                    sk_func_porting_params_set("tr069_acs" , tr069_bak_acs_buf);
                    sk_func_porting_params_set("tr069_bak_acs" , tr069_acs_buf);
                    */
                    LOGI("flag_acs_url = 1 \n");
                    
                    flag_acs_url = 0;
                    sk_set_value_reset_acs_url_flag(0);
                    
                }
            }while(0);
            //add end
 
	} 
	else 
	{
		cpe->retry_count = 0;

        
        flag_acs_url = 0;
        sk_set_value_reset_acs_url_flag(0);

        //改变第一次连接状态
	    sk_tr069_set_first_connect_status(0);
            
        sk_tr069_set_boot_status(1); //定期

        //modify by lijingchao
        //date:2014-12-29
        //comment:兼容中兴平台
        if(1 == flag_start_stun)
        {
            char *value = 0;
            int len = 0;
            flag_start_stun = 0;
         
            sk_tr069_get_address_type(NULL , &value , &len);
            if(NULL != value && len > 0)
            {
                if(!strcmp(value,"PPPoE"))
                {
                    evcpe_info(__func__ , "special for ZhongXing Platform PPPoE not STUN");
                    evcpe_worker_add_msg(EVCPE_WORKER_STUN_ONCMD);
    
                }
            }
            
        }
        //modify end
	}

    evhttp_connection_free(session->conn);
    session->conn = NULL ;
	evcpe_session_free(cpe->session);
	cpe->session = NULL;
}


//会话处理函数
void evcpe_session_message_cb(struct evcpe_session *session,
		enum evcpe_msg_type type, enum evcpe_method_type method_type,
				void *request, void *response, void *arg)
{
	int rc;
	struct evcpe *cpe = arg;

	evcpe_info(__func__ , "evcpe_session_message_cb enter!type:%d method_type:%d\n",type,method_type);
	
	evcpe_info(__func__, "handling %s %s message from ACS",
			evcpe_method_type_to_str(method_type), evcpe_msg_type_to_str(type));

	switch(type) {
	case EVCPE_MSG_REQUEST:
		if ((rc = evcpe_handle_request(cpe, session, method_type, request))) 
		{
			evcpe_error(__func__, "failed to handle request");
			//设置数据失败，暂时不关闭连接
			//goto close_session;
		}
		break;
	case EVCPE_MSG_RESPONSE:
		if ((rc = evcpe_handle_response(cpe, method_type, request, response))) 
		{
			evcpe_error(__func__, "failed to handle response");
			goto close_session;
		}
		break;
	case EVCPE_MSG_FAULT:
		evcpe_info(__func__ , "CWMP fault encountered: %d - %s",
				((struct evcpe_fault *)request)->code,
				((struct evcpe_fault *)request)->string);
		// TODO: notifies
		//goto close_session;
		break;
	default:
		evcpe_error(__func__, "unexpected message type: %d", type);
		rc = EINVAL;
		goto close_session;
	}
	return;

close_session:
//	printf("[evcpe_session_message_cb] evcpe_session_close enter!\n");
	evcpe_session_close(cpe->session, rc);
}

struct evcpe_fault g_faultarray[]=
{
	{EVCPE_CPE_INVALID_ARGUMENTS,	"Invalid arguments",		NULL},
	{EVCPE_CPE_INVALID_PARAM_NAME,	"Invalid parameter name",	NULL},
	{EVCPE_CPE_INVALID_PARAM_TYPE,	"Invalid parameter type",	NULL},
	{EVCPE_CPE_INVALID_PARAM_VALUE,	"Invalid parameter value",	NULL},
	{EVCPE_CPE_NON_WRITABLE_PARAM,	"Attempt to set a non-writable parameter",NULL}
					
};


char* evcpe_get_fault_msg(int code)
{
	int i,size;

	size=sizeof(g_faultarray)/sizeof(struct evcpe_fault);
	
	for(i=0;i<size;i++)
	{
		if(g_faultarray[i].code==code)
		{
			return g_faultarray[i].string;
		}
	}
	return NULL;
	
}




//会话请求
int evcpe_handle_request(struct evcpe *cpe, struct evcpe_session *session,
		enum evcpe_method_type method_type, void *request)
{
	int rc;
	struct evcpe_msg *msg;
	struct evcpe_fault *fault;


	if (!(msg = evcpe_msg_new())) {
		evcpe_error(__func__, "failed to create evcpe_msg");
		rc = ENOMEM;
		goto finally;
	}
	msg->method_type = method_type;
	switch(method_type) 
	{
	case EVCPE_GET_RPC_METHODS:
		rc = evcpe_handle_get_rpc_methods(cpe, request, msg);
		break;
	case EVCPE_GET_PARAMETER_NAMES:
		rc = evcpe_handle_get_param_names(cpe, request, msg);
		break;
	case EVCPE_SET_PARAMETER_ATTRIBUTES:
		rc = evcpe_handle_set_param_attrs(cpe, request, msg);
		break;
	case EVCPE_GET_PARAMETER_ATTRIBUTES:
		rc = evcpe_handle_get_param_attrs(cpe, request, msg);
		break;
	case EVCPE_GET_PARAMETER_VALUES:
		rc = evcpe_handle_get_param_values(cpe, request, msg);
		break;
	case EVCPE_ADD_OBJECT:
		rc = evcpe_handle_add_object(cpe, request, msg);
		break;
    case EVCPE_DELETE_OBJECT:
        rc = evcpe_handle_delete_object(cpe, request, msg);
        break;
	case EVCPE_SET_PARAMETER_VALUES:
		rc = evcpe_handle_set_param_values(cpe, request, msg);
		break;

	case EVCPE_DOWNLOAD:
	{
		struct evcpe_download_response	*download_resp = NULL;
		
		if ((rc = evcpe_handle_download(cpe, request, msg)))
			goto finally;

		download_resp=(struct evcpe_download_response *)msg->data;
		//用户选择
		if(download_resp->status > 0)
		{
			 evcpe_worker_add_msg(EVCPE_WORKER_DOWNLOAD);	//待发送反馈消息后启动下载流程
		}	   
	}	
		break;
	case EVCPE_UPLOAD:
		if ((rc = evcpe_handle_upload(cpe, request, msg)))
			goto finally;
	    evcpe_worker_add_msg(EVCPE_WORKER_UPLOAD);		//待发送反馈消息后启动上传流程
		break;
        
	case EVCPE_REBOOT:
		if ((rc = evcpe_handle_reboot(cpe, request, msg)))
			goto finally;
   
		evcpe_worker_add_msg(EVCPE_WORKER_REBOOT);		//待发送反馈消息后启动重启流程
		
		break;

	case EVCPE_FACTORY_RESET:
		if ((rc = evcpe_handle_factory_reset(cpe, request, msg)))
			goto finally;

		evcpe_worker_add_msg(EVCPE_WORKER_FACTORY_RESET);	//待发送反馈消息后启动恢复出厂流程
		     
		break;

	default:
		evcpe_error(__func__, "unexpected method type: %s",
				evcpe_method_type_to_str(method_type));
		rc = EVCPE_CPE_METHOD_NOT_SUPPORTED;
		break;
	}
	if (rc)
	{
		char *msg_str=NULL;
		if (!(msg->data = fault = evcpe_fault_new()))
		{
			rc = ENOMEM;
			goto finally;
		}
		msg->type = EVCPE_MSG_FAULT;
		if (rc >= EVCPE_CPE_FAULT_MIN && rc <= EVCPE_CPE_FAULT_MAX)
			fault->code = rc;
		else
			fault->code = EVCPE_CPE_INTERNAL_ERROR;

		msg_str=evcpe_get_fault_msg(rc);
		if(msg_str)
		{
			strcpy(fault->string,msg_str);
		}
		else
			strcpy(fault->string,"other unkown error");
	
		
	}
	if ((rc = evcpe_session_send(session, msg)))
	{
		evcpe_error(__func__, "failed to send CWMP %s message ,rc = %d",
				evcpe_method_type_to_str(msg->method_type), rc);
		evcpe_msg_free(msg);
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}


//会话反馈
int evcpe_handle_response(struct evcpe *cpe,
		enum evcpe_method_type method_type, void *request, void *response)
{
	int rc;

	switch (method_type)
	{
	case EVCPE_INFORM:
		if ((rc = evcpe_handle_inform_response(cpe, request, response)))
			goto finally;
		break;
				
	case EVCPE_TRANSFER_COMPLETE:
		evcpe_info(__func__ , "[evcpe_handle_response] EVCPE_TRANSFER_COMPLETE!\n");
		break;
		
	default:
		evcpe_error(__func__, "unexpected method type: %d", method_type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}


//请求处理:设置支持的rpc方法
int evcpe_handle_get_rpc_methods(struct evcpe *cpe,
		struct evcpe_get_rpc_methods *req,
		struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_get_rpc_methods_response *method;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = method = evcpe_get_rpc_methods_response_new()))
	{
		evcpe_error(__func__, "failed to create "
				"evcpe_get_rpc_methods_response");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "GetRPCMethods")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "GetParameterNames")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "GetParameterValues")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "SetParameterValues")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "GetParameterAttributes")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "SetParameterAttributes")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "AddObject")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "DeleteObject")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "Download")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "Upload")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "Reboot")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "FactoryReset")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "GetQueuedTransfers")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	if ((rc = evcpe_method_list_add_method(&method->method_list,
										   "ScheduleInform")))
	{
		evcpe_error(__func__, "failed to add method");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}


//请求处理:读取参数名称
int evcpe_handle_get_param_names(struct evcpe *cpe,
		struct evcpe_get_param_names *req,
		struct evcpe_msg *msg)
{
	int rc=0;
	struct evcpe_get_param_names_response *resp;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_get_param_names_response_new()))
	{
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_names_response");
		rc = ENOMEM;
		goto finally;
	}
	if ((rc = evcpe_repo_to_param_info_list(cpe->repo,
											req->parameter_path, &resp->parameter_list, req->next_level)))
	{
		evcpe_error(__func__, "failed to get param names: %s",
				req->parameter_path);
		evcpe_get_param_names_response_free(resp);
		goto finally;
	}


finally:
	return rc;
}
//请求处理:对getparameter参数进行处理，默认从模板文件的getter方法和default值读取参数值
int evcpe_handle_get_param_values(struct evcpe *cpe,
		struct evcpe_get_param_values *req,
		struct evcpe_msg *msg)
{
	int rc=0;
	struct evcpe_param_name *param;
	struct evcpe_get_param_values_response *resp;

	msg->type = EVCPE_MSG_RESPONSE;
    
	if (!(msg->data = resp = evcpe_get_param_values_response_new()))
	{
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_values_response");
		rc = ENOMEM;
		goto finally;
	}
	
	TAILQ_FOREACH(param, &req->parameter_names.head, entry)
	{
		
		if ((rc = evcpe_repo_to_param_value_list(cpe->repo,
				  param->name, &resp->parameter_list)))
		{
			evcpe_error(__func__, "failed to get param values: %s",
					param->name);
			evcpe_get_param_values_response_free(resp);
			goto finally;
		}
	}

finally:
	return rc;
}


//请求处理:获取参数属性
int evcpe_handle_set_param_attrs(struct evcpe *cpe,
		struct evcpe_get_param_attrs *req,
		struct evcpe_msg *msg)
{
	int rc=0;
	struct evcpe_param_name *param;
	struct evcpe_set_param_attrs_response *resp;
	
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_set_param_attrs_response_new()))
	{
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_attrs_response");
		rc = ENOMEM;
		goto finally;
	}
/*	
	TAILQ_FOREACH(param, &req->parameter_names.head, entry)
	{
		printf("param->name=%s\n",param->name);
		
		if ((rc = evcpe_repo_to_param_attr_list(cpe->repo,
												param->name, &resp->parameter_list)))
		{
			evcpe_error(__func__, "failed to get param values: %s",
					param->name);
			evcpe_set_param_attrs_response_free(resp);
			goto finally;
		}
	}
*/

finally:
	return rc;
}


//请求处理:获取参数属性
int evcpe_handle_get_param_attrs(struct evcpe *cpe,
		struct evcpe_get_param_attrs *req,
		struct evcpe_msg *msg)
{
	int rc=0;
	struct evcpe_param_name *param;
	struct evcpe_get_param_attrs_response *resp;

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_get_param_attrs_response_new()))
	{
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_attrs_response");
		rc = ENOMEM;
		goto finally;
	}
	
	TAILQ_FOREACH(param, &req->parameter_names.head, entry)
	{
		//printf("[evcpe_handle_get_param_attrs] param->name=%s\n",param->name);
		if ((rc = evcpe_repo_to_param_attr_list(cpe->repo,
												param->name, &resp->parameter_list)))
		{
			evcpe_error(__func__, "failed to get param values: %s",
					param->name);
			evcpe_get_param_attrs_response_free(resp);
			goto finally;
		}
	}


finally:
	return rc;
}

//读取参数:获取对象
int evcpe_handle_add_object(struct evcpe *cpe,
		struct evcpe_add_object *req,
		struct evcpe_msg *msg)
{
	int rc=0;
	unsigned int index;
	struct evcpe_add_object_response *resp;

	if ((rc = evcpe_repo_add_obj(cpe->repo, req->object_name, &index)))
	{
		evcpe_error(__func__, "failed to add object: %s", req->object_name);
		goto finally;
	}

    //add by lijingchao
    {
        int add_index = index+1;
        sk_tr069_add_multiple_object_data(NULL , req->object_name , add_index); 
    }
    //end 
    
	// TODO: set ParameterKey
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_add_object_response_new()))
	{
		evcpe_error(__func__, "failed to create add_object_response");
		rc = ENOMEM;
		goto finally;
	}
	resp->instance_number = index;
	resp->status = 0;


finally:
	return rc;
}


int evcpe_handle_delete_object(struct evcpe *cpe,
		struct evcpe_delete_object *req,
		struct evcpe_msg *msg)
{
	int rc = 0;
	unsigned int index = 0;
	struct evcpe_delete_object_response *resp = NULL;

	if ((rc = evcpe_repo_del_obj(cpe->repo, req->object_name)))
	{
		evcpe_error(__func__, "failed to del object: %s", req->object_name);
		goto finally;
	}

    //add by lijingchao
    sk_tr069_delete_multiple_object_data(NULL , req->object_name); 
    //end 
    
	// TODO: set ParameterKey
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_delete_object_response_new()))
	{
		evcpe_error(__func__, "failed to create delete_object_response");
		rc = ENOMEM;
		goto finally;
	}
	resp->status = 0;


finally:
	return rc;
}


//根据用户选择结果返回download到ITMS,download响应的status=1
//表示文件并未下载或应用就关闭连接，否则启动下载进程
int evcpe_handle_download(struct evcpe *cpe,
							struct evcpe_download *req,
							struct evcpe_msg *msg)
{

	int 							rc=0;
	int								enable;
	time_t 							current;
	struct evcpe_download_response	*download_resp;
	

	evcpe_info(__func__ , "evcpe_handle_download_response enter!\n");

	enable=sk_tr069_enable_download();//为1则不允许下载

	//下载参数设置
	sk_tr069_set_download_param(req);
	
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = download_resp = evcpe_download_response_new()))
	{
		evcpe_error(__func__, "failed to create add_object_response");
		rc = ENOMEM;
		evcpe_msg_free(msg);
		goto finally;
	}
	//在这里完成download数据填充
	memcpy(download_resp->commandkey,req->commandkey,sizeof(download_resp->commandkey));
	
	//在这里保存commandkey到e2p中
	sk_tr069_set_commandkey(req->commandkey,strlen(req->commandkey));
	
	//根据用户反馈结果，设置状态
	download_resp->status=enable;
	//这里对协议有疑义，待解决后填充正确的值，先填0,
	current = time(NULL);
	strftime(download_resp->start_time, sizeof (download_resp->start_time), "%Y-%m-%dT%H:%M:%S", localtime(&current));

	

	
finally:
	return rc;
}


int evcpe_handle_upload(struct evcpe *cpe,
							struct evcpe_upload *req,
							struct evcpe_msg *msg)
{

	int 							rc=0;
	int								enable=1;
	time_t 							current = {0};
	struct evcpe_upload_response	*upload_resp = NULL;
	

    evcpe_info(__func__ , "evcpe_handle_upload enter!\n");

	//下载参数设置
	sk_tr069_set_upload_param(req);	

	msg->type = EVCPE_MSG_RESPONSE;
    
	if (!(msg->data = upload_resp = evcpe_upload_response_new()))
	{
		evcpe_error(__func__, "failed to create add_object_response");
		rc = ENOMEM;
		evcpe_msg_free(msg);
		goto finally;
	}
	//在这里完成upload数据填充

	
	//根据用户反馈结果，设置状态
	upload_resp->status = enable;
	
	current = time(NULL);
	strftime(upload_resp->start_time, sizeof (upload_resp->start_time), "%Y-%m-%dT%H:%M:%S", localtime(&current));
    
finally:
    evcpe_info(__func__ , "evcpe_handle_upload exit! rc = %d\n" , rc);
	return rc;
}



//在这里实现重新启动功能，并反馈ITMS结果,正常应该是先返回状态
//之后进入重新启动流程
int evcpe_handle_reboot(struct evcpe *cpe,
							struct evcpe_reboot *req,
							struct evcpe_msg *msg)
{
	int 							rc=0;

	struct evcpe_reboot_response	*reboot_resp;
	


	

	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = reboot_resp = evcpe_reboot_response_new()))
	{
		evcpe_error(__func__, "failed to create add_object_response");
		rc = ENOMEM;
		evcpe_msg_free(msg);
		goto finally;
	}
	//在这里完成reboot数据填充
	memcpy(reboot_resp->commandkey,req->command_key,sizeof(reboot_resp->commandkey));
	printf("[xdl] evcpe_handle_reboot_response enter!--commandkey:%s------\n",reboot_resp->commandkey);

	
finally:
	return rc;	
	

}

//在这里实现恢复出厂设置功能，并反馈ITMS结果,正常应该是先返回状态
//之后进入重新启动流程
int evcpe_handle_factory_reset(struct evcpe *cpe,
								struct evcpe_factory_reset *req,
								struct evcpe_msg *msg)
{

	int 							rc=0;
	struct evcpe_factory_reset_response	*factory_reset_resp;
	

//	printf("[kangzh] evcpe_handle_factory_reset_response enter!\n");


	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = factory_reset_resp = evcpe_reboot_response_new()))
	{
		evcpe_error(__func__, "failed to create add_object_response");
		rc = ENOMEM;
		evcpe_msg_free(msg);
		goto finally;
	}
	
	//在这里完成reset_response数据填充



finally:
	return rc;	

}

//设置参数值
int evcpe_handle_set_param_values(struct evcpe *cpe,
		struct evcpe_set_param_values *req,
		struct evcpe_msg *msg)
{
	int rc=0;
	struct evcpe_set_param_value *param;
	struct evcpe_set_param_values_response *resp;
    int     need_reset=0;
    
	TAILQ_FOREACH(param, &req->parameter_list.head, entry)
	{
		if(!strcmp(param->name,"Device.LAN.IPPingDiagnostics.Host"))	//
		{
            evcpe_worker_add_msg(EVCPE_WORKER_PING);
			
		}
		else if(!strcmp(param->name,"Device.LAN.TraceRouteDiagnostics.Host"))
		{
            evcpe_worker_add_msg(EVCPE_WORKER_TRACEROUTE);
		}
		else if(!strcmp(param->name,"Device.X_00E0FC.PlayDiagnostics.DiagnosticsState"))
		{
            evcpe_worker_add_msg(EVCPE_WORKER_PLAY);
		}		
	    else if(!strcmp(param->name,"Device.X_CTC_IPTV.ServiceInfo.UserID"))
	    {  //更新用户名
            //need_reset=1;
	    }
		else if(!strcmp(param->name,"Device.ManagementServer.STUNEnable"))
		{  
			evcpe_info(__func__ , "the STUNEnable changed\n");
            evcpe_worker_add_msg(EVCPE_WORKER_STUN_ONCMD);
		}
        else if(!strcmp(param->name,"Device.X_00E0FC.BandwidthDiagnostics.DiagnosticsState"))
        {
            //evcpe_worker_add_msg(EVCPE_WORKER_BANDWIDTH_DIAGNOSTICS);
        }
        else if(!strcmp(param->name,"Device.X_00E0FC.PacketCapture.IP"))
        {
            evcpe_worker_add_msg(EVCPE_WORKER_PACKET_CAPTURE);
        }
        else if(!strcmp(param->name,"Device.X_00E0FC.LogParaConfiguration.LogOutPutType"))
        {
            evcpe_worker_add_msg(EVCPE_WORKER_SYSLOG_UPLOAD);
   
        }
		if ((rc = evcpe_repo_set(cpe->repo, param->name,
								 param->data, param->len)))
		{
			evcpe_error(__func__, "failed to set param: %s", param->name);
			// TODO: error codes
			goto finally;
		}	

	}

	// TODO: set ParameterKey
	msg->type = EVCPE_MSG_RESPONSE;
	if (!(msg->data = resp = evcpe_set_param_values_response_new())) 
	{
		evcpe_error(__func__, "failed to create "
				"evcpe_get_param_values_response");
		rc = ENOMEM;
		goto finally;
	}
	resp->status = 0;

    if(need_reset)
    {
        evcpe_worker_add_msg(EVCPE_WORKER_REBOOT);
    }
finally:
	return rc;
}


int evcpe_handle_del_event(struct evcpe *cpe)
{
	evcpe_repo_del_event(cpe->repo, "0 BOOTSTRAP");
	evcpe_repo_del_event(cpe->repo, "1 BOOT");

	evcpe_repo_del_event(cpe->repo, "2 PERIODIC");
	evcpe_repo_del_event(cpe->repo, "4 VALUE CHANGE");

	evcpe_repo_del_event(cpe->repo, "6 CONNECTION REQUEST");
	evcpe_repo_del_event(cpe->repo, "7 TRANSFER COMPLETE");
	evcpe_repo_del_event(cpe->repo, "8 DIAGNOSTICS COMPLETE");
	evcpe_repo_del_event(cpe->repo, "9 REQUEST DOWNLOAD");

	evcpe_repo_del_event(cpe->repo, "10 AUTONOMOUS TRANSFER COMPLETE");
	evcpe_repo_del_event(cpe->repo, "M REBOOT");
	evcpe_repo_del_event(cpe->repo, "M Reboot");

	evcpe_repo_del_event(cpe->repo, "M DOWNLOAD");
	evcpe_repo_del_event(cpe->repo, "M UPLOAD");

	//evcpe_repo_del_event(cpe->repo, "M X_CTC_SHUT_DOWN");
	evcpe_repo_del_event(cpe->repo, "X CMCC Shutdown");

	evcpe_repo_del_event(cpe->repo, "M CTC LOG_PERIODIC");

    evcpe_repo_del_event(cpe->repo, "M TRANSFERCOMPLETE");

	return 0;
	
}




//对inform进行反馈
int evcpe_handle_inform_response(struct evcpe *cpe,
		struct evcpe_inform *req, struct evcpe_inform_response *resp)
{
	if (resp->max_envelopes != 1)
	{
		evcpe_error(__func__, "invalid max envelopes: %d", resp->max_envelopes);
		return EPROTO;
	}

	//kangzh:删除所有eventcode
	evcpe_handle_del_event(cpe);
	
	return 0;
}

int evcpe_dns_entry_resolve(struct evcpe_dns_entry *entry, const char *hostname)
{
	int rc;

	evcpe_debug(__func__, "resolving DNS: %s", hostname);
	if ((rc = evdns_resolve_ipv4(hostname, 0, evcpe_dns_cb, entry)))
		evcpe_error(__func__, "failed to resolve IPv4 address: %s",
				entry->name);
	return rc;
}


int evcpe_dns_add(struct evcpe *cpe, const char *hostname)
{
	int rc;
	struct evcpe_dns_entry *entry;
	struct in_addr addr;

	evcpe_debug(__func__, "adding hostname: %s", hostname);

	if ((rc = evcpe_dns_cache_add(&cpe->dns_cache, hostname, &entry)))
	{
		evcpe_error(__func__, "failed to create DNS entry: %s", hostname);
		goto finally;
	}
	if (inet_aton(hostname, &addr))
	{
		if (!(entry->address = strdup(hostname)))
		{
			evcpe_error(__func__, "failed to duplicate address");
			rc = ENOMEM;
			evcpe_dns_cache_remove(&cpe->dns_cache, entry);
			goto finally;
		}
	}
	else
	{
		if ((rc = evcpe_dns_entry_resolve(entry, entry->name)))
		{
			evcpe_error(__func__, "failed to resolve entry: %s", hostname);
			goto finally;
		}
		evtimer_set(&entry->ev, evcpe_dns_timer_cb, entry);
		event_base_set(cpe->evbase, &entry->ev);
	}
	rc = 0;

finally:
	return rc;
}

void evcpe_dns_timer_cb(int fd, short event, void *arg)
{
	if (evcpe_dns_entry_resolve(arg, ((struct evcpe_dns_entry *)arg)->name))
		evcpe_error(__func__, "failed to start DNS resolution: %s",
				((struct evcpe_dns_entry *)arg)->name);
}

void evcpe_dns_cb(int result, char type, int count, int ttl,
    void *addresses, void *arg)
{
	struct evcpe_dns_entry *entry = arg;
	const char *address;

	evcpe_debug(__func__, "starting DNS callback");

	switch (result)
	{
	case DNS_ERR_NONE:
		break;
  case DNS_ERR_TIMEOUT:
        evcpe_debug(__func__ ,"type =%d , count = %d , ttl =%d\n",type , count , ttl);
	case DNS_ERR_SERVERFAILED:
	case DNS_ERR_FORMAT:
	case DNS_ERR_TRUNCATED:
	case DNS_ERR_NOTEXIST:
	case DNS_ERR_NOTIMPL:
	case DNS_ERR_REFUSED:
	default:
		evcpe_error(__func__, "DNS resolution failed: %d", result);
		goto exception;
	}

	evcpe_debug(__func__, "type: %d, count: %d, ttl: %d: ", type, count, ttl);
	switch (type)
	{
	case DNS_IPv6_AAAA:
	{
		// TODO
		break;
	}
	case DNS_IPv4_A:
	{
		struct in_addr *in_addrs = addresses;
		/* a resolution that's not valid does not help */
		if (ttl < 0)
		{
			evcpe_error(__func__, "invalid DNS TTL: %d", ttl);
			goto exception;
		}
		if (count == 0)
		{
			evcpe_error(__func__, "zero DNS address count");
			goto exception;
		}
		else if (count == 1)
		{
			address = inet_ntoa(in_addrs[0]);
		}
		else
		{
			address = inet_ntoa(in_addrs[rand() % count]);
		}
		if (!(entry->address = strdup(address)))
		{
			evcpe_error(__func__, "failed to duplicate address string");
			goto exception;
		}
		evcpe_debug(__func__, "address resolved for entry \"%s\": %s",
				entry->name, address);
		entry->tv.tv_sec = ttl;
		break;
	}
	case DNS_PTR:
		/* may get at most one PTR */
		if (count != 1)
		{
			evcpe_error(__func__, "invalid PTR count: %d", count);
			goto exception;
		}
		address = *(char **)addresses;
		if (evcpe_dns_entry_resolve(entry, address))
		{
			evcpe_error(__func__, "failed to start DNS resolve: %s", address);
			goto exception;
		}
		break;
	default:
		evcpe_error(__func__, "unexpected type: %d", type);
		goto exception;
	}
	goto finally;

exception:
	entry->tv.tv_sec = 0;

finally:
	evcpe_info(__func__, "next DNS resolution in %ld seconds: %s",
				   entry->tv.tv_sec, entry->name);
	if (event_add(&entry->ev, &entry->tv))
	{
		evcpe_error(__func__ , "failed to schedule DNS resolution");
	}
}

int sk_set_acs_dns(struct evcpe *cpe,char * url)
{
		int rc;
		char szBuffer[1024];
		memset(szBuffer,0,sizeof(szBuffer));
		strcpy(szBuffer,url);
		evcpe_info(__func__ , "[sk_set_acs_dns] acs_url=%s   00000\n\n",url);
		if(cpe==NULL)
			return 0;
		evcpe_info(__func__ , "[sk_set_acs_dns] acs_url=%s   1111\n\n",url);

        if(cpe->acs_url)
        {
            evcpe_url_free(cpe->acs_url);
        }
        
        if (!(cpe->acs_url= evcpe_url_new())) 
		{
			evcpe_error(__func__ , "failed to create evcpe_url");
			rc = ENOMEM;
			goto finally;
		}	 
		
		if ((rc = evcpe_url_from_str(cpe->acs_url, szBuffer))) 
		{
			evcpe_error(__func__ , "failed to parse ACS URL: %s", szBuffer);
			goto finally;
		}
		evcpe_info(__func__ , "[sk_set_acs_dns] acs_url=%s\n\n",szBuffer);

		evcpe_info(__func__ , "[sk_set_acs_dns] evcpe_dns_add:%s\n",cpe->acs_url->host);
		if ((rc = evcpe_dns_add(cpe, cpe->acs_url->host))) 
		{
			evcpe_error(__func__ , "failed to resolve ACS hostname");
			goto finally;
		}
	finally:
		return 0;
}


//add by lijingchao

int evcpe_enable_periodic_inform(struct evcpe *cpe)
{
    int rc = -1;
    int periodic_tv = 0 ;
	char *value=NULL;
    int len = 0;

    if(NULL == cpe)
    {
        goto finally;
    }
    evcpe_info(__func__, "evcpe_enable_periodic_inform() call");
    sk_tr069_get_period_inform_interval(NULL,&value , &len);
	if(NULL != value)
    {		
        sscanf(value,"%d",&periodic_tv);
	}
    else
    {
        periodic_tv = TR069_INFORM_INTERVAL;
    }
    
	evtimer_set(&cpe->periodic_ev, evcpe_start_session_cb, cpe);
	if ((rc = event_base_set(cpe->evbase, &cpe->periodic_ev))) 
	{
		evcpe_error(__func__, "failed to set event base");
		goto finally;
	}
     
	evutil_timerclear(&cpe->periodic_tv);
	cpe->periodic_tv.tv_sec = periodic_tv;
	evcpe_info(__func__, "scheduling periodic inform in %ld second(s)",
			cpe->periodic_tv.tv_sec);
    
	if ((rc = event_add(&cpe->periodic_ev, &cpe->periodic_tv))) 
	{
		evcpe_error(__func__, "failed to schedule periodic inform");
		goto finally;
	}
    
finally:
    return rc;
}

void evcpe_disable_periodic_inform(struct evcpe *cpe)
{
    if(NULL == cpe)
    {
        return ;
    }
    evcpe_info(__func__, "evcpe_disable_periodic_inform() call"); 
    if (event_initialized(&cpe->periodic_ev) &&
			evtimer_pending(&cpe->periodic_ev, NULL)) {
		event_del(&cpe->periodic_ev);
	}
}

struct evcpe *get_cpe_object(void)
{
	evcpe_t * thiz = get_evcpe_object();
    if(NULL != thiz)
    {
        return thiz->cpe;
    }
    return NULL;
}


static struct evcpe_worker_msg *evcpe_worker_msg_new(void)
{
	struct evcpe_worker_msg *msg = NULL;

	if (!(msg = calloc(1, sizeof(struct evcpe_worker_msg)))) {
		evcpe_error(__func__, "failed to calloc evcpe_worker_msg");
		return NULL;
	}
	memset(msg,0,sizeof(struct evcpe_worker_msg));
    
	return msg;
}


static void evcpe_worker_msg_free(struct evcpe_worker_msg *msg)
{
	if (NULL == msg) 
    {
        return;
	}
    free(msg);
    msg = NULL;
}

const char *evcpe_worker_msg_type_to_str(enum evcpe_worker_type type)
{
	switch (type) {

        case EVCPE_WORKER_CONNECTION_REQUEST:
            return "6 CONNECTION REQUEST";
            
        case EVCPE_WORKER_VALUE_CHANGE:
            return "4 VALUE CHANGE";

        case EVCPE_WORKER_2_PERIODIC:
            return "2 PERIODIC";
            
        case EVCPE_WORKER_SHUTDOWN:
            return "shutdown";
            
		case EVCPE_WORKER_DOWNLOAD:   	    
			return "donwload";
            
		case EVCPE_WORKER_UPLOAD:	
            return "upload";
            
		case EVCPE_WORKER_REBOOT:		
			return "reboot";
            
		case EVCPE_WORKER_FACTORY_RESET:    
			return "factory reset";
            
		case EVCPE_WORKER_PING:			  
			return "ping";
            
		case EVCPE_WORKER_TRACEROUTE:
            return "traceroute";
            
		case EVCPE_WORKER_PLAY:          
			return "play control";
            
		case EVCPE_WORKER_STUN_ONCMD:   
            return "stun";
                
        case EVCPE_WORKER_REAUTH:                   
            return "reauth";

        case EVCPE_WORKER_AWAKEN:                   
            return "awaken";
                
         case EVCPE_WORKER_BANDWIDTH_DIAGNOSTICS: 
            return "bandwidth";

         case EVCPE_WORKER_PACKET_CAPTURE:      
            return "packet capture";
            
         case EVCPE_WORKER_SYSLOG_UPLOAD:
            return "syslog upload";
            
		default:
			return "unknown";
	  
    }
}

void evcpe_worker_msg_queue_init(void)
{
    struct evcpe *cpe = get_cpe_object();
    if(NULL == cpe)
        return;
    evcpe_debug(__func__, "init evcpe_worker_msg_queue");
    TAILQ_INIT(&cpe->worker_queue);
}

void evcpe_worker_msg_queue_clear(struct evcpe_worker_msg_queue *queue)
{
	struct evcpe_worker_msg *msg = NULL;
	evcpe_debug(__func__, "clearing evcpe_worker_msg_queue");
	while ((msg = TAILQ_FIRST(queue))) {
		TAILQ_REMOVE(queue, msg, entry);
		evcpe_worker_msg_free(msg);
	}
}


struct evcpe_worker_msg * evcpe_worker_msg_queue_find_msg(struct evcpe_worker_msg_queue *queue , enum evcpe_worker_type type)
{
    struct evcpe_worker_msg * msg = NULL;
    if(NULL == queue)
    {
        goto finally;
    }

	TAILQ_FOREACH(msg, queue , entry) {
		if (msg->worker_type == type) {
			return msg;
		}
	}
finally:
     return NULL;
}

evcpe_session_status_e evcpe_get_session_status(struct evcpe *cpe)
{
    evcpe_session_status_e  status = EVCPE_SESSION_STATUS_FREE; 
    if(NULL != cpe && NULL != cpe->session)
    {
        status = EVCPE_SESSION_STATUS_BUSY;
    }
    return status;
}


static void evcpe_worker_start_session_cb(int fd, short event, void *arg)
{
   struct evcpe *cpe = (struct evcpe *)arg;
   if(NULL  == cpe) 
   {
       goto finally;
   }
   
   evcpe_start_session(arg);
   
finally:
    return;
}

void evcpe_worker_start_session(struct evcpe *cpe)
{
    int rc = 0;
    
    if(NULL == cpe)
        goto finally;

    if (event_initialized(&cpe->session_ev) &&	!evtimer_pending(&cpe->session_ev, NULL)) 
	{
        evutil_timerclear(&cpe->session_tv); 
        cpe->session_tv.tv_sec = 0;
        cpe->session_tv.tv_usec = 0;
		if ((rc = event_add(&cpe->session_ev, &cpe->session_tv)))
		{
			LOGI("failed to schedule start session");
		}
	}
finally:
    return ;
}


void evcpe_worker_add_msg(enum evcpe_worker_type type)
{
   struct evcpe_worker_msg *msg = NULL;
   struct evcpe *cpe = get_cpe_object();
   if(NULL == cpe)
   {
        evcpe_error(__func__ , "my god , cpe is null\n");
        return;
   }
   
   if(type <= EVCPE_WORKER_NONE || type >= EVCPE_WORKER_MAX)
   {
        evcpe_error(__func__ , "evcpe_worker_add_msg failed! msg is %s \n" , evcpe_worker_msg_type_to_str(type));
   }

 
   msg = evcpe_worker_msg_new();
   if(NULL == msg)
   {
        evcpe_error(__func__,"evcpe_worker_msg_new failed!\n");
   }
   else
   {
       msg->worker_type = type;
       TAILQ_INSERT_TAIL(&cpe->worker_queue , msg, entry);
       evcpe_info(__func__,"add worker msg :%s\n",evcpe_worker_msg_type_to_str(type));
   }

}

void evcpe_worker_loop(int fd, short ev, void *arg)
{
    struct evcpe *cpe = arg;
    struct evcpe_worker_msg *msg = NULL;
    if(NULL == cpe)
        return;
    msg = TAILQ_FIRST(&cpe->worker_queue);
    
    if(EVCPE_SESSION_STATUS_BUSY == evcpe_get_session_status(cpe))
    {
        goto finally;
    }
    
    if(NULL != msg)
    {
        switch(msg->worker_type)
        {
            case EVCPE_WORKER_CONNECTION_REQUEST:   //6 CONNECTION REQUEST 事件
               	sk_tr069_start_6_connection_request(cpe);
                break;
                
            case EVCPE_WORKER_VALUE_CHANGE:     //4 VALUE CHANGE 事件
             	sk_tr069_start_4_value_change(cpe);
                break;
                
            case EVCPE_WORKER_2_PERIODIC:       //2 PERIODIC 事件
                sk_tr069_start_2_periodic(cpe);
                break;
                
            case EVCPE_WORKER_SHUTDOWN:         //启动网管注销流程
                sk_tr069_start_shutdown(cpe);
                break;
                
    		case EVCPE_WORKER_DOWNLOAD:   	    //启动下载流程
    			sk_tr069_start_download();
    			break;
                
    		case EVCPE_WORKER_UPLOAD:		    //启动上传流程	
    			sk_tr069_start_upload(cpe);
    			break;
                
    		case EVCPE_WORKER_REBOOT:		    //启动重启流程
    			sk_tr069_reboot();
    			break;
                
    		case EVCPE_WORKER_FACTORY_RESET:    //启动恢复出厂设置流程
    			sk_tr069_factoryreset();
    			break;
                
    		case EVCPE_WORKER_PING:			    //启动ping流程
    			sk_tr069_ping();
    			break;
                
    		case EVCPE_WORKER_TRACEROUTE:	    //启动traceroute流程
    			sk_tr069_traceroute();
    			break;
                
    		case EVCPE_WORKER_PLAY:             //启动检查播放流控流程
    			sk_tr069_play();
    			break;	

    		case EVCPE_WORKER_STUN_ONCMD:       //启动NAT穿越流程
    			sk_tr069_stun_on_cmd(cpe);
    			break;

            case EVCPE_WORKER_REAUTH:           //启动重新认证流程
            	sk_tr069_start_reauth(cpe);
                break;
                
            case EVCPE_WORKER_AWAKEN:           //待机唤醒
                sk_tr069_start_awaken(cpe);
                break;

             case EVCPE_WORKER_BANDWIDTH_DIAGNOSTICS: //启动带宽测试流程
                sk_tr069_bandwidth_diagnostics(cpe);
                break;

             case EVCPE_WORKER_PACKET_CAPTURE:      //启动远程抓包流程
                sk_tr069_packet_capture(cpe);
                break;
                
             case EVCPE_WORKER_SYSLOG_UPLOAD:
                sk_tr069_syslog_upload(cpe);
                break;
                
    		default:
                evcpe_error(__func__ , "unknown\n");
    			break;
    	  
        }
        TAILQ_REMOVE(&cpe->worker_queue, msg , entry);
        evcpe_worker_msg_free(msg);
    }
    else
    {
        //printf("time:%d\n",time(NULL));
    }
   
    
finally:
    return ;
}

void band_width_test_response_start()
{
	evcpe_worker_add_msg(EVCPE_WORKER_BANDWIDTH_DIAGNOSTICS);
}

//add end


