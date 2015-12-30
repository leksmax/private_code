#ifndef SK_CMD_PARAM_H
#define SK_CMD_PARAM_H
#ifdef __cplusplus
extern "C"
{
#endif
int sk_api_params_get(const char *name, char *value, int size);
int sk_api_params_set(const char *name, const char *value);
int sk_api_params_get_params_print_string(char *message, int size);
int sk_api_params_clear(const char *key);
int sk_api_upgrade_check();
int sk_api_upgrade_check_url(const char* url);
int sk_api_sendbroadcast(const char* name, const char* param_name, const char* param_value);
int sk_factory_reset();
int sk_upgrade_start(const char *path);
char* sk_get_config_path();
#ifdef __cplusplus
}
#endif

#endif // SK_CMD_PARAM_H

