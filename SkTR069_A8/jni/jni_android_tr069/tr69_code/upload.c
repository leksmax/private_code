#include <stdlib.h> 
#include <string.h>
#include "upload.h"
#include "log.h"

struct evcpe_upload  *evcpe_upload_new(void)
{
	struct evcpe_upload *method;

	if (!(method = calloc(1, sizeof(struct evcpe_upload)))) {
		evcpe_error(__func__, "failed to calloc evcpe_upload");
		return NULL;
	}
	memset(method,0,sizeof(struct evcpe_upload));
	return method;
}

void evcpe_upload_free(struct evcpe_upload *method)
{
	if (!method) return;	
	free(method);
}

struct evcpe_upload_response *evcpe_upload_response_new(void)
{
	struct evcpe_upload_response *method;

	if (!(method = calloc(1, sizeof(struct evcpe_upload_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_upload_response");
		return NULL;
	}
	return method;
}

void evcpe_upload_response_free(struct evcpe_upload_response *method)
{
	if (!method) return;
	free(method);
}

