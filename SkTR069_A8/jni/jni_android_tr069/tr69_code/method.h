// $Id: method.h 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#ifndef EVCPE_METHOD_H_
#define EVCPE_METHOD_H_

#include <time.h>

#include "data.h"

enum evcpe_method_type {
	EVCPE_UNKNOWN_METHOD,				//0
	EVCPE_GET_RPC_METHODS,				//1				
	EVCPE_SET_PARAMETER_VALUES,			//2
	EVCPE_GET_PARAMETER_VALUES,			//3
	EVCPE_GET_PARAMETER_NAMES,			//4
	EVCPE_SET_PARAMETER_ATTRIBUTES,		//5
	EVCPE_GET_PARAMETER_ATTRIBUTES,		//6
	EVCPE_ADD_OBJECT,					//7
	EVCPE_DELETE_OBJECT,				//8
	EVCPE_REBOOT,						//9
	EVCPE_DOWNLOAD,						//10
	EVCPE_UPLOAD,						//11
	EVCPE_FACTORY_RESET,				//12
	EVCPE_GET_QUEUED_TRANSFERS,			//13
	EVCPE_GET_ALL_QUEUED_TRANSFERS,		//14
	EVCPE_SCHEDULE_INFORM,				//15	
	EVCPE_SET_VOUCHERS,					//16
	EVCPE_GET_OPTIONS,					//17
	EVCPE_INFORM,						//18
	EVCPE_TRANSFER_COMPLETE,			//19
	EVCPE_AUTONOMOUS_TRANSFER_COMPLETE,	//20
	EVCPE_REQUEST_DOWNLOAD,				//21	
	EVCPE_KICKED						//22
};

const char *evcpe_method_type_to_str(enum evcpe_method_type type);

#endif /* EVCPE_METHOD_H_ */
