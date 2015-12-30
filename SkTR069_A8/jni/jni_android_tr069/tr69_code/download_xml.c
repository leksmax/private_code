#include <event.h>

#include "log.h"
#include "data_xml.h"
#include "cpe.h"
#include "download_xml.h"
#include "repo.h"
#include "data.h"


int evcpe_download_response_to_xml(struct evcpe_download_response *method,struct evbuffer *buffer)
{
	int rc=0;

	//20120703，新增下面一行
	if ((rc = evcpe_xml_add_string(buffer,"CommandKey",method->commandkey)))
		goto finally;
	
	if ((rc = evcpe_xml_add_int(buffer,"Status", method->status)))
		goto finally;
	
	if ((rc = evcpe_xml_add_datetime(buffer,"StartTime", method->start_time)))
		goto finally;

	if ((rc = evcpe_xml_add_datetime(buffer,"CompleteTime", method->complete_time)))
		goto finally;


finally:
		return rc;
	
}

int evcpe_transfer_complete_to_xml(struct evcpe_transfer_complete *method,struct evbuffer *buffer)
{
	int rc=0;
	char value[64]={0};
	sk_api_params_get("tr069_commandkey",value,64);
	evcpe_info(__func__,"---xdl---transfer_complete_commandkey:%s\n",value);
//	if ((rc = evcpe_xml_add_string(buffer,	"CommandKey", event->command_key)))
	if ((rc = evcpe_xml_add_string(buffer,	"CommandKey", value)))
		goto finally;


	if (rc = evcpe_add_buffer(buffer, "<FaultStruct>\n"))
		goto finally;

	if ((rc = evcpe_upgrade_fault_to_xml(&(method->fault_struct), buffer))) 
//	if ((rc = evcpe_fault_to_xml(&(method->fault_struct), buffer))) 
	{
		evcpe_error(__func__, "failed to marshal fault");
		goto finally;
	}

	if (rc = evcpe_add_buffer(buffer, "</FaultStruct>\n"))
		goto finally;
	
	if ((rc = evcpe_xml_add_datetime(buffer,"StartTime", method->start_time)))
		goto finally;


	if ((rc = evcpe_xml_add_datetime(buffer,"CompleteTime", method->complete_time)))
		goto finally;


finally:
	return rc;
}



