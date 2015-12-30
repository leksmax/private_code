// $Id: session.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <evhttp.h>

#include "log.h"
#include "util.h"
#include "msg_xml.h"
#include "method.h"

#include "http_author_util.h"
#include "http_digest_calc.h"

#include "session.h"
#include "download.h"



#define PRINTLOC printf("%s(%d)\n", __func__, __LINE__)

static void evcpe_session_http_close_cb(struct evhttp_connection *conn, void *arg);
static void evcpe_session_http_cb(struct evhttp_request *req, void *arg);
static int evcpe_session_send_do(struct evcpe_session *session,
		struct evcpe_msg *msg);

//关闭http连接,如果有关闭回调，则启用回调
void evcpe_session_close(struct evcpe_session *session, int code)
{
	evcpe_info(__func__, "closing CWMP session");
	evhttp_connection_set_closecb(session->conn, NULL, NULL);
	if (session->close_cb)  //这个函数:evcpe_session_terminate_cb
		(*session->close_cb)(session, code, session->close_cbarg);
}

//关闭http连接
void evcpe_session_http_close_cb(struct evhttp_connection *conn, void *arg)
{
	evcpe_info(__func__, "HTTP connection closed");
	evcpe_session_close(arg, ECONNRESET);
}


//会话处理
int evcpe_session_handle_incoming(struct evcpe_session *session,
		struct evbuffer *buffer)
{
	int rc;
	struct evcpe_msg *msg, *req;

	if (!(msg = evcpe_msg_new())) 
	{
		evcpe_error(__func__, "failed to create evcpe_msg");
		rc = ENOMEM;
		goto finally;
	}

		
	//处理请求
	if ((rc = evcpe_msg_from_xml(msg, buffer))) 
	{
		evcpe_error(__func__, "failed to unmarshal response");
		goto finally;
	}
	//printf("evcpe_session_handle_incoming 1\n\n");
	
