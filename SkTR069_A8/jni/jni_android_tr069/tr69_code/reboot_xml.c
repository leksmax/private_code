
#include <event.h>

#include "log.h"
#include "data_xml.h"
#include "cpe.h"
#include "reboot_xml.h"
#include "repo.h"
#include "data.h"

int evcpe_reboot_response_to_xml(
	struct evcpe_reboot_response *method,
	struct evbuffer *buffer)
{
	int rc=0;

	if ((rc = evcpe_xml_add_string(buffer,"CommandKey", method->commandkey)))
//	if ((rc = evcpe_xml_add_string(buffer,"CommandKey","540")))
		goto finally;
	

finally:
		return rc;
	
}

