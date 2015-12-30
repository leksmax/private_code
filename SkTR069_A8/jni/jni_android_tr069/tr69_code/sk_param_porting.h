#ifndef __SK_PARAM_PORTING__
#define __SK_PARAM_PORTING__
#ifdef __cplusplus
extern "C" {
#endif
int sk_func_porting_params_get(const char *name,char *value,int size);
int sk_func_porting_params_set(const char *name,const char *value);
int sk_func_porting_reboot();
int sk_func_porting_factoryreset();
int sk_tr069_factoryreset();
int sk_tr069_porting_upgrade(const char* url);
int sk_tr069_porting_set_logserverurl(char *url);
int sk_tr069_porting_set_loguploadinterval(int interval);
int sk_tr069_porting_set_logrecordinterval(int interval);
#ifdef __cplusplus
}
#endif
#endif