#include <stdlib.h> 
#include <string.h>
#include "download.h"
#include "log.h"

struct evcpe_download  *evcpe_download_new(void)
{
	struct evcpe_download *method;

	if (!(method = calloc(1, sizeof(struct evcpe_download)))) {
		evcpe_error(__func__, "failed to calloc evcpe_download");
		return NULL;
	}
	memset(method,0,sizeof(struct evcpe_download));
	return method;
}

void evcpe_download_free(struct evcpe_download *method)
{
	if (!method) return;	
	free(method);
}

struct evcpe_download_response *evcpe_download_response_new(void)
{
	struct evcpe_download_response *method;

	if (!(method = calloc(1, sizeof(struct evcpe_download_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_download_response");
		return NULL;
	}
	return method;
}

void evcpe_download_response_free(struct evcpe_download_response *method)
{
	if (!method) return;
	free(method);
}



struct evcpe_transfer_complete *evcpe_transfer_complete_new(void)
{
	struct evcpe_transfer_complete *method;

	if (!(method = calloc(1, sizeof(struct evcpe_transfer_complete)))) {
		evcpe_error(__func__, "failed to calloc evcpe_transfer_complete");
		return NULL;
	}
	return method;
}

void evcpe_transfer_complete_free(struct evcpe_transfer_complete *method)
{
	if (!method) return;
	free(method);
}



struct evcpe_transfer_complete_response *evcpe_transfer_complete_response_new(void)
{
	struct evcpe_transfer_complete_response *method;

	if (!(method = calloc(1, sizeof(struct evcpe_transfer_complete_response)))) {
		evcpe_error(__func__, "failed to calloc evcpe_transfer_complete_response");
		return NULL;
	}
	return method;
}

void evcpe_transfer_complete_response_free(struct evcpe_transfer_complete_response *method)
{
	if (!method) return;
	free(method);
}





