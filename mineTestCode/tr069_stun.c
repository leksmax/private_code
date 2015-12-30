#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>

#include "tr069_stun.h"

struct TR069Msgq {
	int size;
	int fds[2];
};

typedef struct TR069Msgq* TR069Msgq_t;

static TR069Msgq_t g_stunPipe = NULL; /* Pipe that stun communicate with tr069 main thread. */

int needUserNameAttr = 0;
void stun_PackOctet(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    IN STUN_BYTE octet)
{
    /* Check to see if the octet fits */
    if ( messageDescriptor->offset >= messageDescriptor->bufferSize ) {
        printf( "Error: Message's offset is lager then bufferSize\n");
        return ;
    }

    /* Push the octet into the buffer */
    messageDescriptor->buffer[messageDescriptor->offset] = octet;
    messageDescriptor->offset++;

    /* Success */
    return ;

}

void stun_PackWord(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    IN STUN_WORD value)
{
    int i;

    /* Pack each octet into the buffer */

#if TARGET_LITTLE_ENDIAN
    for ( i=sizeof(STUN_WORD)-1; i>=0; i-- )
#else /* TARGET_LITTLE_ENDIAN */
    for ( i=0; i<sizeof(STUN_WORD); i++ )
#endif /* TARGET_LITTLE_ENDIAN */
    {
        stun_PackOctet( messageDescriptor, ((STUN_BYTE*)(&value))[i] );
    }

}

void stun_PackDword(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    IN STUN_WORD value)
{
    int i;

    /* Pack each octet into the buffer */

#if TARGET_LITTLE_ENDIAN
    for ( i=sizeof(STUN_DWORD)-1; i>=0; i-- )
#else /* TARGET_LITTLE_ENDIAN */
    for ( i=0; i<sizeof(STUN_DWORD); i++ )
#endif /* TARGET_LITTLE_ENDIAN */
    {
        stun_PackOctet( messageDescriptor, ((STUN_BYTE*)(&value))[i] );
    }

}

void stun_PackOctets(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    IN STUN_BYTE *value,
    IN STUN_DWORD valueSize)
{
    STUN_DWORD i;

    if( value == NULL && valueSize > 0 ) {
        printf( "Error: !( value == NULL && valueSize > 0 )\n" );
        return;
    }

    for ( i=0; i<valueSize; i++) {
        stun_PackOctet( messageDescriptor, value[i] );
    }

}

void stun_PackVal128(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    IN STUN_VAL128 *value)
{
    stun_PackOctets(messageDescriptor, (STUN_BYTE*) value, sizeof(STUN_VAL128));
}


int stun_UnpackOctet(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    OUT STUN_BYTE *octet)
{
    /* Check to see if an octet is available */
    if ( messageDescriptor->offset >= messageDescriptor->bufferSize ) {
        printf("The buffer is too small\n");
        return -1;
    }

    /* Pop the octet from the buffer */
    *octet = messageDescriptor->buffer[messageDescriptor->offset];
    messageDescriptor->offset++;

    /* Success */
    return 0;

}

int stun_UnpackWord(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    OUT STUN_WORD *value)
{
    int i;

    /* Unpack each octet from the buffer */

#if TARGET_LITTLE_ENDIAN
    for ( i=sizeof(STUN_WORD)-1; i>=0; i-- )
#else /* TARGET_LITTLE_ENDIAN */
    for ( i=0; i<sizeof(STUN_WORD); i++ )
#endif /* TARGET_LITTLE_ENDIAN */
    {
        if(stun_UnpackOctet( messageDescriptor, &((STUN_BYTE*)value)[i] )){
            return -1;
        }
    }

    return 0;
}

int stun_UnpackDword(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    OUT STUN_DWORD *value)
{
    int i;

    /* Unpack each octet from the buffer */

#if TARGET_LITTLE_ENDIAN
    for ( i=sizeof(STUN_DWORD)-1; i>=0; i-- )
#else /* TARGET_LITTLE_ENDIAN */
    for ( i=0; i<sizeof(STUN_DWORD); i++ )
#endif /* TARGET_LITTLE_ENDIAN */
    {
        if(stun_UnpackOctet( messageDescriptor, &((STUN_BYTE*)value)[i] )){
            return -1;
        }
    }

    return 0;
}

int stun_UnpackOctets(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    OUT STUN_BYTE *value,
    IN STUN_DWORD valueSize)
{
     /* Check to see if there are enough octets in the buffer. Guard against overflow.*/
    if( ( messageDescriptor->offset + valueSize < messageDescriptor->offset ) ||
        ( messageDescriptor->offset + valueSize > messageDescriptor->bufferSize ) ) {
        return -1;
    }

    /* Pop the octets from the buffer */
    memcpy(value, &(messageDescriptor->buffer[messageDescriptor->offset]), valueSize);
    messageDescriptor->offset += valueSize;

    /* Success */
    return 0;
}

