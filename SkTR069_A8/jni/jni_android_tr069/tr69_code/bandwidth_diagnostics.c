#include <stdlib.h> 
#include <string.h>
#include "bandwidth_diagnostics.h"




int sk_start_bandwidth_diagnostics(const evcpe_bandwidth_attribute_t * p_bandwidth_attribute , evcpe_bandwidth_results_t *p_bandwidth_results_t)
{

    if(!strcmp(p_bandwidth_attribute->download_url , "http"))
    {
        sk_func_porting_params_set("tr069_bandwidth_errorcode", "0" );
        sk_func_porting_params_set("tr069_bandwidth_maxspeed", "128");
        sk_func_porting_params_set("tr069_bandwidth_avgspeed", "125");
        sk_func_porting_params_set("tr069_bandwidth_minspeed", "100");
        sleep(5);
    }
    else if(!strcmp(p_bandwidth_attribute->download_url , "ftp"))
    {
        sk_func_porting_params_set("tr069_bandwidth_errorcode", "0" );
        sk_func_porting_params_set("tr069_bandwidth_maxspeed", "128");
        sk_func_porting_params_set("tr069_bandwidth_avgspeed", "125");
        sk_func_porting_params_set("tr069_bandwidth_minspeed", "100");
        sleep(5);
    }
    else if(!strcmp(p_bandwidth_attribute->download_url ,"sftp"))
    {
       sk_func_porting_params_set("tr069_bandwidth_errorcode", "0" );
       sk_func_porting_params_set("tr069_bandwidth_maxspeed", "128");
       sk_func_porting_params_set("tr069_bandwidth_avgspeed", "125");
       sk_func_porting_params_set("tr069_bandwidth_minspeed", "100");
        
       sk_func_porting_params_set("tr069_bandwidth_start", "");
    }
    else
    {
        sk_func_porting_params_set("tr069_bandwidth_errorcode", "102050" );
        sk_func_porting_params_set("tr069_bandwidth_maxspeed", "0");
        sk_func_porting_params_set("tr069_bandwidth_avgspeed", "0");
        sk_func_porting_params_set("tr069_bandwidth_minspeed", "0");
    }
	
	return 0;
}

