#include <event.h>

#include "log.h"
#include "data_xml.h"
#include "cpe.h"
#include "upload_xml.h"
#include "repo.h"
#include "data.h"

int evcpe_upload_response_to_xml(
	struct evcpe_upload_response *method,
	struct evbuffer *buffer)
{
	int rc=0;

	if ((rc = evcpe_xml_add_int(buffer,"Status", method->status)))
		goto finally;
	
	if ((rc = evcpe_xml_add_datetime(buffer,"StartTime", method->start_time)))
		goto finally;

	if ((rc = evcpe_xml_add_datetime(buffer,"CompleteTime", method->complete_time)))
		goto finally;



finally:
		return rc;
	
}





