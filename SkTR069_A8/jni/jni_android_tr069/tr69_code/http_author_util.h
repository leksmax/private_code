#ifndef __HTTP_AUTH_UTIL_H__
#define __HTTP_AUTH_UTIL_H__

#ifndef VOID
#define VOID void
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef SHORT
#define SHORT short
#endif

#ifndef USHORT
#define USHORT  unsigned short
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

//typedef USHORT BOOL;
//#define TRUE ((BOOL)1)
//#define FALSE ((BOOL)0)

#ifdef NULL
#undef NULL
#endif

#define NULL    0

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#ifdef WIN32
    #ifndef XINLINE
    #define XINLINE
    #endif
#else
    #define XINLINE inline
#endif

#ifdef WIN32
#include <stdio.h>
#define strncasecmp _strnicmp
#define snprintf    _snprintf
#define vsnprintf _vsnprintf
#else
#include <stdlib.h>
#endif

#define HTTPAUTH_DEFAULT_REALM          "TR069 Authenticate"

typedef enum tagE_E_HTTPAUTH_ERR_CODE
{
    E_HTTPAUTH_DIGEST_ERR_realm,
    E_HTTPAUTH_DIGEST_ERR_nonce,
    E_HTTPAUTH_DIGEST_ERR_qop,
    E_HTTPAUTH_DIGEST_ERR_username,
    E_HTTPAUTH_DIGEST_ERR_uri,
    E_HTTPAUTH_DIGEST_ERR_cnonce,
    E_HTTPAUTH_DIGEST_ERR_nc,
    E_HTTPAUTH_DIGEST_ERR_response,
    E_HTTPAUTH_DIGEST_ERR_algorithm,
    E_HTTPAUTH_DIGEST_ERR_opaque,

    // For extention
    E_HTTPAUTH_ERR_PARA,
    E_HTTPAUTH_ERR_SYNTAX,
    E_HTTPAUTH_ERR_MEMORY,

    E_HTTPAUTH_ERR_SYS,

    E_HTTPAUTH_ERR_CREDENTIAL,

    E_HTTPAUTH_ERR_NONE
} E_HTTPAUTH_ERR_CODE;

typedef enum tagHTTPDigest_ConfigBits
{
    HTTPDigest_Config_Qop_Auth                      = 0x0001,
    HTTPDigest_Config_Qop_Auth_Int                  = 0x0002,
    HTTPDigest_Config_Qop_Auth_None                 = 0x0004,
    HTTPDigest_Config_Qop_Mask                      = 0x00FF,

    HTTPDigest_Config_Algorithm_MD5                 = 0x0100,
    HTTPDigest_Config_Algorithm_MD5_sess            = 0x0200,
    HTTPDigest_Config_Algorithm_Mask                = 0xFF00
} HTTPDigest_ConfigBits;

typedef enum tagE_HTTPAUTH_TYPE
{
    E_HTTPAUTH_TYPE_NONE,
    E_HTTPAUTH_TYPE_BASIC,
    E_HTTPAUTH_TYPE_DIGEST
} E_HTTPAUTH_TYPE;


typedef struct tagHTTPAuth_Config
{
    // Used by server, must be set by user
    E_HTTPAUTH_TYPE  eAuthType;
    HTTPDigest_ConfigBits eAbility;

    // Shared by both Basic and Digest authorization, must be set by user
    CHAR *pcRealm;              // Constant, could used by server only, can not be freed
    CHAR *pcUserName;
    CHAR *pcPassword;

    // Should change for each session
    CHAR *pcNonce;

    // Used by client, must be set by user
    CHAR *pcUri;
    CHAR *pcMethod;             // HTTP request method
    LONG lNonceCount;          // For HTTP client only

    // Used by client, parse the WWW-Authenticate header to get this field
    CHAR *pcOpaque;
} HTTPAuth_Config;

#ifdef __cplusplus
extern "C" {
#endif

E_HTTPAUTH_ERR_CODE HTTPAuth_BuildAuthorizationHeader_sj(IN HTTPAuth_Config *pstConfig,
                                                             IN const char *pcAuthenticate,
                                                             OUT char **ppcAuthorization);

E_HTTPAUTH_ERR_CODE HTTPAuth_BuildChallengeHeader(IN HTTPAuth_Config *pstConfig,
                                                  OUT char **ppcAuthenticate);

E_HTTPAUTH_ERR_CODE HTTPAuth_CheckAuthorization(IN HTTPAuth_Config *pstConfig,
                                                      IN const char *pcAuthorization);

LONG LibBase64_Encode(unsigned CHAR* input, const LONG inputlen, unsigned CHAR** output);
LONG LibBase64_Decode(unsigned CHAR* input, const LONG inputlen, unsigned CHAR** output);


typedef enum tagHTTPDigest_FieldIndex
{
    HTTPDigest_Field_realm,
    HTTPDigest_Field_nonce,
    HTTPDigest_Field_qop,
    HTTPDigest_Field_username,
    HTTPDigest_Field_uri,
    HTTPDigest_Field_cnonce,
    HTTPDigest_Field_nc,
    HTTPDigest_Field_response,
    HTTPDigest_Field_algorithm,
    HTTPDigest_Field_opaque,
    HTTPDigest_Field_End
} HTTPDigest_FieldIndex;

typedef char *HTTPDigest_Fields[HTTPDigest_Field_End];

E_HTTPAUTH_ERR_CODE HTTPDigest_ParseAuthHeader(
                                                    IN const CHAR *pcSrc,
                                                    INOUT HTTPDigest_Fields pstDigestFields);

void HTTPDigest_FreeFields(IN HTTPDigest_Fields pstDigestFields);

#ifdef __cplusplus
}
#endif

#endif
