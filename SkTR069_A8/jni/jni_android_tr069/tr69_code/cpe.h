// $Id: cpe.h 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#ifndef EVCPE_CPE_H_
#define EVCPE_CPE_H_

//#include "handler.h"
#include "dns_cache.h"
#include "method.h"
#include "xml.h"
#include "repo.h"
#include "url.h"
#include "session.h"
#include "persister.h"
#include "evcpe-config.h"

#include "error.h"

#include <evcpe.h>

typedef void (*evcpe_request_cb)(struct evcpe_request *req, void *cbarg);

typedef void (*evcpe_error_cb)(struct evcpe *cpe,
		enum evcpe_error_type type, int code, const char *reason, void *cbarg);

typedef int (*evcpe_request_hook_cb)(enum evcpe_method_type type, void *data);

enum evcpe_auth_type 
{
	EVCPE_AUTH_NONE,
	EVCPE_AUTH_BASIC,
	EVCPE_AUTH_DIGEST
};


typedef enum evcpe_session_status_tag{
    EVCPE_SESSION_STATUS_FREE,
    EVCPE_SESSION_STATUS_BUSY 
}evcpe_session_status_e;



enum evcpe_worker_type
{
	EVCPE_WORKER_NONE,
      
    EVCPE_WORKER_CONNECTION_REQUEST = 11,//6 CONNECTION REQUEST 事件
    EVCPE_WORKER_VALUE_CHANGE,           //4 VALUE CHANGE 事件
    EVCPE_WORKER_2_PERIODIC,            //2 PERIODIC 事件
    
    EVCPE_WORKER_SHUTDOWN = 101,    //启动网管注销流程
	EVCPE_WORKER_DOWNLOAD ,	        //启动下载流程
	EVCPE_WORKER_UPLOAD,			//启动上传流程	
	EVCPE_WORKER_REBOOT,			//启动重启流程
	EVCPE_WORKER_FACTORY_RESET,		//启动恢复出厂设置流程
	EVCPE_WORKER_PING,				//启动ping流程
	EVCPE_WORKER_TRACEROUTE,		//启动traceroute流程	
	EVCPE_WORKER_PLAY,				//play检测
	EVCPE_WORKER_STUN_ONCMD,        //tr111
	EVCPE_WORKER_REAUTH,            //重新网管登陆认证
	EVCPE_WORKER_AWAKEN,            //待机唤醒
    EVCPE_WORKER_BANDWIDTH_DIAGNOSTICS , //宽带检测
    EVCPE_WORKER_PACKET_CAPTURE ,   //远程抓包
    EVCPE_WORKER_SYSLOG_UPLOAD,        //日志上报
    
    
	EVCPE_WORKER_MAX
};


struct evcpe_worker_msg {
    enum evcpe_worker_type worker_type;
	TAILQ_ENTRY(evcpe_worker_msg) entry;
};

TAILQ_HEAD(evcpe_worker_msg_queue, evcpe_worker_msg);


struct evcpe {
	struct event_base *evbase;

	evcpe_request_cb cb;
	evcpe_error_cb error_cb;
	void *cbarg;

	struct evcpe_dns_cache dns_cache;

	struct evcpe_repo *repo;

	enum evcpe_auth_type acs_auth;
	struct evcpe_url *acs_url;
	//sunjian:
	struct evcpe_url *acs_bakurl;
	
	char acs_username[64];
	char acs_password[64];
	unsigned int acs_timeout;

	struct evcpe_url *proxy_url;
	const char *proxy_username;
	const char *proxy_password;

	struct evcpe_url *creq_url;
	const char creq_username[64];
	const char creq_password[64];
	unsigned int creq_interval;
	time_t creq_last;

	struct evhttp *http;

	struct evcpe_session *session;

    //================add by skyworth start
	unsigned int 	retry_count;
	struct event 	retry_ev;
	struct timeval 	retry_tv;

    struct event 	acs_url_retry_ev;
	struct timeval 	acs_url_retry_tv;

	struct event 	periodic_ev;
	struct timeval 	periodic_tv;

    struct event 	session_ev;
	struct timeval 	session_tv;
	int				inform_times;				//上报次数
	int				event_flag;					//事件类型
	int 			transfer_flag;				//传输状态

    struct evcpe_worker_msg_queue worker_queue;
    struct event 	worker_loop_ev;
	struct timeval 	worker_loop_tv;
	//================add by skyworth end

};

struct evcpe *evcpe_new(struct event_base *evbase,
		evcpe_request_cb cb, evcpe_error_cb error_cb, void *cbarg);

void evcpe_free(struct evcpe *cpe);

int evcpe_set(struct evcpe *cpe,struct evcpe_repo *repo);

int evcpe_bind_http(struct evcpe *cpe, const char *address, u_short port,
		const char *prefix);

int evcpe_is_attached(struct evcpe *cpe);

int evcpe_make_request(struct evcpe *cpe, const char *acs_url,
		struct evcpe_request *req);

int evcpe_set_acs(struct evcpe *cpe, const char *address, u_short port,
		const char *uri);

int evcpe_set_auth(struct evcpe *cpe, enum evcpe_auth_type type,
		const char *username, const char *password);

int evcpe_start(struct evcpe *cpe);

int evcpe_start_session(struct evcpe *cpe);


int evcpe_enable_periodic_inform(struct evcpe *cpe);

void evcpe_disable_periodic_inform(struct evcpe *cpe);

struct evcpe *get_cpe_object(void);

evcpe_session_status_e evcpe_get_session_status(struct evcpe *cpe);

//add by lijingchao

const char *evcpe_worker_msg_type_to_str(enum evcpe_worker_type type);

void evcpe_worker_msg_queue_init(void);

void evcpe_worker_msg_queue_clear(struct evcpe_worker_msg_queue *queue);

struct evcpe_worker_msg * evcpe_worker_msg_queue_find_msg(struct evcpe_worker_msg_queue *queue , enum evcpe_worker_type type);

void evcpe_worker_add_msg(enum evcpe_worker_type type);

void evcpe_worker_loop(int fd, short ev, void *arg);

void evcpe_worker_start_session(struct evcpe *cpe);

//add end

#endif /* EVCPE_CPE_H_ */
