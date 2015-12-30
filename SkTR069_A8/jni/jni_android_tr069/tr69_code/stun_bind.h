#ifndef STUN_H
#define STUN_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>

#define STUN_VERSION 1.0

#define STUN_MAX_STRING 256
#define STUN_MAX_UNKNOWN_ATTRIBUTES 8
#define STUN_MAX_MESSAGE_SIZE 2048

#define PORT 3478

#if 1 //32bit:
#define REQ_LEN					(28 + 60 + 8)
#define REQ_IPCHANGE_LEN		(28 + 60 + 8 +4)
#define MSG_LEN 				(32+20+16+8)
#define MSG_IPCHANGE_LEN  		(32+20+16+8+4)
#define RES_LEN		100
#define USER_NAME_LEN (32)
#define DSLFORUM_STR_LEN (20)
#else //22 bit:
#define REQ_LEN					(28 + 60 + 8-10)
#define REQ_IPCHANGE_LEN		(28 + 60 + 8 +4-10)
#define MSG_LEN 				(32+20+16+8-10)
#define MSG_IPCHANGE_LEN  		(32+20+16+8+4-10)
#define RES_LEN		100
#define USER_NAME_LEN (20) //32-21=11
#define DSLFORUM_STR_LEN (20)
#endif


typedef unsigned char  UInt8;
typedef unsigned short UInt16;
typedef unsigned int   UInt32;
typedef struct{unsigned char octet[16];} UInt128;

//define types of stun message
#define BindingRequest            0x0001
#define BindingResponse           0x0101
#define BindingErrorResponse      0x0111
#define SharedSecretRequest       0x0002
#define SharedSecretResponse      0x0102
#define SharedSecretErrorResponse 0x0112
#define ConnectionRequestBinding  0xc001
#define BindingChange 			  0xc002
//define stun attributes
#define MappedAddress             0x0001
#define ResponseAddress           0x0002
#define ChangeRequest             0x0003
#define SourceAddress             0x0004
#define ChangedAddress            0x0005
#define Username                  0x0006
#define Password                  0x0007
#define MessageIntegrity          0x0008
#define ErrorCode                 0x0009
#define UnknowAttributes          0x000a
#define ReflectedFrom             0x000b

//define state here
#define Build             1
#define WaitResponse      2
#define ParseResponse     3
#define EndProcess        0

typedef struct
{
	UInt32 mapped_ip;
	UInt16 mapped_port;
	UInt32 changed_ip;
	UInt16 changed_port;
}Address4;

//define the head of stun message 
typedef struct 
{
	UInt16 msgType;
	UInt16 msgLength;
	UInt128 id;
}StunMsgHdr;


typedef struct
{
	UInt16 type;
	UInt16 length;
}StunAtrHdr;

typedef struct
{
	UInt16 port;
	UInt32 addr;
}StunAddress4;

typedef struct
{
	UInt8 pad;
	UInt8 family;
	StunAddress4 ipv4;
}StunAtrAddress4;

typedef struct
{
	UInt32 value;
}StunAtrChangeRequest;

typedef struct
{
	UInt16 pad; // all 0
	UInt8 errorClass;
	UInt8 number;
	char reason[STUN_MAX_STRING];
	UInt16 sizeReason;
}StunAtrError;

typedef struct
{
	UInt16 attrType[STUN_MAX_UNKNOWN_ATTRIBUTES];
	UInt16 numAttributes;
}StunAtrUnknown;

typedef struct
{
	char value[STUN_MAX_STRING];      
	UInt16 sizeValue;
}StunAtrString;

typedef struct
{
	char hash[20];
}StunAtrIntegrity;

typedef struct
{
	StunMsgHdr msgHdr;
	
	bool hasMappedAddress;
	StunAtrAddress4  mappedAddress;
	
	bool hasResponseAddress;
	StunAtrAddress4  responseAddress;
	
	bool hasChangeRequest;
	StunAtrChangeRequest changeRequest;
	
	bool hasSourceAddress;
	StunAtrAddress4 sourceAddress;
	
	bool hasChangedAddress;
	StunAtrAddress4 changedAddress;
	
	bool hasUsername;
	StunAtrString username;
	
	bool hasPassword;
	StunAtrString password;
	
	bool hasMessageIntegrity;
	StunAtrIntegrity messageIntegrity;
	
	bool hasErrorCode;
	StunAtrError errorCode;
	
	bool hasUnknownAttributes;
	StunAtrUnknown unknownAttributes;
	
	bool hasReflectedFrom;
	StunAtrAddress4 reflectedFrom;
}StunMessage; 
//generat transction ID
UInt128 createTransctionID(void);

void buildSimBindingRequest(char *buf, UInt128 *ID);

void buildTestBindingRequest(char *buf, bool change_ip, bool change_port,
                              UInt128 *ID,char *usrname,bool change);

bool sendBindingRequest(char *buf, int fd, UInt32 ip, UInt16 port, UInt16 l);

bool receiveResponse(char *buf, int fd, UInt16 l);

bool parseBindingResponse(char *buf, Address4 *addr, UInt128 *ID, int count);

bool sendMessage(char *bufReq, char *bufRes, int fd, bool change_ip,
		 bool change_port, UInt32 ip, UInt16 port, UInt16 l, Address4 *addr);

int stun_get_mapping_port();


#ifdef __cplusplus
}
#endif

#endif