int stun_UnpackVal128(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    OUT STUN_VAL128 *value)
{
     STUN_DWORD i;

    for ( i=0; i<sizeof(STUN_VAL128) ; i++) {
        if ( stun_UnpackOctet( messageDescriptor, &((STUN_BYTE *)value)[i] ) ) {
            return -1;
        }
    }

    return 0;
}


int stun_FreeMessageDescriptor(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor)
{
    if(NULL != messageDescriptor){
        free(messageDescriptor);
        messageDescriptor = NULL;
    }

    return RESULT_SUCCESS;
}

int stun_AllocateMessageDescriptor(
    IN STUN_BYTE *message,
    IN STUN_DWORD messageSize,
    OUT STUN_MESSAGE_DESCRIPTOR **messageDescriptor)
{
    int retVal;
    STUN_MESSAGE_DESCRIPTOR *localMessageDescriptor = NULL;

    /* Initialization */
    *messageDescriptor = NULL;

    /* Allocate the descriptor for the message */
    localMessageDescriptor = (STUN_MESSAGE_DESCRIPTOR *)malloc( sizeof(STUN_MESSAGE_DESCRIPTOR) );

    CHECK_MEM( localMessageDescriptor );

    localMessageDescriptor->buffer = message;
    localMessageDescriptor->bufferSize = messageSize;
    localMessageDescriptor->offset = 0;


    /* Return the descriptor to the caller */
    *messageDescriptor = localMessageDescriptor;
    localMessageDescriptor = NULL;
    retVal= RESULT_SUCCESS;

    return retVal;
Err:
    retVal= RESULT_ERROR;
    /* Clean up locally used resources */
    if ( localMessageDescriptor != NULL ) {
        stun_FreeMessageDescriptor( localMessageDescriptor );
    }

    return retVal;

}


STUN_DWORD stun_FixedMessageSize(
    IN STUN_WORD messageType )
{
    return STUN_MESSAGE_HEADER_SIZE;
}

STUN_MESSAGE_DESCRIPTOR* stun_AllocateMessage(
    IN STUN_WORD messageType,
    IN STUN_DWORD variableSize)
{
    STUN_MESSAGE_DESCRIPTOR *messageDescriptor = NULL;

    /* Allocate the descriptor for the message */
    if( stun_AllocateMessageDescriptor( NULL, 0, &messageDescriptor ) ){
        printf("Allocate message descriptor failed\n");
    }

    /*  Allocate the buffer */
    messageDescriptor->bufferSize = stun_FixedMessageSize( messageType );
    if ( messageDescriptor->bufferSize == 0 ) {
        printf("Message size is zero\n" );
    }

    messageDescriptor->bufferSize += variableSize;
    
    messageDescriptor->buffer= (STUN_BYTE *)malloc( messageDescriptor->bufferSize );
    if ( messageDescriptor->buffer == NULL ) {
        printf("Allocate messsage's buffer failed\n" );
    }

    /* Pack the message type into the message */
    stun_PackWord( messageDescriptor, messageType );

    /* Pack the message length into the message */
    stun_PackWord( messageDescriptor, variableSize );

    /* Return the buffer to the caller */
    return messageDescriptor;

Err:

    /* Clean up on error */
    if ( messageDescriptor != NULL ) {
        if ( messageDescriptor->buffer != NULL ) {
            free( messageDescriptor->buffer );
        }

        /* Free the descriptor itself */
        stun_FreeMessageDescriptor( messageDescriptor );
    }

    return NULL;
}

int stun_GenerateTransactionID(
    OUT STUN_VAL128 *id)
{
    int i, randNum;
    
    for(i = 0; i < sizeof(STUN_VAL128); i += sizeof(int)){
        srand( time( NULL ) );
        randNum = rand();
        id->val128[i+0] = randNum ;
        id->val128[i+1] = randNum >> 8;
        id->val128[i+2] = randNum >> 16;
        id->val128[i+3] = randNum >> 24;
    }
    
    return RESULT_SUCCESS;
}

STUN_WORD stun_GetMessageType(
    IN STUN_BYTE * message,
    IN STUN_DWORD messageSize)
{
    STUN_WORD messageType = -1;
    
    if(messageSize < 2){
        printf("Message is too short! \n");
        return RESULT_ERROR;
    }

    if(message[0] == 0 || message[0] == 1){
        messageType = (message[0] << 8) | message[1];
    }else{
        messageType = TR111_CONNECTION_REQUEST_TYPE;
    }

    printf("messageType = %#X\n", messageType);
    return messageType;
}

