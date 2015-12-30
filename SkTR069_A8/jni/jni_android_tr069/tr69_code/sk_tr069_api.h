#ifndef LIB_SKTR069_H_
#define LIB_SKTR069_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SKTR069_CB_NM_ACCEPT_NOTIFY	1	//p1 (int type )
#define SKTR069_CB_NM_SET_PLAY_URL	2	//p1,p2 (const char * url)

enum {
	NOTIFY_SET_CURRENT_PLAY_URL = 1,//表示获取播放地址
	NOTIFY_SET_PACKAGE_LOST_RATE //表示获取丢包率
};

typedef void(*sk_tr069_callback_t)(void*o, int msg, int p1, const void*p2);

int sk_tr069_init();
int sk_tr069_set_callback(void*o, sk_tr069_callback_t cb);

int sk_tr069_append_log_info(int logtype, const char *value);
int sk_tr069_tell_auth_failed();
int sk_tr069_tell_joinchannel_failed(const char* value);
int sk_tr069_tell_current_play_url(const char* value);
int sk_tr069_tell_package_lost_rate(const char * rate);
int sk_tr069_tell_epg_server(const char* url);

#ifdef __cplusplus
}
#endif

#endif /* LIB_SKTR069_H_ */

