
#ifndef EVCPE_UPLOAD_XML_H_
#define EVCPE_UPLOAD_XML_H_

#include <event.h>

#include "upload.h"

int evcpe_upload_response_to_xml(
		struct evcpe_upload_response *method,
		struct evbuffer *buffer);

#endif /* EVCPE_SET_PARAM_VALUES_XML_H_ */

