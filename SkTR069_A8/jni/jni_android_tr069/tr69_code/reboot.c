#include <stdlib.h> 
#include <string.h>
#include "reboot.h"
#include "log.h"

struct evcpe_reboot  *evcpe_reboot_new(void)
{
	struct evcpe_reboot *method;

	if (!(method = calloc(1, sizeof(struct evcpe_reboot)))) {
		evcpe_error(__func__, "failed to calloc evcpe_reboot");
		return NULL;
	}
	memset(method,0,sizeof(struct evcpe_reboot));
	return method;
}

void evcpe_reboot_free(struct evcpe_reboot *method)
{
	if (!method) return;	
	free(method);
}

struct evcpe_reboot_response *evcpe_reboot_response_new(void)
{
	struct evcpe_reboot_response *method;

	if (!(method = calloc(1, sizeof(struct evcpe_reboot_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_reboot_response");
		return NULL;
	}
	return method;
}

void evcpe_reboot_response_free(struct evcpe_reboot_response *method)
{
	if (!method) return;
	free(method);
}
