#ifndef EVCPE_DOWNLOAD_XML_H_
#define EVCPE_DOWNLOAD_XML_H_

#include <event.h>

#include "download.h"

int evcpe_download_response_to_xml(struct evcpe_download_response *method,struct evbuffer *buffer);

int evcpe_transfer_complete_to_xml(struct evcpe_transfer_complete *method,struct evbuffer *buffer);

#endif /* EVCPE_SET_PARAM_VALUES_XML_H_ */

