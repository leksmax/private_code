#ifndef __STUN_H__
#define __STUN_H__

#include <stdio.h>

#ifdef __cpluscplus
extern "C"
#endif

/* function parameter direction. */
#define IN
#define OUT

/* Default STUN port for STUN Request.*/
#define STUN_PORT_DEFAULT 3478

/* Default rebinding interval: 30 seconds. */
#define STUN_REBIND_INTERVAL 30 

/* brcm 7405 is little endian */
#define TARGET_LITTLE_ENDIAN 1

#define RESULT_ERROR -1
#define RESULT_SUCCESS 0

#define STUN_TRACE(x...)	\
do{							\
	printf("[%s,%d]", __FUNCTION__, __LINE__); \
	printf(x);	\
}while(0)

#define CHECK_MEM(expr)\
do{						\
	if(NULL == expr){	\
		STUN_TRACE("Allocate memory failed\n");\
		goto Err;			\
	}	\
}while(0)

typedef char			STUN_CHAR;
typedef unsigned char	STUN_BYTE;
typedef unsigned short	STUN_WORD;
typedef unsigned int	STUN_DWORD, STUN_BOOL;
typedef int				STUN_INT;
typedef void			STUN_VOID;

typedef struct{
	STUN_BYTE val128[16];	/* 128 bits == 16 Bytes */
}STUN_VAL128;

#define IPV4_FAMILY	0x01

/* STUN message header size. */
#define STUN_MESSAGE_HEADER_SIZE (				\
			sizeof(STUN_WORD) +		/* Message type */ 	\
			sizeof(STUN_WORD) +		/* Message length */\
			sizeof(STUN_VAL128))	/* Transaction ID */	

/*STUN message attribute header size*/
#define STUN_MESSAGE_ATTR_HEADER_SIZE (		\
			sizeof(STUN_WORD) +		/* Message type */ 	\
			sizeof(STUN_WORD) )		/* Message length */

/* STUN message attribute: ADDRESS-MAPPED size. */
#define STUN_ATTR_MAPPED_ADDR_SIZE	(	\
			sizeof(STUN_BYTE) +		/* Attribute value pad */			\
			sizeof(STUN_BYTE) +		/* Attribute IP address family */	\
			sizeof(STUN_WORD) +		/* Attribute port */				\
			sizeof(STUN_DWORD))		/* Attribute IP address */
			
/* STUN message attribute: RESPONSE-ADDRESS size. */
#define STUN_ATTR_RESPONSE_ADDR_SIZE (	\
			sizeof(STUN_BYTE) + 	/* Attribute value pad */			\
			sizeof(STUN_BYTE) + 	/* Attribute IP address family */	\
			sizeof(STUN_WORD) + 	/* Attribute port */				\
			sizeof(STUN_DWORD))		/* Attribute IP address */

/* STUN message attribute: CHANGED-ADDRESS size. */
#define STUN_ATTR_CHANGED_ADDR_SIZE	(	\
			sizeof(STUN_BYTE) + 	/* Attribute value pad */			\
			sizeof(STUN_BYTE) + 	/* Attribute IP address family */	\
			sizeof(STUN_WORD) + 	/* Attribute port */				\
			sizeof(STUN_DWORD))		/* Attribute IP address */
				
/* STUN message attribute: CHANGED-REQUEST size. Only used two bits. */
#define STUN_ATTR_CHANGED_REQUEST_SIZE (	\
			sizeof(STUN_DWORD))		/* Attribute value: only used two bits for IP/PORT change. */

/* STUN message attribute: SOURCE-ADDRESS size. */
#define STUN_ATTR_SOURCE_ADDR_SIZE	(	\
			sizeof(STUN_BYTE) + 	/* Attribute value pad */			\
			sizeof(STUN_BYTE) + 	/* Attribute IP address family */	\
			sizeof(STUN_WORD) + 	/* Attribute port */				\
			sizeof(STUN_DWORD))		/* Attribute IP address */

/* STUN message attribute: MESSAGE-INTEGRITY size. The value is fixed 20 Bytes. */
#define STUN_ATTR_MESSAGE_INTEGRITY_SIZE 0x0014

/* STUN message attribute: UNKNOWN-ATTRIUTES size. */
#define STUN_ATTR_UNKNOWN_ATTRIUTES_SIZE	(	\
			sizeof(STUN_WORD) + 	/* Attribute 1 type */			\
			sizeof(STUN_WORD) + 	/* Attribute 2 type */	\
			sizeof(STUN_WORD) + 	/* Attribute 3 type */				\
			sizeof(STUN_WORD))		/* Attribute 4 type */

/* STUN message attribute: REFLECTED-FORM size. */
#define STUN_ATTR_REFLECTED_ADDR_FROM_SIZE	(	\
			sizeof(STUN_BYTE) + 	/* Attribute value pad */			\
			sizeof(STUN_BYTE) + 	/* Attribute IP address family */	\
			sizeof(STUN_WORD) + 	/* Attribute port */				\
			sizeof(STUN_DWORD)) 	/* Attribute IP address */

