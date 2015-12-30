// $Id: fault_xml.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "log.h"
#include "xml.h"
#include "sk_tr069.h"

#include "fault_xml.h"

int evcpe_upgrade_fault_to_xml(struct evcpe_fault *fault, struct evbuffer *buffer)
{
	int rc=0;
	int err_code = 0;
	char *s_code =  NULL;

	evcpe_debug(__func__, "marshaling fault");
	//s_code = sk_api_params_get_str("tr069_upgrade_faultcode");
	
	printf("---xdl-sss--tr069_upgrade_faultcode:%s\n",s_code);
	if(s_code != NULL)
	{
		err_code =  atoi(s_code);
	}
		printf("---xdl---tr069_upgrade_faultcode:%d\n",err_code);
	if ((rc = evcpe_xml_add_int(buffer, "FaultCode",err_code)))  //xdl 20120826	
//	if ((rc = evcpe_xml_add_int(buffer, "FaultCode",fault->code )))  //
		goto finally;
	
	
	if ((rc = evcpe_xml_add_string(buffer, "FaultString",fault->string )))  //
		goto finally;
	
finally:
	return rc;

}
int evcpe_fault_to_xml(struct evcpe_fault *fault, struct evbuffer *buffer)
{
	int rc = 0;

	evcpe_debug(__func__, "marshaling fault");
	
	if ((rc = evcpe_xml_add_int(buffer, "FaultCode",fault->code )))  //
		goto finally;
	
	
	if ((rc = evcpe_xml_add_string(buffer, "FaultString",fault->string )))  //
		goto finally;
	
finally:
	return rc;

}