int stun_PackMessageAttr(
    IN STUN_MESSAGE_ATTR_TYPE attrType,
    IN STUN_VOID *attrValue,
    IN STUN_DWORD attrSize,
    OUT STUN_MESSAGE_DESCRIPTOR *messageDescriptor )
{
    stun_PackWord(messageDescriptor, attrType);
    stun_PackWord(messageDescriptor, attrSize);

    switch(attrType){
        case CONNECTION_REQUEST_BINDING:
            stun_PackOctets(messageDescriptor,
                        (STUN_BYTE *)STUN_ATTR_CONNECTION_REQUEST_BINDING_VALUE,
                        STUN_ATTR_CONNECTION_REQUEST_BINDING_SIZE );
            break;
        case BINDING_CHANGE:    
            break;
        case USERNAME:
            stun_PackOctets(messageDescriptor, (STUN_BYTE *)attrValue, attrSize);
            break;
        case MESSAGE_INTEGRITY:
        {
            STUN_ATTR_INTEGRITY digest;
            STUN_CHAR password[STUN_STRING_MAX_LEN] = {0};

            tr069_api_getValue("STUNPassword", password, NULL);
            HMAC_SHA1DigestGenerate((STUN_BYTE *)password, strlen(password), messageDescriptor->buffer,
                                    messageDescriptor->offset - STUN_MESSAGE_ATTR_HEADER_SIZE, digest.value );
            stun_PackOctets(messageDescriptor, digest.value, STUN_SHA1_DIGEST_LEN);

            printf("stunPassword: %s, len = %d\n", password, strlen(password));
       
            break;
        }
        default:
            break;
    }

    return RESULT_SUCCESS;
}

int stun_UnpackMessageAttr(
    IN STUN_MESSAGE_DESCRIPTOR *messageDescriptor,
    OUT STUN_MESSAGE *parsedMsg)
{
    int i;
    STUN_WORD msgAttrType;
    STUN_WORD msgAttrSize;

    while(messageDescriptor->offset < messageDescriptor->bufferSize){
        stun_UnpackWord(messageDescriptor, &msgAttrType);
        stun_UnpackWord(messageDescriptor, &msgAttrSize);
        
        printf("Message attribute type : %u, size = %u\n", msgAttrType, msgAttrSize);

        switch(msgAttrType){
            case MAPPED_ADDRESS:
                parsedMsg->hasMappedAddr = 1;
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->mappedAddr.pad));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->mappedAddr.family));
                stun_UnpackWord(messageDescriptor, &(parsedMsg->mappedAddr.port));
                stun_UnpackDword(messageDescriptor, &(parsedMsg->mappedAddr.address));
                break;
            case RESPONSE_ADDRESS:
                parsedMsg->hasResponseAddr = 1;
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->responseAddr.pad));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->responseAddr.family));
                stun_UnpackWord(messageDescriptor, &(parsedMsg->responseAddr.port));
                stun_UnpackDword(messageDescriptor, &(parsedMsg->responseAddr.address));
                break;
            case CHANGE_REQUEST:
                parsedMsg->hasChangeRequest = 1;
                stun_UnpackDword(messageDescriptor, &(parsedMsg->changeRequest.value));
                break;
            case SOURCE_ADDRESS:
                parsedMsg->hasSourcedAddr = 1;
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->sourceAddr.pad));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->sourceAddr.family));
                stun_UnpackWord(messageDescriptor, &(parsedMsg->sourceAddr.port));
                stun_UnpackDword(messageDescriptor, &(parsedMsg->sourceAddr.address));
                break;
            case CHANGED_ADDRESS:
                parsedMsg->hasChangedAddr = 1;
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->changedAddr.pad));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->changedAddr.family));
                stun_UnpackWord(messageDescriptor, &(parsedMsg->changedAddr.port));
                stun_UnpackDword(messageDescriptor, &(parsedMsg->changedAddr.address));
                break;
            case USERNAME:
                parsedMsg->hasUserName = 1;
                stun_UnpackOctets(messageDescriptor, (STUN_BYTE *)(parsedMsg->userName.value), msgAttrSize);
                parsedMsg->userName.size = msgAttrSize;
                break;
            case PASSWORD:
                parsedMsg->hasPassword = 1;
                stun_UnpackOctets(messageDescriptor, (STUN_BYTE *)(parsedMsg->password.value), msgAttrSize);
                parsedMsg->password.size = msgAttrSize;
                break;
            case MESSAGE_INTEGRITY:
                parsedMsg->hasMessageIntegrity = 1;
                stun_UnpackOctets(messageDescriptor, parsedMsg->messageIntegrity.value, msgAttrSize);
                break;
            case ERROR_CODE:
                parsedMsg->hasErrorCode = 1;
                stun_UnpackWord(messageDescriptor, &(parsedMsg->errorCode.pad));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->errorCode.errorClass));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->errorCode.number));
                stun_UnpackOctets(messageDescriptor, (STUN_BYTE *)(parsedMsg->errorCode.reason.value), 
                                msgAttrSize - STUN_ATTR_ERROR_CODE_FIX_SIZE);
                
                parsedMsg->errorCode.reason.size = msgAttrSize - STUN_ATTR_ERROR_CODE_FIX_SIZE;
                break;
            case UNKNOWN_ATTRIBUTES:
                parsedMsg->hasUnknownAttr = 1;
                for(i = 0; i < STUN_UNKNOWN_ATTR_MAX; i++){
                    stun_UnpackWord(messageDescriptor, &(parsedMsg->unknownAttr.attrType[i]));
                }
                break;
            case REFLECTED_FROM:
                parsedMsg->hasReflectedFrom = 1;
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->reflectedFrom.pad));
                stun_UnpackOctet(messageDescriptor, &(parsedMsg->reflectedFrom.family));
                stun_UnpackWord(messageDescriptor, &(parsedMsg->reflectedFrom.port));
                stun_UnpackDword(messageDescriptor, &(parsedMsg->reflectedFrom.address));
                break;
            case CONNECTION_REQUEST_BINDING:
                parsedMsg->hasConnectionRequestBinding = 1;
                stun_UnpackOctets(    messageDescriptor, (STUN_BYTE *)(parsedMsg->connectionRequestBinding.value),
                                    STUN_ATTR_CONNECTION_REQUEST_BINDING_SIZE);
                break;
            case BINDING_CHANGE:
                parsedMsg->hasBindingChange = 1;
                break;
            default:
                printf("Unrecognized stun message attribute.\n");
                break;
        }
    }

    return RESULT_SUCCESS;
}

