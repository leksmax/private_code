
#ifndef EVCPE_REBOOT_XML_H_
#define EVCPE_REBOOT_XML_H_

#include <event.h>

#include "reboot.h"

int evcpe_reboot_response_to_xml(
		struct evcpe_reboot_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_SET_PARAM_VALUES_XML_H_ */

