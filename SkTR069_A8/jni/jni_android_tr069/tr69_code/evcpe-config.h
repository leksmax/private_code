// $Id: evcpe-config.h 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
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

#ifndef EVCPECONFIG_H_
#define EVCPECONFIG_H_

#define EVCPE_VERSION "0.1"
//#define EVCPE_SOAP_ENC_XMLNS "soap-enc"
//#define EVCPE_SOAP_ENV_XMLNS "soap-env"

#define EVCPE_SOAP_ENC_XMLNS "SOAP-ENC"
#define EVCPE_SOAP_ENV_XMLNS "SOAP-ENV"


#define EVCPE_CWMP_XMLNS "cwmp"

//modify by lijingchao 修改为60秒
//#define EVCPE_ACS_TIMEOUT 600
#define EVCPE_ACS_TIMEOUT 			(30)

#define	TR069_INFORM_INTERVAL		(60)		//定时上报周期,江苏移动项目规定默认为1分钟

#define EVCPE_CREQ_INTERVAL 		(1)
#define EVCPE_XML_INDENT "    "

#define	EVCPE_NO_BOOT      		(0)
#define	EVCPE_BOOTING      		(1)
#define	EVCPE_PERIODIC    		(2)

#define	EVCPE_NO_DOWNLOAD 	 	(0)
#define	EVCPE_DOWNLOADING      	(1)



#define EVCPE_REQ_MAX 			(10)


#endif /* EVCPECONFIG_H_ */