/* TBD: STUN Binding life discovery. */
int stun_PackBindingRequestMsg(
    IN STUN_VAL128 *transactionID,
    IN BINDING_REQ_TYPE type,
    OUT STUN_BYTE **pMessage,
    OUT STUN_DWORD *pMessageSize)
{
    STUN_DWORD retVal = RESULT_SUCCESS;
    STUN_INT isPrimary = 0, isBindingChange = 0, needAuth = 0;
    STUN_DWORD msgBodySize = 0;
    STUN_CHAR userName[STUN_STRING_MAX_LEN] = {0}; /* Must be 4 bytes alignment.Guaranteed by server. */
    STUN_INT  userNameSize = 0; 
    STUN_MESSAGE_DESCRIPTOR *messageDescriptor = NULL;

    if( NULL == pMessage || pMessageSize == NULL ){
        printf("pMessage==NULL || pMessageSize == NULL\n");
    }
    
    printf("Binding request type= %d\n", type);
    switch(type){
        case BINDING_REQ_BASIC:
            isPrimary = 1;
            break;
        case BINDING_REQ_AUTHORIZED:
            isPrimary = needAuth = 1;
            break;
        case BINDING_CHANGE_BASIC:
            isPrimary = isBindingChange = 1;
            break;
        case BINDING_CHANGE_AUTHORIZED:
            isPrimary = isBindingChange = needAuth = 1;
            break;
        default:
            printf("Not support this binding request, type= %d\n", type);
            break;
    }

    /* Get the message body size. */
    if( isPrimary ){
        msgBodySize += STUN_MESSAGE_ATTR_HEADER_SIZE + STUN_ATTR_CONNECTION_REQUEST_BINDING_SIZE;
    }
    if( isBindingChange ){
        msgBodySize += STUN_MESSAGE_ATTR_HEADER_SIZE;
    }
    if( needAuth ){
        tr069_api_getValue("STUNUsername", userName, NULL);
        userNameSize = strlen(userName);
        msgBodySize += STUN_MESSAGE_ATTR_HEADER_SIZE + userNameSize;
        msgBodySize += STUN_MESSAGE_ATTR_HEADER_SIZE + STUN_ATTR_MESSAGE_INTEGRITY_SIZE;
    }
    if(needUserNameAttr){
        tr069_api_getValue("STUNUsername", userName, NULL);
        userNameSize = strlen(userName);
        msgBodySize += STUN_MESSAGE_ATTR_HEADER_SIZE + userNameSize;
    }
    messageDescriptor = stun_AllocateMessage( BINDING_REQUEST, msgBodySize );
    CHECK_MEM( messageDescriptor );

    /* Packet transaction ID into message. */
    stun_PackVal128(messageDescriptor, transactionID);
    
    /* Packet message attribute into message. */
    if( isPrimary ){
           stun_PackMessageAttr(CONNECTION_REQUEST_BINDING, NULL, 
                STUN_ATTR_CONNECTION_REQUEST_BINDING_SIZE, messageDescriptor);
    }

    if( isBindingChange ){
        stun_PackMessageAttr(BINDING_CHANGE, NULL, STUN_ATTR_BINDING_CHANGE_SIZE, messageDescriptor);
    }
    if(needUserNameAttr){
        stun_PackMessageAttr(USERNAME, userName, userNameSize, messageDescriptor);
    }
    if( needAuth ){
        stun_PackMessageAttr(USERNAME, userName, userNameSize, messageDescriptor);
        stun_PackMessageAttr(MESSAGE_INTEGRITY, NULL, STUN_ATTR_MESSAGE_INTEGRITY_SIZE, messageDescriptor);
    }

    *pMessage = messageDescriptor->buffer;
    *pMessageSize = messageDescriptor->bufferSize;

    printf("userName = %s, bufferSize = %d, offset =%d\n", userName,
            messageDescriptor->bufferSize, messageDescriptor->offset);

    retVal = RESULT_SUCCESS;
    
Err:
    retVal= RESULT_ERROR;
    if(NULL != messageDescriptor){
        stun_FreeMessageDescriptor( messageDescriptor );
    }
    
    return retVal;
}