	//该字段来自数据模板文件配置
	session->hold_requests = msg->hold_requests;
	//printf("evcpe_session_handle_incoming 2\n\n");
	//printf("evcpe_session_handle_incoming 3,msg->type=%d\n\n",msg->type);
	switch (msg->type) 
	{
	case EVCPE_MSG_REQUEST:	//回调处理,调用这个函数:evcpe_session_message_cb
		TAILQ_INSERT_TAIL(&session->req_in, msg, entry);
		//printf("evcpe_session_handle_incoming 4\n\n");
		(*session->cb)(session, msg->type, msg->method_type,
				msg->data, NULL, session->cbarg);
		break;
	case EVCPE_MSG_RESPONSE:
	case EVCPE_MSG_FAULT:
		//printf("EVCPE_MSG_FAULT-----------------------------------------\n\n");
		if (!(req = TAILQ_FIRST(&session->req_out))) 
		{
			evcpe_error(__func__, "no pending CPE request");
			rc = EPROTO;
			goto finally;
		}
		if (msg->type == EVCPE_MSG_RESPONSE
				&& msg->method_type != req->method_type) 
		{
			evcpe_error(__func__, "method of request/response doesn't match: "
					"%d != %d", req->method_type, msg->method_type);
			rc = EPROTO;
			goto finally;
		}
		//回调处理,调用这个函数:evcpe_session_message_cb
		(*session->cb)(session, msg->type, msg->method_type,
				req, msg->data, session->cbarg);
		TAILQ_REMOVE(&session->req_out, req, entry);
		evcpe_msg_free(req);
		evcpe_msg_free(msg);
		break;
	default:
		evcpe_error(__func__, "unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

//http反馈
void evcpe_session_http_cb(struct evhttp_request *http_req, void *arg) 
{
	int rc = 0;
	const char *cookies = NULL;
	struct evcpe_msg *msg = NULL ;
	struct evcpe_session *session = arg;
	evcpe_t *this = get_evcpe_object();
   // printf("[tr069] evcpe_session_http_cb enter!\n\n");

	
	if (http_req == NULL 
        || 0 == http_req->response_code 
        || 404 == http_req->response_code 
        || 408 == http_req->response_code)
	{
		evcpe_info(__func__, "session timed out");
		rc = ETIMEDOUT;
		goto close;
	}

    evcpe_info(__func__, "[cpecms][cpe <== cms]HTTP response code: %d", http_req->response_code);
    evcpe_info(__func__, "[cpecms][cpe <== cms] HTTP response content: \n%.*s",
			EVBUFFER_LENGTH(http_req->input_buffer),
			EVBUFFER_DATA(http_req->input_buffer));
    
  	if((http_req->response_code == 401) && (session->retry_cnt < 5 ))  //应付浙江需要认证的需求
	{
		//evcpe_error(__func__, "HTTP response_code=%d: %s\n\n",http_req->response_code,evhttp_find_header(http_req->input_headers, "WWW-Authenticate"));

       

        char *authentication=NULL;
        
		if ((cookies = evhttp_find_header(http_req->input_headers, "Set-Cookie"))
			&& (rc = evcpe_cookies_set_from_header(&session->cookies, cookies))) 
    	{
    		evcpe_error(__func__ , "failed to set cookies: %s", cookies);
    		//goto close;
    	}

		do
		{
			
			authentication=evhttp_find_header(http_req->input_headers, "WWW-Authenticate");

			if(authentication==NULL)
				break;
			
			if(session->authentication!=NULL)
			{
				free(session->authentication);
				session->authentication=NULL;
			}
			session->authentication=strdup(authentication);
			
			session->need_digest=1;
			session->retry_cnt++;
			
		}
		while(0);
			
		
		//sk_tr069_print_time("evcpe_session_http_cb");
		#if 0
		evcpe_create_inform(this->cpe);
        #else
        msg	= TAILQ_FIRST(&session->req_out);
        if(EVCPE_INFORM != msg->method_type)
        {
            evcpe_error(__func__, "my God , recv 401 ,can not find req_out msg!\n");
            evcpe_create_inform(this->cpe);
        }
        else
        {
            if ((rc = evcpe_session_send_do_for_auth(session, msg))) 
        	{
        		evcpe_error(__func__, "failed to send first request");
        		goto close;
        	}
        }
        #endif
		return;
		//goto close;
	}
 
	session->retry_cnt=0;
	
	if ((cookies = evhttp_find_header(http_req->input_headers, "Set-Cookie"))
			&& (rc = evcpe_cookies_set_from_header(&session->cookies, cookies))) 
	{
		evcpe_error(__func__, "failed to set cookies: %s", cookies);
		goto close;
	}

	
	//如果反馈有数据，则对反馈数据处理
	if (EVBUFFER_LENGTH(http_req->input_buffer) &&
		(rc = evcpe_session_handle_incoming(session, http_req->input_buffer))) 
	{
	
		evcpe_error(__func__, "failed to handle incoming data");
		goto close;
	}


	if ((msg = TAILQ_FIRST(&session->res_pending))) //如果队列中有response数据等待，则发送
	{
        evcpe_debug(__func__ , "[tr069] evcpe_session_http_cb 1!\n\n");
		if ((rc = evcpe_session_send_do(session, msg))) 
		{
			evcpe_error(__func__, "failed to response ACS request");
			goto close;
		}
		TAILQ_REMOVE(&session->res_pending, msg, entry);
		evcpe_msg_free(msg);
	} 
	else if (session->hold_requests)    //该字段来自数据模板文件配置
	{
        evcpe_debug(__func__ , "[tr069] evcpe_session_http_cb 2!\n\n"); 
		if ((rc = evcpe_session_send_do(session, NULL)))    //发送空包
		{
			evcpe_error(__func__, "failed to send empty HTTP request");
			goto close;
		}
	} 
	else if ((msg = TAILQ_FIRST(&session->req_pending))) //如果队列中有request数据等待，则发送
	{ 
        evcpe_debug(__func__ , "[tr069] evcpe_session_http_cb 3!\n\n");
		if ((rc = evcpe_session_send_do(session, msg))) 
		{
			evcpe_error(__func__, "failed to send CPE request");
			goto close;
		}
		TAILQ_REMOVE(&session->req_pending, msg, entry);
		TAILQ_INSERT_TAIL(&session->req_out, msg, entry);
	} 
	else if (!EVBUFFER_LENGTH(http_req->input_buffer)) //接收到空包，则关掉会话
	{
		//printf("[tr069] evcpe_session_http_cb 4!\n\n"); 
        //add by lijingchao 2014-02-18
        rc = 0;
        //end add
		evcpe_debug(__func__ , "[tr069] evcpe_session_http_cb 4!\n\n");		 
		//当接收到空包时，主动关闭会话
		evcpe_info(__func__, "session termination criteria are met!\n\n");
		goto close;
	} 
	else 							//否则发送一个空包给server
	{  
		
		printf("[evcpe_session_http_cb] --------------transfer_flag:%d---------------0!\n\n",this->cpe->transfer_flag); 
		if(this->cpe->transfer_flag>0) //当前正处于下载状态
		{	
			struct evcpe_transfer_complete 	*method=NULL;
			time_t 							current;
			char							*cmd_key=NULL;
			int								length=0;
			//printf("[evcpe_session_http_cb] -----------------------------1!\n\n"); 
			evcpe_debug(__func__ , "[tr069] evcpe_session_http_cb 6!\n\n");
			if(msg!=NULL)
			{
				printf("[evcpe_session_http_cb] msg is not null!\n");
				goto close;
			}
			printf("[evcpe_session_http_cb] -----------------------------2!\n\n");
			
			if (!(msg = evcpe_msg_new()))  //创建消息体结构
			{
				evcpe_error(__func__, "failed to create evcpe_msg");
				rc = ENOMEM;
				
				goto close;
			}
			printf("[evcpe_session_http_cb] -----------------------------3!\n\n");
	
			msg->session = evcpe_ltoa(random());
			msg->type = EVCPE_MSG_REQUEST;
			msg->method_type = EVCPE_TRANSFER_COMPLETE;
			
			if (!(method = calloc(1, sizeof(struct evcpe_transfer_complete)))) 
			{
				evcpe_error(__func__, "failed to calloc evcpe_inform");
				free(msg);
				goto close;
			}
			
			memset(method,0,sizeof(struct evcpe_transfer_complete));
			msg->data = method;

			//sk_tr069_get_commandkey(&method->command_key,sizeof(method->command_key));
			sk_tr069_get_commandkey(method->command_key,sizeof(method->command_key));
			
			
			//strcpy(transform_complete->start_time,"0");
		   
			current = time(NULL);
			strftime(method->complete_time, sizeof (method->complete_time), "%Y-%m-%dT%H:%M:%S", localtime(&current));

			current-=300;
			strftime(method->start_time, sizeof (method->complete_time), "%Y-%m-%dT%H:%M:%S", localtime(&current));

			method->fault_struct.code=0;
			strcpy(method->fault_struct.string,"It is right!");
			
			//printf("[evcpe_session_http_cb] -----------------------------4!\n\n");
			if ((rc = evcpe_session_send_do(session, msg))) 
			{
                this->cpe->transfer_flag=0;
				evcpe_error(__func__, "failed to response ACS request");
				free(method);
				free(msg);
				goto close;
			}
			this->cpe->transfer_flag=0;
            TAILQ_INSERT_TAIL(&session->req_out, msg, entry);
			//printf("[evcpe_session_http_cb] -----------------------------5!\n\n");		
			
		}
		else  
		{
	        // printf("[tr069] evcpe_session_http_cb 5,session=0x%x!\n\n",session);
	        evcpe_debug(__func__ , "[tr069] evcpe_session_http_cb 5!\n\n");
			if ((rc = evcpe_session_send_do(session, NULL))) 
			{
				evcpe_error(__func__, "failed to send empty HTTP request");
				goto close;
			}
		}
		

	}

	return;

close:
   evcpe_info(__func__ , "[evcpe_session_http_cb]evcpe_session_close\n");
   evcpe_session_close(session, rc);
}

//建立新的会话，并将相关队列和资源初始化
struct evcpe_session *evcpe_session_new(struct evhttp_connection *conn,
		struct evcpe_url *acs, evcpe_session_cb cb, void *cbarg)
{
	struct evcpe_session *session;

	evcpe_debug(__func__, "constructing evcpe_session");

	if (!(session = calloc(1, sizeof(struct evcpe_session)))) {
		evcpe_error(__func__, "failed to calloc evcpe_session");
		return NULL;
	}
	
	memset(session,0,sizeof(struct evcpe_session));
	
	RB_INIT(&session->cookies);
	TAILQ_INIT(&session->req_in);
	TAILQ_INIT(&session->req_out);
	TAILQ_INIT(&session->req_pending);
	TAILQ_INIT(&session->res_pending);
	session->conn = conn;
	session->acs = acs;
	session->cb = cb;
	session->cbarg = cbarg;
	session->need_digest=0;

    session->data_out = NULL;
    session->datlen = 0;
	
	return session;
}

//释放会话资源
void evcpe_session_free(struct evcpe_session *session)
{
	evcpe_t *this=get_evcpe_object();
	if (!session) return;

	evcpe_debug(__func__, "destructing evcpe_session");

	evcpe_cookies_clear(&session->cookies);
	evcpe_msg_queue_clear(&session->req_in);
	evcpe_msg_queue_clear(&session->req_out);
	evcpe_msg_queue_clear(&session->req_pending);
	evcpe_msg_queue_clear(&session->res_pending);
	if (session->conn) {
		evhttp_connection_set_closecb(session->conn, NULL, NULL);
	}

	if(session->authentication)
	{
		free(session->authentication);
		session->authentication=NULL;
	}
    
    if(NULL != session->data_out)
    {    
        free(session->data_out);
        session->data_out = NULL;
    }
    
	
	free(session);
}

void evcpe_session_set_close_cb(struct evcpe_session *session,
		evcpe_session_close_cb close_cb, void *cbarg)
{
	session->close_cb = close_cb;
	session->close_cbarg = cbarg;
}

int evcpe_session_start(struct evcpe_session *session)
{
	int rc;
	struct evcpe_msg *req;

	if (!(req = TAILQ_FIRST(&session->req_pending))) 
	{
		evcpe_error(__func__, "no pending CPE request");
		rc = EINVAL;
		goto finally;
	}
	if (req->method_type != EVCPE_INFORM) 
	{
		evcpe_error(__func__, "first CPE request must be an inform");
		rc = EINVAL;
		goto finally;
	}
	req = TAILQ_FIRST(&session->req_pending);  //取得队列中的第一个排队包，发送出去，并将该等待从等待队列中删除
	if ((rc = evcpe_session_send_do(session, req))) 
	{
		evcpe_error(__func__, "failed to send first request");
		goto finally;
	}

	TAILQ_REMOVE(&session->req_pending, req, entry);
	TAILQ_INSERT_TAIL(&session->req_out, req, entry);
	rc = 0;

finally:
	return rc;
}

//实际将发送数据放入到队列中
int evcpe_session_send(struct evcpe_session *session, struct evcpe_msg *msg)
{
	int rc;
	struct evcpe_msg *req;

	if (!session || !msg) 
	{
		printf("[evcpe_session_send] arg is null,session=%x,msg=%x!\n",session,msg);
		return EINVAL;
	}
	switch (msg->type) 
	{
	case EVCPE_MSG_REQUEST:
		TAILQ_INSERT_TAIL(&session->req_pending, msg, entry);
		break;
        
	case EVCPE_MSG_RESPONSE:
	case EVCPE_MSG_FAULT:
		if (!(req = TAILQ_FIRST(&session->req_in))) 
		{
			evcpe_error(__func__, "no pending ACS request");
			rc = -1;
			goto finally;
		} 
		else if (req->method_type != msg->method_type) 
		{		
			evcpe_error(__func__, "method type mismatch: %s != %s",
					evcpe_method_type_to_str(req->method_type),
					evcpe_method_type_to_str(msg->method_type));
			rc = -1;
			goto finally;
		}
	
		if(req->session==NULL)
		{
			printf("[evcpe_session_send] req->session is null!\n");
		}
		else
		{
			msg->session = strdup(req->session);
			if (!msg->session) 
			{
				evcpe_error(__func__, "failed to duplicate session ID");
				rc = ENOMEM;
				goto finally;
			}
		} 
		TAILQ_INSERT_TAIL(&session->res_pending, msg, entry);
		TAILQ_REMOVE(&session->req_in, req, entry);
		evcpe_msg_free(req);
		break;
        
	default:
		evcpe_error(__func__, "unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
        
	}
	rc = 0;

finally:
	return rc;
}

static int evcpe_session_add_header(struct evkeyvalq *keyvalq,
		const char *key, const char *value)
{
	evcpe_debug(__func__, "adding HTTP header: %s => %s", key, value);
	return evhttp_add_header(keyvalq, key, value);
}

static char* evcpe_get_authorization(struct evcpe_session *session)
{
	HTTPAuth_Config config={0};
	char  method[20]="POST";
	char acsuser[64]={0};
	char acspwd[64]={0};
	char *value=NULL;
	int len=0;
	
	char *authorization=NULL;
	
	config.eAbility=HTTPDigest_Config_Algorithm_MD5;
	config.eAuthType=E_HTTPAUTH_TYPE_DIGEST;
	config.lNonceCount=1;
	config.pcMethod=method;
	
	sk_tr069_get_acsuser(NULL,&value,&len);
	strcpy(acsuser,value);
	
	config.pcUserName=acsuser;

	sk_tr069_get_acspass(NULL,&value,&len);
	strcpy(acspwd,value);
	
	config.pcPassword=acspwd;
	config.pcUri=session->acs->uri;
//	printf("session->acs->uri=%s\n",session->acs->uri);
	HTTPAuth_BuildAuthorizationHeader_sj(&config,session->authentication,&authorization);
	return authorization;
		
}





int evcpe_session_send_do(struct evcpe_session *session, struct evcpe_msg *msg)
{
	int rc, len;
	char buffer[513]={0};
	struct evhttp_request *req;
	struct evcpe_cookie *cookie;
    
    //printf("[tr069] evcpe_session_send_do enter!\n\n");
    
	if (msg)
		evcpe_info(__func__, "sending CWMP %s message in HTTP request",	evcpe_msg_type_to_str(msg->type));
	else
		evcpe_info(__func__, "sending empty HTTP request");
	//printf("[tr069] evcpe_session_send_do 1!\n\n");
	//设置接收数据回调
	if (!(req = evhttp_request_new(evcpe_session_http_cb, session))) 
	{
		evcpe_error(__func__, "failed to create evhttp_connection");
		rc = ENOMEM;
		goto finally;
	}
        
	//printf("[tr069] evcpe_session_send_do 2!\n\n");
	
	//生成xml数据
	if (msg && msg->data && (rc = evcpe_msg_to_xml(msg,req->output_buffer))) 
	{
		evcpe_error(__func__, "failed to create SOAP message");
		evhttp_request_free(req);
		goto finally;
	}

    //add by lijingchao
    if(NULL != msg && EVCPE_INFORM == msg->method_type)
    {
    
        if(NULL != session->data_out)
        {    
            free(session->data_out);
            session->data_out = NULL;
        }

        session->data_out = (char*)calloc( 1 , EVBUFFER_LENGTH(req->output_buffer));
        session->datlen = EVBUFFER_LENGTH(req->output_buffer);
        if(NULL != session->data_out)
        {
           rc  = evbuffer_copyout(req->output_buffer , session->data_out , session->datlen);
           if(rc != session->datlen)
           {
                free(session->data_out);
                session->data_out = NULL;  
                session->datlen = 0;
           }
        }
    }
    //debug end
    
	req->major = 1;
	req->minor = 1;

   //printf("[tr069] 3 session->acs=0x%x\n\n",session->acs);

    
	snprintf(buffer, sizeof(buffer), "%s:%d",session->acs->host, session->acs->port);
	if ((rc = evcpe_session_add_header(req->output_headers,"Host", buffer))) 
	{
		evcpe_error(__func__, "failed to add header: Host=\"%s\"", buffer);
		evhttp_request_free(req);
		goto finally;
	}

	
    //printf("[tr069] 4 session->acs=0x%x\n\n",session->acs);
	if (!RB_EMPTY(&session->cookies)) 
	{
		len = 0;
		RB_FOREACH(cookie, evcpe_cookies, &session->cookies) 
		{
			len += snprintf(buffer + len, sizeof(buffer) - len, "%s=%s; ",
					cookie->name, cookie->value);
		}
		if (len - 2 < sizeof(buffer))
		{
			buffer[len - 2] = '\0';
		}
        if ((rc = evcpe_session_add_header(req->output_headers,
				"Cookie", buffer))) 
		{
			evcpe_error(__func__, "failed to add header: Cookie=\"%s\"", buffer);
			evhttp_request_free(req);
			goto finally;
		}
	}

     //printf("[tr069] 5 session->acs=0x%x\n\n",session->acs);
    
	if (msg && msg->data && (rc = evcpe_session_add_header(req->output_headers,"SOAPAction", ""))) 
	{
		evcpe_error(__func__, "failed to add header: SOAPAction=\"\"");
		evhttp_request_free(req);
		goto finally;
	}
	//////////////////////////////////////////////////////////////////////////
	//在这里加入认证相关报文
	if(session->need_digest)
	{
		char *authorization=NULL;
		
		authorization=evcpe_get_authorization(session);
		if(authorization!=NULL)
		{
		//	printf("authorization=%s\n\n",authorization);
		
			if ((rc = evcpe_session_add_header(req->output_headers,	"Authorization", authorization))) 
			{
				evcpe_error(__func__, "failed to add header: ""Authorization=\"%s\"",authorization);
				evhttp_request_free(req);
				free(authorization);
				goto finally;
			}
		
			free(authorization);
		}
		else
			printf("authorization is null,rc=%d!\n\n",rc);
		
	}


	

	//////////////////////////////////////////////////////////////////////////
    // printf("[tr069] 6 session->acs=0x%x\n\n",session->acs);
	if ((rc = evcpe_session_add_header(req->output_headers,	"User-Agent", "skyworth-"EVCPE_VERSION))) 
	{
		evcpe_error(__func__, "failed to add header: "
				"User-Agent=\"skyworth-"EVCPE_VERSION"\"");
		evhttp_request_free(req);
		goto finally;
	}

     //printf("[tr069] 7 session->acs=0x%x\n\n",session->acs);
    
	if ((rc = evcpe_session_add_header(req->output_headers,	"Content-Type", "text/xml"))) 
	{
		evcpe_error(__func__, "failed to add header: Content-Type=text/xml");
		evhttp_request_free(req);
		goto finally;
	}
    
	
    evcpe_info(__func__, "making HTTP request");
    evcpe_info(__func__, "[cpecms][cpe ==> cms]HTTP request content:\n%.*s",
            EVBUFFER_LENGTH(req->output_buffer),
			EVBUFFER_DATA(req->output_buffer));
    
    rc = 0;
	if ( session->acs->uri != NULL )
		rc = evhttp_make_request(session->conn, req,EVHTTP_REQ_POST, session->acs->uri);
	
	if (rc ) 
	{
		evcpe_error(__func__, "failed to make request");
		evhttp_request_free(req);
		goto finally;
	}
     //printf("[tr069] 9 session->acs=0x%x\n\n",session->acs);
	//设置关闭会话回调,kangzh,屏蔽掉
	//evhttp_connection_set_closecb(session->conn, evcpe_session_http_close_cb,session);
	rc = 0;

finally:
	return rc;
}

int evcpe_session_send_do_for_auth(struct evcpe_session *session, struct evcpe_msg *msg)
{
	int rc, len;
	char buffer[513]={0};
	struct evhttp_request *req;
	struct evcpe_cookie *cookie;
    
	if (msg)
		evcpe_info(__func__, "sending CWMP %s message in HTTP request",	evcpe_msg_type_to_str(msg->type));
	else
		evcpe_info(__func__, "sending empty HTTP request");

	//设置接收数据回调
	if (!(req = evhttp_request_new(evcpe_session_http_cb, session))) 
	{
		evcpe_error(__func__, "failed to create evhttp_connection");
		rc = ENOMEM;
		goto finally;
	}
      
    if(NULL != session->data_out && NULL != msg && EVCPE_INFORM == msg->method_type)
    {
        //生成xml数据
    	if (msg && msg->data && (rc = evbuffer_prepend(req->output_buffer , session->data_out , session->datlen))) 
    	{
    		evcpe_error(__func__, "failed to create SOAP message");
    		evhttp_request_free(req);
    		goto finally;
    	}
    }
    else if (msg && msg->data && (rc = evcpe_msg_to_xml(msg,req->output_buffer))) 
	{
		evcpe_error(__func__, "failed to create SOAP message");
		evhttp_request_free(req);
		goto finally;
	}
    
    if(NULL != session->data_out)
    {    
        free(session->data_out);
        session->data_out = NULL;
    }
    
   
	req->major = 1;
	req->minor = 1;

  	snprintf(buffer, sizeof(buffer), "%s:%d",session->acs->host, session->acs->port);
	if ((rc = evcpe_session_add_header(req->output_headers,"Host", buffer))) 
	{
		evcpe_error(__func__, "failed to add header: Host=\"%s\"", buffer);
		evhttp_request_free(req);
		goto finally;
	}

	
	if (!RB_EMPTY(&session->cookies)) 
	{
		len = 0;
		RB_FOREACH(cookie, evcpe_cookies, &session->cookies) 
		{
			len += snprintf(buffer + len, sizeof(buffer) - len, "%s=%s; ",
					cookie->name, cookie->value);
		}
		if (len - 2 < sizeof(buffer))
		{
			buffer[len - 2] = '\0';
		}
        if ((rc = evcpe_session_add_header(req->output_headers,
				"Cookie", buffer))) 
		{
			evcpe_error(__func__, "failed to add header: Cookie=\"%s\"", buffer);
			evhttp_request_free(req);
			goto finally;
		}
	}
    
	if (msg && msg->data && (rc = evcpe_session_add_header(req->output_headers,"SOAPAction", ""))) 
	{
		evcpe_error(__func__, "failed to add header: SOAPAction=\"\"");
		evhttp_request_free(req);
		goto finally;
	}
	//////////////////////////////////////////////////////////////////////////
	//在这里加入认证相关报文
	if(session->need_digest)
	{
		char *authorization=NULL;
		
		authorization=evcpe_get_authorization(session);
		if(authorization!=NULL)
		{
		//	printf("authorization=%s\n\n",authorization);
		
			if ((rc = evcpe_session_add_header(req->output_headers,	"Authorization", authorization))) 
			{
				evcpe_error(__func__, "failed to add header: ""Authorization=\"%s\"",authorization);
				evhttp_request_free(req);
				free(authorization);
				goto finally;
			}
		
			free(authorization);
		}
		else
			printf("authorization is null,rc=%d!\n\n",rc);
		
	}

	if ((rc = evcpe_session_add_header(req->output_headers,	"User-Agent", "skyworth-"EVCPE_VERSION))) 
	{
		evcpe_error(__func__, "failed to add header: "
				"User-Agent=\"skyworth-"EVCPE_VERSION"\"");
		evhttp_request_free(req);
		goto finally;
	}

     //printf("[tr069] 7 session->acs=0x%x\n\n",session->acs);
    
	if ((rc = evcpe_session_add_header(req->output_headers,	"Content-Type", "text/xml"))) 
	{
		evcpe_error(__func__, "failed to add header: Content-Type=text/xml");
		evhttp_request_free(req);
		goto finally;
	}
    
	
    evcpe_info(__func__, "making HTTP request");


    //HEX_INFO("[cpe ==> cms]", EVBUFFER_DATA(req->output_buffer), EVBUFFER_LENGTH(req->output_buffer));
	//sk_tr069_print_time("evcpe_session_send_do");
	
	//printf("[tr069] 8 session->acs=0x%x,session->conn=0x%x\n\n",session->acs,session->conn);
	//发送请求
    
    evcpe_info(__func__, "[cpecms][cpe ==> cms]HTTP request content:\n%.*s",
            EVBUFFER_LENGTH(req->output_buffer),
			EVBUFFER_DATA(req->output_buffer));
    
    rc = 0;
	if ( session->acs->uri != NULL )
		rc = evhttp_make_request(session->conn, req,EVHTTP_REQ_POST, session->acs->uri);
	
	if (rc ) 
	{
		evcpe_error(__func__, "failed to make request");
		evhttp_request_free(req);
		goto finally;
	}
     //printf("[tr069] 9 session->acs=0x%x\n\n",session->acs);
	//设置关闭会话回调,kangzh,屏蔽掉
	//evhttp_connection_set_closecb(session->conn, evcpe_session_http_close_cb,session);
	rc = 0;

finally:
	return rc;
}

