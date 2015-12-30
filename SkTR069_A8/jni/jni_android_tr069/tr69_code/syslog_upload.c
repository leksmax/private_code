#include <stdlib.h> 
#include <string.h>
#include "syslog_upload.h"
#include "log.h"

int sk_start_syslog_diagnostics(sk_syslog_t *syslog)
{
    return sk_func_porting_params_set("tr069_syslog_start","");
}