int stun_ValidateMessage(
    IN STUN_BYTE *message,
    IN STUN_DWORD messageSize,
    OUT STUN_MESSAGE_DESCRIPTOR **messageDescriptor )
{
    int retVal;
    STUN_WORD messageType;
    
    STUN_MESSAGE_DESCRIPTOR *localMsgDescriptor = NULL;

    if(stun_AllocateMessageDescriptor(message, messageSize, &localMsgDescriptor)){
        printf("Allocate message descriptor failed\n");
    }

    if(stun_UnpackWord(localMsgDescriptor, &messageType)){
        printf("unpack message type error\n");
    }

    printf("received message type: %#x\n", messageType);
    
    if((messageType != BINDING_RESPONSE) && (messageType != BINDING_ERROR_RESPONSE)){
        printf("Invalid. is not binding response message\n");
    }

    *messageDescriptor = localMsgDescriptor;
    localMsgDescriptor = NULL;
    
    retVal = RESULT_SUCCESS;

    return retVal;
Err:
    retVal = RESULT_ERROR;
    if(localMsgDescriptor != NULL){
        stun_FreeMessageDescriptor(localMsgDescriptor);
    }
    
    return retVal;
}

int stun_UnpackBindingResponseMsg(
    IN STUN_BYTE *message,
    IN STUN_DWORD messageSize,
    OUT STUN_MESSAGE *parsedMsg )
{
    int retVal = RESULT_SUCCESS;
    STUN_MESSAGE_DESCRIPTOR *messageDescriptor = NULL;
    STUN_MESSAGE *localMsg = parsedMsg;
    
    retVal = stun_ValidateMessage(message, messageSize, &messageDescriptor);
    if(RESULT_SUCCESS != retVal){
        printf("invalid message\n");
    }

    /* Unpack message header.*/
    stun_UnpackWord(messageDescriptor, &(localMsg->header.length));
    stun_UnpackVal128(messageDescriptor, &(localMsg->header.id));
    
    /* Unpack message attribute. */
    if(localMsg->header.length > 0){
        retVal = stun_UnpackMessageAttr(messageDescriptor, localMsg);
    }
    
Err:
    if(NULL != messageDescriptor){
        stun_FreeMessageDescriptor( messageDescriptor );
    }
    
    return retVal;
}