/* STUN message attribute: ERROR_CODE fix size. */
#define STUN_ATTR_ERROR_CODE_FIX_SIZE	(	\
			sizeof(STUN_WORD) + 	/* Attribute value pad */			\
			sizeof(STUN_BYTE) + 	/* Attribute error code class */	\
			sizeof(STUN_BYTE) ) 	/* Attribute number */				

/* TR-111 specific extension, used in tr069. 
	Indicates the binding on which the CPE is listening for UDP Connection Request*/
#define STUN_ATTR_CONNECTION_REQUEST_BINDING_VALUE "dslforum.org/TR-111 "
#define STUN_ATTR_CONNECTION_REQUEST_BINDING_SIZE  0x0014   /* fixed 20 bytes. */


/* TR-111 specific extension, used in tr069. 
	Indicates that the binding has changed. No value, length must is zero. */
#define STUN_ATTR_BINDING_CHANGE_SIZE 0x0


/* ACS connect request message type. The value is magic number. */
#define TR111_CONNECTION_REQUEST_TYPE 0x1234

/* Connect request/change binding with different messgage attributes. 
 * Defined according to TR-111 */
typedef enum{
	BINDING_REQ_BASIC = 0,
	BINDING_REQ_AUTHORIZED,
	BINDING_CHANGE_BASIC,
	BINDING_CHANGE_AUTHORIZED
}BINDING_REQ_TYPE;

/* There are two types of request: 
 * Binding Request, send over UDP, and Shared Secrect Request, send over TSL over TCP. */
typedef enum{
    BINDING_REQUEST                 = 0x0001,
    BINDING_RESPONSE                = 0x0101,
    BINDING_ERROR_RESPONSE          = 0x0111,
    SHARED_SECRECT_REQUEST          = 0x0002,
    SHARED_SECRECT_RESPONSE         = 0x0102,
    SHARED_SECRECT_ERROR_RESPONSE   = 0x0112,
}STUN_MESSAGE_TYPE;

/* STUN message attribute. */
typedef enum{
    MAPPED_ADDRESS      = 0x0001,
    RESPONSE_ADDRESS    = 0x0002,
    CHANGE_REQUEST      = 0x0003,
    SOURCE_ADDRESS      = 0x0004,
    CHANGED_ADDRESS     = 0x0005,
    USERNAME            = 0x0006,
    PASSWORD            = 0x0007,
    MESSAGE_INTEGRITY   = 0x0008,
    ERROR_CODE          = 0x0009,
    UNKNOWN_ATTRIBUTES  = 0x000a,
    REFLECTED_FROM      = 0x000b,

	/* TR-111 specfic extension, used in tr069. */
	CONNECTION_REQUEST_BINDING = 0xC001,
	BINDING_CHANGE			   = 0xC002
}STUN_MESSAGE_ATTR_TYPE;

/* NAT type that STUN deals with, wihch is described in RFC3489. */
typedef enum{
    NAT_TYPE_UNKNOW = 0,
    NAT_TYPE_OPEN,              /* On the open Internet, no NAT translation. */
    NAT_TYPE_BLOCKED,           /* Firewall that blocks UDP */
    NAT_TYPE_SYMMETRIC_FIREWALL,/* Like Symmetric NAT, but no translation.*/
    NAT_TYPE_SYMMETRIC,  /* Same internal IP/PORT is mapped into different external PORT in different session. */
    NAT_TYPE_FULL_CONE,  /* After holing,  the client can receive any UDP packets sended to the NAT mapped external IP/PORT. */
    NAT_TYPE_RESTRICTED_CONE,   /* After holing,  client only can receive UDP packets sended by STUN server 
                                  to the NAT mapped external IP/PORT. */

    NAT_TYPE_RESTRICTED_PORT_CONE,  /* After holing,  client only can receive UDP packets sended by STUN server from origin PORT
                                       to the NAT mapped external IP/PORT. */
}STUN_NAT_TYPE;

#define STUN_STRING_MAX_LEN 256
#define STUN_UNKNOWN_ATTR_MAX 4
#define STUN_SHA1_DIGEST_LEN 20	/*  HMAC-SHA1 orig output length */
#define STUN_SHA1_HEXA_LEN 40	/*  HMAC-SHA1 hexadecimal output length */

typedef struct{
    STUN_BYTE type;     /* Binding or shared secret request/response message type. */
    STUN_WORD length;   /* Total length of STUN payload, not include the header. */
    STUN_VAL128 id;     /* 128bit, Transtraction ID used to correlate request and reponse.*/
}STUN_MESSAGE_HEADER;

typedef struct{
    STUN_WORD type; 	/* Message attribute type. */
    STUN_WORD length;	/* Total length of message attribute payload, not include the header. */
}STUN_MESSAGE_ATTR_HEADER;

typedef struct{
	STUN_BYTE pad;	/* zero. */
	STUN_BYTE family;
	STUN_WORD port;
	STUN_DWORD address;
}STUN_IPV4_ADDR;

