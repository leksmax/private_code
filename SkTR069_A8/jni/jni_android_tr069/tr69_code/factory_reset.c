#include <stdlib.h> 
#include <string.h>
#include "factory_reset.h"
#include "log.h"

struct evcpe_factory_reset  *evcpe_factory_reset_new(void)
{
	struct evcpe_factory_reset *method;

	if (!(method = calloc(1, sizeof(struct evcpe_factory_reset)))) {
		evcpe_error(__func__, "failed to calloc evcpe_factory_reset");
		return NULL;
	}
	memset(method,0,sizeof(struct evcpe_factory_reset));
	return method;
}

void evcpe_factory_reset_free(struct evcpe_factory_reset *method)
{
	if (!method) return;	
	free(method);
}

struct evcpe_factory_reset_response *evcpe_factory_reset_response_new(void)
{
	struct evcpe_factory_reset_response *method;

	if (!(method = calloc(1, sizeof(struct evcpe_factory_reset_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_factory_reset_response");
		return NULL;
	}
	return method;
}

void evcpe_factory_reset_response_free(struct evcpe_factory_reset_response *method)
{
	if (!method) return;
	free(method);
}