int stun_ProcessBindingResponseMsg(
    IN STUN_BYTE *message,
    IN STUN_DWORD messageSize,
    OUT TR069_STUN_MESSAGE *validInfo )
{
    //STUN_CHAR stunUserName[STUN_STRING_MAX_LEN];
    STUN_CHAR stunpssword[STUN_STRING_MAX_LEN];
    STUN_BYTE digest[STUN_SHA1_DIGEST_LEN] = {0};
    STUN_MESSAGE parsedMsg;
    STUN_INT retVal;
    
    memset(&parsedMsg, 0, sizeof(parsedMsg));
    retVal = stun_UnpackBindingResponseMsg(message, messageSize, &parsedMsg);
    if(RESULT_SUCCESS != retVal){
        printf(" unpack binding reponse message error\n");
    }

    /* Verify transaction ID */
    if(memcmp(validInfo->bindingReqID.val128, parsedMsg.header.id.val128, 16)){
        printf("invalid binding response\n");
    }

    /* Verify message integrity. */
    if(parsedMsg.hasMessageIntegrity){
        if( validInfo->type == BINDING_REQ_BASIC ||
            validInfo->type == BINDING_CHANGE_BASIC){
            printf("request message is basic, but response need integrity\n");
        }else{
            validInfo->type = BINDING_REQ_BASIC;
        }

        tr069_api_getValue("STUNPassword", stunpssword, NULL);
        HMAC_SHA1DigestGenerate((STUN_BYTE *)stunpssword, strlen(stunpssword), message,
            messageSize - (STUN_MESSAGE_ATTR_HEADER_SIZE + STUN_ATTR_MESSAGE_INTEGRITY_SIZE),
            digest );

        if(memcmp(parsedMsg.messageIntegrity.value, digest, STUN_SHA1_DIGEST_LEN)){
            printf("Reponse message is not integrity. \n");
        }
    }

    if(parsedMsg.hasErrorCode){
        int errorCode = (parsedMsg.errorCode.errorClass & 0x7) * 100 + 
                        parsedMsg.errorCode.number;
        
        printf("errorCode = %d\n", errorCode);

        if(errorCode == 401){
            printf("Unautheorized\n");
            if( validInfo->type == BINDING_REQ_BASIC){
                validInfo->type = BINDING_REQ_AUTHORIZED;
            }else if(validInfo->type == BINDING_CHANGE_BASIC){
                validInfo->type = BINDING_CHANGE_AUTHORIZED;
            }
        }else{
            /* TBD: Other errorCode. */
            validInfo->type = BINDING_REQ_BASIC;
        }
    }

    if(parsedMsg.hasMappedAddr){
        /* NAT mapped address changed, then enter binding change phase. */
        if( (validInfo->publicAddr.address != 0) &&
            (validInfo->publicAddr.address != parsedMsg.mappedAddr.address)){
            validInfo->type = BINDING_CHANGE_BASIC;
        }else{
            validInfo->type = BINDING_REQ_BASIC;
        }

        char buf[16] = {0};
        char notifyAddr[32] = {0};
        char addrDotFormat[16] = {0};

        tr069_port_getValue("Device.LAN.IPAddress", buf, NULL);
        validInfo->localAddr.address = tr069_ipAddrDot2Int( buf );

        if(validInfo->localAddr.address != parsedMsg.mappedAddr.address){
            if(!validInfo->natDetected){
                tr069_ipAddrInt2Dot(parsedMsg.mappedAddr.address, addrDotFormat, sizeof(addrDotFormat));
                sprintf(notifyAddr, "%s:%u", addrDotFormat, parsedMsg.mappedAddr.port);
                tr069_api_setValue("UDPConnectionRequestAddress", notifyAddr, 0);
                tr069_api_setValue("NATDetected", NULL, 1);

                validInfo->natDetected = 1;
                validInfo->publicAddr = parsedMsg.mappedAddr;
            }
        }else{
            /* TBD: set local address/port as UDPConnectionRequestAddress */
            if(validInfo->natDetected){
                tr069_ipAddrInt2Dot(parsedMsg.mappedAddr.address, addrDotFormat, sizeof(addrDotFormat));
                sprintf(notifyAddr, "%s:%u", addrDotFormat, parsedMsg.mappedAddr.port);
                tr069_api_setValue("UDPConnectionRequestAddress", notifyAddr, 0);
                tr069_api_setValue("NATDetected", NULL, 0);

                validInfo->natDetected = 0;
            }
        }
    }

    return RESULT_SUCCESS;
Err:
    return RESULT_ERROR;
}

int stun_UnpackUPDConnectRequestMsg(
    IN STUN_BYTE *message,
    IN STUN_DWORD messageSize,
    OUT TR111_UDP_CONNECT_REQUEST *connRequest )
{
    int len, item; 
    STUN_CHAR *msg = (STUN_CHAR *)message;
    TR111_UDP_CONNECT_REQUEST *req = connRequest;

    printf("ConnRequest:%s\n", message);

    /* Extract request line: method, uri, version. */
    item = sscanf(msg, "%s%256s%8s",req->method, req->uri, req->version);
    if(item != 3 || strcasecmp(req->method, "GET") || strcasecmp(req->version,"HTTP/1.1")){
        printf("http request line is invalid\n");
    }

    /* Extract host/port from uri. */
    if(sscanf(req->uri, "http://%16[^:]:%u%n", req->host, &(req->port), &len) == 2){
    }else if(sscanf(req->uri, "http://%16[^?]%n", req->host, &len) == 1){
        req->port = 80;  /* Http default port */
    }

    /* Extract query string from uri. */
       item = sscanf(req->uri+len, 
                    "?ts=%32[^&]"
                     "&id=%u"
                    "&un=%64[^&]"
                    "&cn=%64[^&]"
                     "&sig=%s%n", 
                     req->ts, &(req->id), req->un, req->cn, req->sig, &len);
    if(item != 5){
        printf("Querey string parse error\n");
    }

    printf("method:%s,uri:%s,version:%s,host:%s, port=%u"
                "ts= %s, id=%u, un = %s, cn = %s, sig = %s\n",
                req->method, req->uri, req->version, req->host, req->port,
                req->ts, req->id, req->un, req->cn, req->sig);
    
    return RESULT_SUCCESS;
 Err:
    return RESULT_ERROR;
}

