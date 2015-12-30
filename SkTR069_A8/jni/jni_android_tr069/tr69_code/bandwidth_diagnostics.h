#ifndef  EVCPE_BANDWIDTH_DIAGNOSTICS_H_
#define  EVCPE_BANDWIDTH_DIAGNOSTICS_H_



typedef struct evcpe_bandwidth_attribute_s
{
	int   diagnostics_state;
    char  download_url[256];
    char  username[256];
    char  password[256];
}evcpe_bandwidth_attribute_t;



typedef struct evcpe_bandwidth_results_s
{
	int status;
	int error_code;			
    int max_speed;
    int avg_speed;
    int min_speed;
    
}evcpe_bandwidth_results_t;


int sk_start_bandwidth_diagnostics(const evcpe_bandwidth_attribute_t * p_bandwidth_attribute , evcpe_bandwidth_results_t *p_bandwidth_results_t);

#endif//EVCPE_BANDWIDTH_DIAGNOSTICS_H_