typedef struct{
	STUN_DWORD value;
}STUN_ATTR_CHANGE_REQUEST;

typedef struct{
	STUN_CHAR value[STUN_STRING_MAX_LEN];
	STUN_DWORD size;
}STUN_STRING;

typedef struct{
	STUN_BYTE value[STUN_SHA1_DIGEST_LEN];
}STUN_ATTR_INTEGRITY;

typedef struct{
	STUN_WORD pad; /* zero */
	STUN_BYTE errorClass; /* low 3 bits valid. */
	STUN_BYTE number;
	STUN_STRING reason;
}STUN_ATTR_ERROR_CODE;

typedef struct{
	STUN_WORD attrType[STUN_UNKNOWN_ATTR_MAX];
	STUN_WORD attrNum;
}STUN_ATTR_UNKNOWN_ATTR;

/* Define a vector to record valut/size pair. */
typedef struct{
	STUN_VOID * ptr;
	STUN_DWORD size;
}STUN_VECTOR;

/* TR-111 extension used in tr069. For example:
 * "GET http://10.1.1.1:8080?ts=1120673700&id=1234&un=CPE57689 
 * &cn=XTGRWIPC6D3IPXS3&sig=3545F7B5820D76A3DF45A3A509DA8D8C38F13512 HTTP/1.1"*/
typedef struct{
    STUN_CHAR uri[STUN_STRING_MAX_LEN]; /* uri. */
    STUN_CHAR method[8];    /* must be HTTP GET method. */
	STUN_CHAR version[9];   /* HTTP/1.1 */
	STUN_CHAR host[16];     /* connectionRequest addr or CFE local addr. */
	STUN_DWORD port;        /* port to send connection request message. */
	STUN_CHAR  ts[32];	    /* timestamp. millisecond */
	STUN_DWORD id;	        /* same in the same request connection */
	STUN_CHAR un[64];       /* user name. */
	STUN_CHAR cn[64];       /* cnonce. random chosed by ACS. */
	STUN_BYTE sig[STUN_SHA1_HEXA_LEN + 1]; /* 40 character hexadecimal output. */
}TR111_UDP_CONNECT_REQUEST;

/* STUN message are TLV(type-length-value) encoded using big endian(network ordered) binary.
   All message start with a STUN header, followed by a STUN payload.*/
typedef struct{
    STUN_MESSAGE_HEADER header;

	STUN_BOOL hasMappedAddr;
	STUN_IPV4_ADDR mappedAddr;

	STUN_BOOL hasResponseAddr;
	STUN_IPV4_ADDR responseAddr;

	STUN_BOOL hasChangedAddr;
	STUN_IPV4_ADDR changedAddr;

	STUN_BOOL hasChangeRequest;
	STUN_ATTR_CHANGE_REQUEST changeRequest;
	
	STUN_BOOL hasSourcedAddr;
	STUN_IPV4_ADDR sourceAddr;

	STUN_BOOL hasUserName;
	STUN_STRING userName;

	STUN_BOOL hasPassword;
	STUN_STRING password;

	STUN_BOOL hasMessageIntegrity;
	STUN_ATTR_INTEGRITY messageIntegrity;

	STUN_BOOL hasErrorCode;
	STUN_ATTR_ERROR_CODE errorCode;

	STUN_BOOL hasUnknownAttr;
	STUN_ATTR_UNKNOWN_ATTR unknownAttr;

	STUN_BOOL hasReflectedFrom;
	STUN_IPV4_ADDR reflectedFrom;

	STUN_BOOL hasConnectionRequestBinding;
	STUN_STRING connectionRequestBinding;

	STUN_BOOL hasBindingChange;
}STUN_MESSAGE;

/* Define a structure for packing and unpacking STUN messages. */
typedef struct{
    STUN_BYTE* buffer;
    STUN_DWORD bufferSize;
    STUN_DWORD offset;
}STUN_MESSAGE_DESCRIPTOR;

typedef enum {
	STUN_TASK_ACTIVE,
	STUN_TASK_SUSPEND
}STUN_TASK_STATE;

typedef struct{
	BINDING_REQ_TYPE type;	    /* Binding request type. */	
	STUN_VAL128 bindingReqID;	/* Binding request transaction ID */
	STUN_IPV4_ADDR publicAddr;	/* NAT mapped plublic ip addr. */
	STUN_IPV4_ADDR localAddr;	/* Local ip addr of the customer device. */

	STUN_CHAR  conReqTS[32];	/* most recently valid UDP connect request timestamp. millisecond. */
	STUN_DWORD conReqID;	    /* most recently valid UDP connect request ID. */
	STUN_BOOL  natDetected;	    /* NAT Detected flag. */

	STUN_INT		sockfd;		/* Binding request socket fd. */
	STUN_TASK_STATE state;		/* STUN task state: ACTIVE, SUSPEND. */
}TR069_STUN_MESSAGE;


int stun_api_pipe_message(
	IN STUN_DWORD id,
	IN STUN_INT arg);

void *stun_api_task(void*);

#ifdef __cpluscplus
}
#endif

#endif