int stun_ProcessUPDConnectRequestMsg(
    IN STUN_BYTE *message,
    IN STUN_DWORD messageSize,
    OUT TR069_STUN_MESSAGE *validInfo )
{
#if 0
    STUN_BYTE digestHexa[STUN_SHA1_HEXA_LEN] = {0};
    STUN_BYTE digest[STUN_SHA1_DIGEST_LEN] = {0};
    STUN_BYTE text[STUN_STRING_MAX_LEN] = {0};
    STUN_INT len;
    STUN_CHAR conReqUserName[STUN_STRING_MAX_LEN];
    STUN_CHAR conReqPassword[STUN_STRING_MAX_LEN];
    TR111_UDP_CONNECT_REQUEST connRequest;

    memset(&connRequest, 0 ,sizeof(connRequest));
    if(RESULT_SUCCESS != stun_UnpackUPDConnectRequestMsg( message, messageSize, &connRequest )){
        printf(" unpack UDP connect request message error\n");
    }

    if(strncmp(validInfo->conReqTS, connRequest.ts, 32) >= 0){
        printf(" Timestamp is equel or less than most recently valid timestamp\n");
    }
    
    if( validInfo->conReqID == connRequest.id){
        printf(" Message ID is same as last valid ID\n");
    }

    tr069_api_getValue("ConnectionRequestUsername", conReqUserName, NULL);
    if(strcmp(conReqUserName, connRequest.un )){
        printf(" Connection userName is invalid\n");
    }

    tr069_api_getValue("ConnectionRequestPassword", conReqPassword, NULL);
    len = sprintf((STUN_CHAR *)text, "%s%u%s%s",connRequest.ts, connRequest.id, connRequest.un, connRequest.cn);
    HMAC_SHA1DigestGenerate((STUN_BYTE *)conReqPassword, strlen(conReqPassword), text, len, digest);
    HMAC_SHA1HedaxOutput(digest, digestHexa);

    printf("password: %s, Text:%s, hamcSig = %s\n", conReqPassword, text, digestHexa);
    if(memcmp(connRequest.sig, digestHexa, STUN_SHA1_HEXA_LEN)){
        printf("Reponse message is not integrity. \n");
    }

    /* Record most recently valid info. */
    strcpy(validInfo->conReqTS, connRequest.ts);
    validInfo->conReqID = connRequest.id;
#endif

    /* Init a connection request session from CFE to ACS. */
    tr069_api_setValue("Task.Connect", NULL, 1);

    return RESULT_SUCCESS;
#if 0
Err:
    return RESULT_ERROR;
#endif
}

/* Open a no blocked UDP socket*/
int stun_UDPOpenSocket(
    IN STUN_INT family,
    IN STUN_INT type,
    IN STUN_INT protocol)
{
    STUN_INT sockfd = -1;

    sockfd = socket(family, type, protocol);
    if(sockfd < 0){
        printf("socket error\n");
        return RESULT_ERROR;
    }

    /* NO BLOCK socket.*/
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);

    return sockfd;
}

void stun_UDPCloseSocket(
    IN STUN_INT sockfd)
{
    if(sockfd > 0){
        close(sockfd);
    }
}

int stun_UDPSendMsg(
    IN STUN_INT sockfd,
    IN STUN_BYTE *sendMsg,
    IN STUN_DWORD sendMsgSize)
{
    STUN_CHAR stunServerAddr[16];
    STUN_DWORD  stunServerPort;
    struct sockaddr_in servaddr;
    STUN_INT ret;

    tr069_api_getValue("STUNServerAddress", stunServerAddr, NULL);
    stunServerPort = 0;
    tr069_api_getValue("STUNServerPort", NULL, (unsigned int *)&stunServerPort);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, stunServerAddr, &servaddr.sin_addr);
    servaddr.sin_port = htons(stunServerPort);

    
    ret = sendto(sockfd, sendMsg, sendMsgSize, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret == -1)
        printf("sendto error");
    
    return 0;
Err:
    return -1;
}

int stun_UDPRecvMsg(
    IN STUN_INT sockfd,
    OUT STUN_BYTE **recvMsg,
    OUT STUN_DWORD *recvMsgSize)
{
    char buf[2048] = {0};
    char *temp = NULL;
    STUN_INT ret, len;

    len = sizeof(buf);

    ret = recvfrom(sockfd, buf, len , 0, NULL, 0);
    if(ret <= 0){
        printf("recvfrom error");
    }

    temp = (char *)malloc(ret);
    CHECK_MEM(temp);
    memcpy(temp, buf, ret);

    *recvMsg = (STUN_BYTE *)temp;
    *recvMsgSize = ret;
    
    return 0;
Err:
    return -1;
}

