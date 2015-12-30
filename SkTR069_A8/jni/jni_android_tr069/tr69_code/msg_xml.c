// $Id: msg_xml.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "util.h"
#include "minixml.h"
#include "fault_xml.h"
#include "inform_xml.h"
#include "get_rpc_methods_xml.h"
#include "get_param_names_xml.h"
#include "get_param_attrs.h"
#include "set_param_attrs.h"
#include "get_param_values_xml.h"
#include "set_param_values_xml.h"
#include "add_object_xml.h"
#include "delete_object_xml.h"
#include "download_xml.h"

#include "msg_xml.h"
#include "download.h"
#include "upload.h"


static int evcpe_msg_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_msg_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len);
static int evcpe_msg_xml_data_cb(void *data,
		const char *text, unsigned len);
static int evcpe_msg_xml_attr_cb(void *data,
		const char *ns, unsigned nslen,
		const char *name, unsigned name_len, const char *value, unsigned value_len);

int evcpe_msg_to_xml(struct evcpe_msg *msg, struct evbuffer *buffer)
{
	int rc;
	const char *method = evcpe_method_type_to_str(msg->method_type);

	evcpe_debug(__func__, "marshaling SOAP message");

	if ((rc = evcpe_add_buffer(buffer, "<?xml version=\"1.0\"?>\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Envelope "
			"xmlns:"EVCPE_SOAP_ENV_XMLNS
			"=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"xmlns:"EVCPE_SOAP_ENC_XMLNS
			"=\"http://schemas.xmlsoap.org/soap/encoding/\" "
			"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			"xmlns:"EVCPE_CWMP_XMLNS
			"=\"urn:dslforum-org:cwmp-%d-%d\">\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Header>\n"
			"<"EVCPE_CWMP_XMLNS":ID "
			EVCPE_SOAP_ENV_XMLNS":mustUnderstand=\"1\">%s"
			"</"EVCPE_CWMP_XMLNS":ID>\n"
			"</"EVCPE_SOAP_ENV_XMLNS":Header>\n"
			"<"EVCPE_SOAP_ENV_XMLNS":Body>\n",
			msg->major, msg->minor, msg->session))) {
		evcpe_error(__func__, "failed to append buffer");
		goto finally;
	}
	switch(msg->type) 
	{
	case EVCPE_MSG_REQUEST:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%s>\n", method))) 
		{
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		switch(msg->method_type) 
		{
		case EVCPE_INFORM:
			if ((rc = evcpe_inform_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal inform");
				goto finally;
			}
			break;
		case EVCPE_TRANSFER_COMPLETE:	//kangzh
			if ((rc = evcpe_transfer_complete_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal inform");
				goto finally;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected request type: %d", msg->method_type);
			rc = EINVAL;
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%s>\n", method))) 
		{
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		break;
	case EVCPE_MSG_RESPONSE:

		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_CWMP_XMLNS":%sResponse>\n", method)))
		{
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}

		switch(msg->method_type) 
		{
		case EVCPE_GET_RPC_METHODS:
			if ((rc = evcpe_get_rpc_methods_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal get_rpc_methods_response");
				goto finally;
			}
			break;
		case EVCPE_ADD_OBJECT:
			if ((rc = evcpe_add_object_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal add_object_response");
				goto finally;
			}
			break;
       	case EVCPE_DELETE_OBJECT:
			if ((rc = evcpe_delete_object_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal delete_object_response");
				goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_ATTRIBUTES:
			if ((rc = evcpe_get_param_attr_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal get_param_attr_response");
				goto finally;
			}				
			break;
		case EVCPE_GET_PARAMETER_VALUES:
			if ((rc = evcpe_get_param_values_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal get_param_values_response");
				goto finally;
			}
			break;
		case EVCPE_GET_PARAMETER_NAMES:
			if ((rc = evcpe_get_param_names_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal get_param_names_response");
				goto finally;
			}
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
		case EVCPE_SET_PARAMETER_VALUES:
			if ((rc = evcpe_set_param_values_response_to_xml(msg->data, buffer))) 
			{
				evcpe_error(__func__, "failed to marshal set_param_values_response");
				goto finally;
			}
			break;

		case EVCPE_DOWNLOAD:   //+ by aobai
			if ((rc = evcpe_download_response_to_xml(msg->data, buffer)))
			{
				evcpe_error(__func__, "failed to marshal download");
				goto finally;
			}
			break;
		case EVCPE_UPLOAD:		//kangzh
			if ((rc = evcpe_upload_response_to_xml(msg->data, buffer)))
			{
				evcpe_error(__func__, "failed to marshal upload");
				goto finally;
			}
			break;		
		case EVCPE_REBOOT:
			if ((rc = evcpe_reboot_response_to_xml(msg->data, buffer)))
			{
				evcpe_error(__func__, "failed to marshal reboot");
				goto finally;
			}

			
			break;
		case EVCPE_FACTORY_RESET:
			
			break;
		default:
			evcpe_error(__func__, "unexpected response type: %d", msg->method_type);
			rc = EINVAL;
			goto finally;
		}

		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":%sResponse>\n", method)))
		{
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		
		break;
	case EVCPE_MSG_FAULT:
		if ((rc = evcpe_add_buffer(buffer, "<"EVCPE_SOAP_ENV_XMLNS":Fault>\n"
				"<faultcode>Client</faultcode>\n"
				"<faultstring>CWMP fault</faultstring>\n"
				"<detail>\n"
				"<"EVCPE_CWMP_XMLNS":Fault>\n"))) 
		{
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		if ((rc = evcpe_fault_to_xml(msg->data, buffer))) 
		{
			evcpe_error(__func__, "failed to marshal fault");
			goto finally;
		}
		if ((rc = evcpe_add_buffer(buffer, "</"EVCPE_CWMP_XMLNS":Fault>\n"
				"</detail>\n"
				"</"EVCPE_SOAP_ENV_XMLNS":Fault>\n"))) 
		{
			evcpe_error(__func__, "failed to append buffer");
			goto finally;
		}
		break;
	default:
		evcpe_error(__func__, "unexpected message type: %d", msg->type);
		rc = EINVAL;
		goto finally;
	}
	if ((rc = evcpe_add_buffer(buffer,
			"</"EVCPE_SOAP_ENV_XMLNS":Body>\n</"EVCPE_SOAP_ENV_XMLNS":Envelope>\n"))) {
		evcpe_error(__func__, "failed to append buffer");
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

int evcpe_msg_from_xml(struct evcpe_msg *msg, struct evbuffer *buffer)
{
	int rc = 0;
	struct evcpe_msg_parser parser;
	struct evcpe_xml_element *elm;

	evcpe_debug(__func__, "unmarshaling SOAP message");

	if (!EVBUFFER_LENGTH(buffer)) return 0;

	parser.msg = msg;
	parser.xml.data = &parser;
	parser.xml.xmlstart = (const char *)EVBUFFER_DATA(buffer);
	parser.xml.xmlsize = EVBUFFER_LENGTH(buffer);
	parser.xml.starteltfunc = evcpe_msg_xml_elm_begin_cb;
	parser.xml.endeltfunc = evcpe_msg_xml_elm_end_cb;
	parser.xml.datafunc = evcpe_msg_xml_data_cb;   //数据分析回调
	parser.xml.attfunc = evcpe_msg_xml_attr_cb;
	RB_INIT(&parser.xmlns);
	SLIST_INIT(&parser.stack);
	if ((rc = parsexml(&parser.xml))) {
		evcpe_error(__func__, "failed to parse SOAP message: %d", rc);
		/******************************************************************
      	sunjian: 目前xml模板解析不了移动一些页面特殊处理
		*******************************************************************/
		rc = 0;
	}
	while((elm = evcpe_xml_stack_pop(&parser.stack))) {
		evcpe_error(__func__, "pending stack: %.*s", elm->len, elm->name);
		free(elm);
	}
	evcpe_xmlns_table_clear(&parser.xmlns);
	return rc;
}

int evcpe_msg_xml_elm_begin_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	const char *attr;
	unsigned attr_len;
	const char *urn = "urn:dslforum-org:cwmp-";
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *parent = evcpe_xml_stack_peek(&parser->stack);


	evcpe_trace(__func__, "element begin: %.*s (namespace: %.*s)",
			len, name, nslen, ns);

	if (parent && !parent->ns_declared) 
	{
		evcpe_error(__func__, "parent namespace not declared: %.*s:%.*s",
				parent->nslen, parent->ns, parent->len, parent->name);
		goto syntax_error;
	}
	
	if (parent && parent->nslen && !nslen) 
	{
		ns = parent->ns;
		nslen = parent->nslen;
	}
	if (!evcpe_strncmp("Envelope", name, len)) 
	{
		if (parent) 
		{
			evcpe_error(__func__, "parent element is not expected");
			goto syntax_error;
		}
	} 
	else if (!parent) 
	{
		evcpe_error(__func__, "parent element is expected");
		goto syntax_error;
	} 
	else if (!evcpe_strncmp("Header", name, len)) 
	{
		if (evcpe_strncmp("Envelope", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("ID", name, len)) 
	{
		if (evcpe_strncmp("Header", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
		if (evcpe_xmlns_table_get(&parser->xmlns, ns, nslen, &attr, &attr_len)) 
		{
			evcpe_error(__func__, "undefined XML namespace: %.*s", nslen, ns);
			return -1;
		}
		// TODO: get cwmp version
	} 
	else if (!evcpe_strncmp("HoldRequests", name, len)) 
	{
		if (evcpe_strncmp("Header", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("NoMoreRequests", name, len)) 
	{
		if (evcpe_strncmp("Header", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("Body", name, len)) 
	{
		if (evcpe_strncmp("Envelope", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("Fault", name, len)) 
	{
		if (!evcpe_strncmp("Body", parent->name, parent->len)) 
		{
			if (!(parser->msg->data = evcpe_fault_new()))
				return ENOMEM;
			parser->msg->type = EVCPE_MSG_FAULT;
		} 
		else if (evcpe_strncmp("detail", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("faultcode", name, len)) 
	{
		if (evcpe_strncmp("Fault", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("faultstring", name, len)) 
	{
		if (evcpe_strncmp("Fault", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("FaultCode", name, len)) 
	{
		if (evcpe_strncmp("Fault", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("FaultString", name, len)) 
	{
		if (evcpe_strncmp("Fault", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("detail", name, len)) 
	{
		if (evcpe_strncmp("Fault", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("GetRPCMethods", name, len)) 
	{
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_rpc_methods_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_RPC_METHODS;
	} 
    else if (parent && !evcpe_strncmp("GetRPCMethods", parent->name, parent->len)) 
    {
		evcpe_error(__func__, "unexpected child element");
		goto syntax_error;
	} 
    else if (!evcpe_strncmp("GetParameterNames", name, len)) 
    {
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_NAMES;
		if (!(parser->msg->data = evcpe_get_param_names_new()))
			return ENOMEM;
	} 
    else if (!evcpe_strncmp("ParameterPath", name, len)) 
	{
		if (evcpe_strncmp("GetParameterNames", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("NextLevel", name, len)) 
	{
		if (evcpe_strncmp("GetParameterNames", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("GetParameterValues", name, len)) 
    {
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_param_values_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_VALUES;
	} 
    else if (!evcpe_strncmp("GetParameterAttributes", name, len)) 
	{
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_GET_PARAMETER_ATTRIBUTES;
		if (!(parser->msg->data = evcpe_get_param_attrs_new()))
			return ENOMEM;
	} 
    else if (!evcpe_strncmp("ParameterNames", name, len)) 
	{
		if (evcpe_strncmp("GetParameterValues", parent->name, parent->len) &&
				evcpe_strncmp("GetParameterAttributes", parent->name, parent->len)) {
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("string", name, len)) 
    {
		if (evcpe_strncmp("ParameterNames", parent->name, parent->len) &&
				evcpe_strncmp("AccessList", parent->name, parent->len) &&
				evcpe_strncmp("MethodList", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("SetParameterValues", name, len)) 
	{
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_set_param_values_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_SET_PARAMETER_VALUES;
	} 
    else if (!evcpe_strncmp("SetParameterAttributes", name, len)) 
	{
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_set_param_attrs_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_SET_PARAMETER_ATTRIBUTES;
	} 
    else if (!evcpe_strncmp("ParameterList", name, len)) 
    {
		if (evcpe_strncmp("SetParameterValues", parent->name, parent->len) &&
				evcpe_strncmp("SetParameterAttributes", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("ParameterValueStruct", name, len)) 
    {
		if (evcpe_strncmp("ParameterList", parent->name, parent->len)) 
        {                  
			goto unexpected_parent;
		}
	} 
		
	else if (!evcpe_strncmp("SetParameterAttributesStruct", name, len)) 
	{
		if (evcpe_strncmp("ParameterList", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("Name", name, len)) 
	{
		if(evcpe_strncmp("ParameterValueStruct", parent->name, parent->len) &&
				evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len))
		{
			goto unexpected_parent;
		}
		
	} 
	else if (!evcpe_strncmp("Value", name, len)) 
	{
		if (evcpe_strncmp("ParameterValueStruct", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("NotificationChange", name, len)) 
	{
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("Notification", name, len)) 
	{
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("AccessListChange", name, len)) 
	{
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
	else if (!evcpe_strncmp("AccessList", name, len)) 
	{
		if (evcpe_strncmp("SetParameterAttributesStruct", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("GetRPCMethodsResponse", name, len)) 
    {
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_get_rpc_methods_response_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_RESPONSE;
		parser->msg->method_type = EVCPE_GET_RPC_METHODS;
	} 
    else if (!evcpe_strncmp("MethodList", name, len)) 
    {
		if (evcpe_strncmp("GetRPCMethodsResponse", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("InformResponse", name, len)) 
    {
  
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_inform_response_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_RESPONSE;
		parser->msg->method_type = EVCPE_INFORM;
	} 
    else if (!evcpe_strncmp("MaxEnvelopes", name, len)) 
    {
		if (evcpe_strncmp("InformResponse", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("AddObject", name, len)) 
    {
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_add_object_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_ADD_OBJECT;
	} 
    else if (!evcpe_strncmp("DeleteObject", name, len)) 
    {
		if (evcpe_strncmp("Body", parent->name, parent->len)) 
        {
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_add_object_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_DELETE_OBJECT;
	} 
    else if (!evcpe_strncmp("ObjectName", name, len)) 
    {
		if (evcpe_strncmp("AddObject", parent->name, parent->len) &&
				evcpe_strncmp("DeleteObject", parent->name, parent->len)) 
		{
			goto unexpected_parent;
		}
	} 
    else if (!evcpe_strncmp("ParameterKey", name, len)) 
    {
		
		if (evcpe_strncmp("AddObject", parent->name, parent->len)
			  && evcpe_strncmp("DeleteObject", parent->name, parent->len)
			&& evcpe_strncmp("SetParameterValues", parent->name, parent->len))  //+ by aobai
		{
			goto unexpected_parent;
		}
	}
	else  if (!evcpe_strncmp("Download", name, len)) //+ by aobai  receive download order
	{	
		if (evcpe_strncmp("Body", parent->name, parent->len))
		{
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_download_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_DOWNLOAD;

	}else if (!evcpe_strncmp("Reboot", name, len)) // 不需要返回 
	{
		if (evcpe_strncmp("Body", parent->name, parent->len))
		{
			goto unexpected_parent;
		}
		//no need return , use evcpe_upload_new at first       
		if (!(parser->msg->data = evcpe_reboot_new()))
			return ENOMEM;
		
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_REBOOT;
	
	}else if (!evcpe_strncmp("FactoryReset", name, len))
	{
		if (evcpe_strncmp("Body", parent->name, parent->len))
		{
			goto unexpected_parent;
		}
        	
		if (!(parser->msg->data = evcpe_factory_reset_new()))
			return ENOMEM;
		
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_FACTORY_RESET;
	}
	else if ( !evcpe_strncmp("Upload", name, len))  
	{
		if (evcpe_strncmp("Body", parent->name, parent->len))
		{
			goto unexpected_parent;
		}
		if (!(parser->msg->data = evcpe_upload_new()))
			return ENOMEM;
		parser->msg->type = EVCPE_MSG_REQUEST;
		parser->msg->method_type = EVCPE_UPLOAD;
	}
	else if ( !evcpe_strncmp("TransferCompleteResponse", name, len))  
	{
		evcpe_debug(__func__ , "TransferCompleteResponse incoming---------------\n");
		parser->msg->type = EVCPE_MSG_RESPONSE;
        parser->msg->method_type = EVCPE_TRANSFER_COMPLETE;
	}
	else   //+ by aobai
	{
		evcpe_error(__func__, "elem  don't deal with  : %.*s ", len, name );
	}

	if (!(parent = calloc(1, sizeof(struct evcpe_xml_element))))
	{
		evcpe_error(__func__, "failed to calloc evcpe_soap_element");
		return ENOMEM;
	}
	parent->ns = ns;
	parent->nslen = nslen;
	parent->name = name;
	parent->len = len;
	evcpe_xml_stack_put(&parser->stack, parent);
	if (nslen && evcpe_xmlns_table_find(&parser->xmlns, ns, nslen))
		parent->ns_declared = 1;

	return 0;
    
unexpected_parent:
	evcpe_error(__func__, "unexpected parent element");

syntax_error:
	evcpe_error(__func__, "syntax error");
	return EPROTO;
}

int evcpe_msg_xml_elm_end_cb(void *data,
		const char *ns, unsigned nslen, const char *name, unsigned len)
{
	int rc;
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *elm;
	struct evcpe_get_param_values *get_params;
	struct evcpe_set_param_values *set_params;
	struct evcpe_get_param_attrs *get_attrs;

	if (!(elm = evcpe_xml_stack_pop(&parser->stack))) return -1;

	evcpe_trace(__func__, "element end: %.*s (namespace: %.*s)",
			len, name, nslen, ns);

	if ((nslen && evcpe_strcmp(elm->ns, elm->nslen, ns, nslen)) ||
			evcpe_strcmp(elm->name, elm->len, name, len)) {
		evcpe_error(__func__, "element doesn't match start: %.*s:%.*s",
				nslen, ns, len, name);
		rc = EPROTO;
		goto finally;
	}

	if (!evcpe_strncmp("Header", name, len)) {
		if (!parser->msg->session)
		{
		printf("---evcpe_msg_xml_elm_end_cb---1---\n");
		//goto syntax_error;
		}
	}else if (!evcpe_strncmp("Body", name, len)) {
		if (!parser->msg->data)
		{
		printf("---evcpe_msg_xml_elm_end_cb---3---\n");
		//goto syntax_error;
		}
	} else if (!evcpe_strncmp("GetParameterValues", name, len)) {
		get_params = parser->msg->data;
		if (!evcpe_param_name_list_size(&get_params->parameter_names))
		{
		printf("---evcpe_msg_xml_elm_end_cb---4---\n");
		goto syntax_error;
		}
	} else if (!evcpe_strncmp("SetParameterValues", name, len)) {
		set_params = parser->msg->data;
		if (!evcpe_set_param_value_list_size(&set_params->parameter_list))
		{
		printf("---evcpe_msg_xml_elm_end_cb---5---\n");
		goto syntax_error;
		}
	} else if (!evcpe_strncmp("GetParameterAttributes", name, len)) {
		get_attrs = parser->msg->data;
		if (!evcpe_param_name_list_size(&get_attrs->parameter_names))
		{
		printf("---evcpe_msg_xml_elm_end_cb---6---\n");
		goto syntax_error;
		}
	}
	rc = 0;
	goto finally;

syntax_error:
	evcpe_error(__func__, "syntax error");
	rc = EPROTO;

finally:
	free(elm);
	return rc;
}

int evcpe_msg_xml_data_cb(void *data, const char *text, unsigned len)
{
	int rc;
	long val;
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *elm;
	struct evcpe_fault *fault;
	struct evcpe_get_param_names *get_names;
	struct evcpe_get_param_values *get_params;
	struct evcpe_set_param_values *set_params;
	struct evcpe_get_param_attrs *get_attrs;
	struct evcpe_set_param_attrs *set_attrs;
	struct evcpe_param_name *param_name;
	struct evcpe_set_param_value *param_value;
	struct evcpe_set_param_attr *param_attr;
	struct evcpe_access_list_item *item;
	struct evcpe_get_rpc_methods_response *get_methods_resp;
	struct evcpe_method_list_item *method_item;
	struct evcpe_inform_response *inform_resp;
	struct evcpe_add_object *add_obj;
	struct evcpe_delete_object *delete_obj;

	//+ by aobai
	struct evcpe_download * download_params;
	unsigned int  filesize , delayseconds;
	char *  tmp;
	int   host_len;

	struct evcpe_upload * upload_params;
	//+

	if (!(elm = evcpe_xml_stack_peek(&parser->stack))) return -1;

	evcpe_trace(__func__, "text: %.*s", len, text);

	if (!evcpe_strncmp("ID", elm->name, elm->len)) 
	{
		
		if (!(parser->msg->session = malloc(len + 1))) 
		{
			rc = ENOMEM;
			goto finally;
		}
		memcpy(parser->msg->session, text, len);
		parser->msg->session[len] = '\0';
		//printf("[tr069]ID=%s\n\n",parser->msg->session);
	} 
	else if (!evcpe_strncmp("HoldRequests", elm->name, elm->len))  //该字段来自数据模板文件配置
	{
		if (len == 1) 
		{
			if (text[0] == '0')
				parser->msg->hold_requests = 0;
			else if (text[0] == '1')
				parser->msg->hold_requests = 1;
			else
				goto syntax_error;
		} 
		else 
		{
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("NoMoreRequests", elm->name, elm->len)) 
	{
		if (len == 1) 
		{
			if (text[0] == '0')
				parser->msg->no_more_requests = 0;
			else if (text[0] == '1')
				parser->msg->no_more_requests = 1;
			else
				goto syntax_error;
		} 
		else 
		{
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("faultcode", elm->name, elm->len)) 
    {
	} 
    else if (!evcpe_strncmp("faultstring", elm->name, elm->len)) 
    {
		if (evcpe_strncmp("CWMP fault", text, len) && evcpe_strncmp("CWMP Fault", text, len))
		{
			evcpe_error(__func__ , "faultstring CWMP fault \n");
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("FaultCode", elm->name, elm->len)) 
    {
		fault = parser->msg->data;
		if ((rc = evcpe_atol(text, len, &val))) 
		{
			evcpe_error(__func__, "failed to convert to "
					"integer: %.*s", len, text);
			goto finally;
		}
		fault->code = val;
	} 
    else if (!evcpe_strncmp("FaultString", elm->name, elm->len)) 
    {
		fault = parser->msg->data;
		if (len >= sizeof(fault->string))
			return EOVERFLOW;
		memcpy(fault->string, text, len);
		fault->string[len] = '\0';
	} 
    else if (!evcpe_strncmp("string", elm->name, elm->len)) 
	{
		switch (parser->msg->method_type) 
		{
		case EVCPE_GET_RPC_METHODS:
			get_methods_resp = parser->msg->data;
			if ((rc = evcpe_method_list_add(&get_methods_resp->method_list,
					&method_item, text, len)))
				goto finally;
			break;
		case EVCPE_GET_PARAMETER_VALUES:
			get_params = parser->msg->data;
			if ((rc = evcpe_param_name_list_add(&get_params->parameter_names,
					&param_name, text, len)))
				goto finally;
			break;
		case EVCPE_GET_PARAMETER_ATTRIBUTES:
			get_attrs = parser->msg->data;
			if ((rc = evcpe_param_name_list_add(&get_attrs->parameter_names,
					&param_name, text, len)))
				goto finally;
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if ((rc = evcpe_access_list_add(&param_attr->access_list,
					&item, text, len)))
				goto finally;
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
        
	}
    else if (!evcpe_strncmp("ParameterPath", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) 
        {
		case EVCPE_GET_PARAMETER_NAMES:
			get_names = parser->msg->data;
			if (len >= sizeof(get_names->parameter_path))
				goto syntax_error;
			strncpy(get_names->parameter_path, text, len);
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			
			goto syntax_error;
		}
	} 
	else if (!evcpe_strncmp("NextLevel", elm->name, elm->len)) 
	{
	
		switch (parser->msg->method_type) 
		{
		case EVCPE_GET_PARAMETER_NAMES:
		{
			char buffer[20]={0};
			int length=sizeof(buffer);
			
			get_names = parser->msg->data;

			length=length>len?len:length-1;
			
			strncpy(buffer,text,length);
			
			//printf("NextLevel------------------value:%s,len=%d,length=%d\n",buffer,len,length);
			if(!strcmp(buffer,"true"))   //浙江电信该值返回true和false
				get_names->next_level=1;
			else if(!strcmp(buffer,"false"))
				get_names->next_level=0;
			else
			{
			  	if(rc = evcpe_atol(text, len, &val))
				{
					goto syntax_error;
				}
				get_names->next_level = val;
			}
			//get_names->next_level = 1;
		    //modify by lijingchao
		    //针对华为网管平台 处理
            if('\0' == get_names->parameter_path[0])
            {
                strcpy(get_names->parameter_path , "Device.");
            }

            evcpe_info(__func__, "get_names->next_level = %d , get_names->parameter_path = %s",get_names->next_level,get_names->parameter_path);
		}
		break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("Name", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) 
        {
		case EVCPE_SET_PARAMETER_VALUES:
			set_params = parser->msg->data;
			if ((rc = evcpe_set_param_value_list_add(&set_params->parameter_list,
					&param_value, text, len)))
				goto finally;
			parser->list_item = param_value;
			break;
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			set_attrs = parser->msg->data;
			if ((rc = evcpe_set_param_attr_list_add(&set_attrs->parameter_list,
					&param_attr, text, len)))
				goto finally;
			parser->list_item = param_attr;
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
    else if (!evcpe_strncmp("Value", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_VALUES:
			param_value = parser->list_item;
			if ((rc = evcpe_set_param_value_set(param_value, text, len)))
				goto finally;
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("NotificationChange", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) 
		{
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
		{
			char buffer[20]={0};	
			int length=sizeof(buffer);
			param_attr = parser->list_item;

			length=length>len?len:length-1;
			
			strncpy(buffer,text,length);
			
			//printf("NotificationChange------------------value:%s,len=%d,length=%d\n",buffer,len,length);
			if(!strcmp(buffer,"true"))   //浙江电信该值返回true和false
				param_attr->notification_change = 1;
			else if(!strcmp(buffer,"false"))
				param_attr->notification_change = 0;
			else if(len == 1) 
			{
				if (text[0] == '0')
					param_attr->notification_change = 0;
				else if (text[0] == '1')
					param_attr->notification_change = 1;
				else
					goto syntax_error;
			} 
			else 
			{
				goto syntax_error;
			}
		}
		break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
    else if (!evcpe_strncmp("Notification", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) 
        {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len == 1) {
				if (text[0] == '0')
					param_attr->notification = 0;
				else if (text[0] == '1')
					param_attr->notification = 1;
				else if (text[0] == '2')
					param_attr->notification = 2;
				else
					goto syntax_error;
			} else {
				goto syntax_error;
			}
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("AccessListChange", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) 
        {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len == 1) {
				if (text[0] == '0')
					param_attr->access_list_change = 0;
				else if (text[0] == '1')
					param_attr->access_list_change = 1;
				else
					goto syntax_error;
			} else {
	
			    if ((text[0] == 'f')||(text[0] == 'F'))
			    {
			    		param_attr->access_list_change = 0;
			    }
				else
				{
					param_attr->access_list_change = 1;
				}
				evcpe_info(__func__,"evcpe_msg_xml_data_cb 505050  value:%d\n",param_attr->access_list_change);
				//goto syntax_error;
			}
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
    else if (!evcpe_strncmp("AccessList", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) {
		case EVCPE_SET_PARAMETER_ATTRIBUTES:
			param_attr = parser->list_item;
			if (len && !strcmp("1", text))
				param_attr->access_list_change = 1;
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	} 
	else if (!evcpe_strncmp("MaxEnvelopes", elm->name, elm->len)) 
	{
		if (len != 1 )  //浙江MaxEnvelopes值不为1 故屏蔽掉:|| text[0] != '1'
			goto syntax_error;
		inform_resp = parser->msg->data;
		inform_resp->max_envelopes = 1;
	} 
	else if (!evcpe_strncmp("ObjectName", elm->name, elm->len)) 
	{
		switch (parser->msg->method_type) {
		case EVCPE_ADD_OBJECT:
			add_obj = parser->msg->data;
			if (len >= sizeof(add_obj->object_name)) {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(add_obj->object_name, text, len);
			add_obj->object_name[len] = '\0';
			break;
		case EVCPE_DELETE_OBJECT:
			delete_obj = parser->msg->data;
			if (len >= sizeof(delete_obj->object_name)) {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(delete_obj->object_name, text, len);
			delete_obj->object_name[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	} 
    else if (!evcpe_strncmp("ParameterKey", elm->name, elm->len)) 
    {
		switch (parser->msg->method_type) 
        {
		case EVCPE_ADD_OBJECT:
			add_obj = parser->msg->data;
			if (len >= sizeof(add_obj->parameter_key)) 
            {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(add_obj->parameter_key, text, len);
			add_obj->parameter_key[len] = '\0';
			break;
		case EVCPE_DELETE_OBJECT:
			delete_obj = parser->msg->data;
			if (len >= sizeof(delete_obj->parameter_key)) 
            {
				rc = EOVERFLOW;
				goto finally;
			}
			memcpy(delete_obj->parameter_key, text, len);
			delete_obj->parameter_key[len] = '\0';
			break;
		case EVCPE_SET_PARAMETER_VALUES: 
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("CommandKey", elm->name, elm->len))  //+ by aobai
	{
	//将commandkey写到e2p中xdl 20120817
		char cmdkey[64]={0};
		memcpy(cmdkey, text, len);
		cmdkey[len] = '\0'; 
        sk_api_params_set("tr069_commandkey",cmdkey); 
		printf("---xdl---cmdkey:%s\n",cmdkey);
	//end xdl 20120817

    //add by lijingchao  江苏移动项目
    /*
    平台下发参数的格式：
        <CommandKey>ForcedUpgrade-序列号<CommandKey>
        即：<CommandKey>0-123456<CommandKey>解析为非强制
        <CommandKey>1- 123456<CommandKey>解析为强制

    */
    if(EVCPE_DOWNLOAD == parser->msg->method_type)
    {
           if(strcmp("download",cmdkey))
           {
                if('0' == cmdkey[0])
                {
                    sk_tr069_set_forced_upgrade(NULL , "0", 1);
                }
                else
                {
                      sk_tr069_set_forced_upgrade(NULL , "1", 1);
                }

                //sk_api_params_set("tr069_commandkey","download"); 
           }
    }
		//add end
	
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->commandkey, text, len);
			download_params->commandkey[len] = '\0';
			break;
		case EVCPE_UPLOAD:
			upload_params = parser->msg->data;
			memcpy(upload_params->commandkey, text, len);
			upload_params->commandkey[len] = '\0';
			break;
		case EVCPE_REBOOT:
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("FileType", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->filetype, text, len);
			download_params->filetype[len] = '\0';

            evcpe_debug(__func__, " EVCPE_DOWNLOAD: FileType= %s",
					download_params->filetype);
			break;
		case  EVCPE_UPLOAD:
			upload_params = parser->msg->data;
			memcpy(upload_params->filetype, text, len);
			upload_params->filetype[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("URL", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->url, text, len);
			download_params->url[len] = '\0';
            evcpe_debug(__func__, " EVCPE_DOWNLOAD: URL= %s",
					download_params->url);
		
			break;
		case  EVCPE_UPLOAD:
			upload_params = parser->msg->data;
			memcpy(upload_params->url, text, len);
			upload_params->url[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("Username", elm->name, elm->len))  //浙江将UserName改成了Username
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->username, text, len);
			download_params->username[len] = '\0';
			break;
		case EVCPE_UPLOAD:
			upload_params = parser->msg->data;
			memcpy(upload_params->username, text, len);
			upload_params->username[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("Password", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->password, text, len);
			download_params->password[len] = '\0';
			break;
		case EVCPE_UPLOAD:
			upload_params = parser->msg->data;
			memcpy(upload_params->password, text, len);
			upload_params->password[len] = '\0';
			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("FileSize", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			filesize=atoi(text);

			download_params->filesize =  filesize;
			break;

		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("TargetFileName", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->targetfilename, text, len);
			download_params->targetfilename[len] = '\0';
			break;

		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("DelaySeconds", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			delayseconds=atoi(text);
			
			download_params->delayseconds =  delayseconds;

			break;

		case EVCPE_UPLOAD:
			upload_params = parser->msg->data;
			delayseconds=atoi(text);
			
			upload_params->delayseconds =  delayseconds;

			break;
		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("SuccessURL", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->successurl, text, len);
			download_params->successurl[len] = '\0';
			break;

		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (!evcpe_strncmp("FailureURL", elm->name, elm->len))  //+ by aobai
	{
		switch (parser->msg->method_type)
		{
		case EVCPE_DOWNLOAD:

			download_params = parser->msg->data;
			memcpy(download_params->failureurl, text, len);
			download_params->failureurl[len] = '\0';
			break;

		default:
			evcpe_error(__func__, "%d,unexpected evcpe_method_type: %d",
					__LINE__,parser->msg->method_type);
			goto syntax_error;
		}
	}
	else if (len > 0)
	{
		evcpe_error(__func__, "unexpected element: %.*s",
				elm->len, elm->name);
		goto syntax_error;
	}
	rc = 0;

finally:
	return rc;

syntax_error:
	evcpe_error(__func__, "syntax error");
	return EPROTO;
}

int evcpe_msg_xml_attr_cb(void *data, const char *ns, unsigned nslen,
		const char *name, unsigned name_len,
		const char *value, unsigned value_len)
{
	int rc;
	struct evcpe_msg_parser *parser = data;
	struct evcpe_xml_element *parent = evcpe_xml_stack_peek(&parser->stack);

	//printf("[kangzh] evcpe_msg_xml_attr_cb enter!\n\n");

	evcpe_trace(__func__, "attribute: %.*s => %.*s (namespace: %.*s)",
			name_len, name, value_len, value, nslen, ns);

	if (nslen && !evcpe_strncmp("xmlns", ns, nslen))
	{
		if ((rc = evcpe_xmlns_table_add(&parser->xmlns,
				name, name_len, value, value_len)))
			goto finally;
		if (!evcpe_strcmp(parent->ns, parent->nslen, name, name_len))
			parent->ns_declared = 1;

	}
	rc = 0;

finally:
	return rc;
}
