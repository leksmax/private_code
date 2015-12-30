#ifndef LIB_SKTR069_H_
#define LIB_SKTR069_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef enum{ 
GET_PARAM, // TM ---> IPTV ??¨¨?2?¨ºy 
SET_PARAM, // TM ---> IPTV ¨¦¨¨??2?¨ºy 
REBOOT, // TM ---> IPTV 
FACTORYRESET, // TM ---> IPTV 
UPDATE, // TM ---> IPTV 
VALUE_CHANGE_REPORT, // TM <--- IPTV 
START, // TM <--- IPTV 
HEART_BEAT, // TM <---> IPTV 
GET_PARAM_REPLY, // TM <--- IPTV 
SET_PARAM_REPLY, // TM <--- IPTV 
REBOOT_REPLY, // TM <--- IPTV 
FACTORYRESET_REPLY, // TM <--- IPTV 
UPDATE_REPLY, // TM <--- IPTV 
VALUE_CHANGE_REPORT_REPLY, // TM <--- IPTV 
START_FINISH, // TM ---> IPTV 
HEAT_BEAT_REPLY, // TM <---> IPTV 
AUTHENTICATEINFOR,
RATELOSS,
MULTIFAILINFO,
VODFAILINFO,
START_PING, //TM<-->IPTV
START_TRACEROUTE, //TM<->IPTV
START_TCPDUMP,//TM<->IPTV
}op_type;
    typedef void(*sk_tr069_callback_t)(const void* data, int arg1, int arg2);
    int sk_binder_send(const void* data,int len);
    int sk_binder_set_cb(sk_tr069_callback_t cb);
    int sk_binder_cb(const void* data,int len);
    void sk_send_msg(char *users,int cmd,op_type type,char *msg,int len);
#ifdef __cplusplus
}
#endif

#endif /* LIB_SKTR069_H_ */