int stun_BindingDiscoveryMaintain(void *arg)
{    
    fd_set rset;
    int ret, nfds, pipefd;
    struct timeval timeout = {0};
    STUN_DWORD lastTime, currentTime;
    
    STUN_BYTE *sendMsg = NULL;
    STUN_DWORD sendMsgSize;
    STUN_BYTE *recvMsg = NULL;
    STUN_DWORD recvMsgSize;
    STUN_DWORD recvMsgType;
    
    TR069_STUN_MESSAGE validInfo;
    memset(&validInfo, 0, sizeof(validInfo));
    
    validInfo.sockfd = stun_UDPOpenSocket(AF_INET, SOCK_DGRAM, 0);
    if(validInfo.sockfd < 0){
        printf("open the udp socket failed\n");
        return RESULT_ERROR;
    }

    /* Initialize */
    g_stunPipe = tr069_msgq_create(sizeof(struct Message), 20);
    pipefd = tr069_msgq_fd(g_stunPipe);

    validInfo.type = BINDING_REQ_BASIC;
    validInfo.state = STUN_TASK_SUSPEND;
    currentTime = tr069_sec( );
    lastTime = currentTime - STUN_REBIND_INTERVAL;

    while(1){
        if(sendMsg != NULL){
            free(sendMsg);
            sendMsg = NULL;
        }
        
        if(recvMsg != NULL){
            free(recvMsg);
            recvMsg = NULL;
        }

        FD_ZERO(&rset);
        FD_SET(pipefd, &rset);
        nfds = pipefd;

        switch(validInfo.state){
            case STUN_TASK_ACTIVE:
                timeout.tv_sec = STUN_REBIND_INTERVAL;
                
                FD_SET(validInfo.sockfd, &rset);
                if(nfds < validInfo.sockfd){
                    nfds = validInfo.sockfd;
                }

                switch(validInfo.type){
                    case BINDING_REQ_BASIC:
                    case BINDING_CHANGE_BASIC:
                        currentTime = tr069_sec();
                        if(currentTime - lastTime < STUN_REBIND_INTERVAL){
                            break;
                        }
                        lastTime = currentTime;
                        
                        /* New session. Generate the unique transaction  ID. */
                        stun_GenerateTransactionID(&(validInfo.bindingReqID));
                        stun_PackBindingRequestMsg(&(validInfo.bindingReqID), validInfo.type, &sendMsg, &sendMsgSize);
                        stun_UDPSendMsg(validInfo.sockfd, sendMsg, sendMsgSize);
                        break;
                    case BINDING_REQ_AUTHORIZED:
                    case BINDING_CHANGE_AUTHORIZED:
                        stun_PackBindingRequestMsg(&(validInfo.bindingReqID), validInfo.type, &sendMsg, &sendMsgSize);
                        stun_UDPSendMsg(validInfo.sockfd, sendMsg, sendMsgSize);
                        break;
                    default:
                        break;
                }
                
                break;
            case STUN_TASK_SUSPEND:
                timeout.tv_sec = 0x7fffffff;
                break;
            default:
                break;
        }

        printf(" timeout = %ld, sockfd = %u, pipefd = %d, nfds = %d\n",
                timeout.tv_sec, validInfo.sockfd, pipefd, nfds);

        ret = select( nfds + 1, &rset, NULL, NULL, &timeout );
        if(ret <= 0){
            validInfo.type = BINDING_REQ_BASIC;
            printf("timeout\n");
            continue;
        }

        if(FD_ISSET(pipefd, &rset)){
            /* ACS disable/enable the STUN function */
            struct Message msg;
            if (tr069_msgq_getmsg(g_stunPipe, (char *)(&msg))){
                printf("STUN pipe read failed\n");
                continue;
            }
            
            printf("stun recv message id = %d, arg = %d\n", msg.id, msg.arg0);
            
            switch(msg.id){
                case STUN_TASK_ACTIVE:
                case STUN_TASK_SUSPEND:
                    validInfo.state = msg.id;
                    break;
                default:
                    break;
            }
        }else if(FD_ISSET(validInfo.sockfd, &rset)){
            if(stun_UDPRecvMsg(validInfo.sockfd, &recvMsg, &recvMsgSize) < 0){
                printf("receive message error\n");
                continue;
            }
            
            recvMsgType = stun_GetMessageType(recvMsg, recvMsgSize);
            switch(recvMsgType){
            case TR111_CONNECTION_REQUEST_TYPE:
                stun_ProcessUPDConnectRequestMsg(recvMsg, recvMsgSize, &validInfo);
                
                break;
            case BINDING_RESPONSE:
            case BINDING_ERROR_RESPONSE:
                stun_ProcessBindingResponseMsg(recvMsg, recvMsgSize, &validInfo );
                
                break;
            default:
                printf("Unrecognized message\n");
                break;
            }
        }    
    }

    stun_UDPCloseSocket(validInfo.sockfd);
    
    return 0;
}

int stun_api_pipe_message(
    IN STUN_DWORD id,
    IN STUN_INT arg)
{
    struct Message msg;

    printf("send STUN message ID = %u\n", id);
    msg.id = id;
    msg.arg0 = arg;

    if (g_stunPipe)
        return tr069_msgq_putmsg(g_stunPipe, (char *)&msg);
    return -1;
}

void *stun_api_task(void *arg)
{    
    printf(" STUN task inited \n");
    stun_BindingDiscoveryMaintain(arg);
    return (void *)0;
}

#if 1
int main()
{
    STUN_MESSAGE_DESCRIPTOR *descriptor = NULL;
    descriptor =  stun_AllocateMessage(BINDING_REQUEST, 16);
    if(descriptor){
        printf("size= %d, offset = %d\n",descriptor->bufferSize, descriptor->offset );
        printf("%#x, %#x\n", descriptor->buffer[0], descriptor->buffer[1]);
    }else{
        printf("allocateMessage failed\n");
    }

    return 0;
}
#endif

