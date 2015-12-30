#include "md5.h"
#include "http_author_util.h"
#include "http_digest_calc.h"
#include "str_file.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static const CHAR cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const CHAR cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
static VOID LibEncodeBlock( unsigned CHAR in[3], unsigned CHAR out[4], LONG len )
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned CHAR) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned CHAR) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/* Base64 encode a stream adding padding and line breaks as per spec. */
LONG LibBase64_Encode(unsigned CHAR* input, const LONG inputlen, unsigned CHAR** output)
{
	unsigned CHAR* out;
	unsigned CHAR* in;

    if ((NULL == input) || (NULL == output) || (0 == inputlen))
    {
        if (NULL != output)
        {
            (*output) = NULL;
        }
        return 0;
    }

    *output = (unsigned CHAR*)malloc(((inputlen * 4) / 3) + 5);
    out = *output;
    in  = input;

    if (NULL == (*output))
    {
    	(*output) = NULL;
    	return 0;
    }

    while ((in+3) <= (input+inputlen))
    {
    	LibEncodeBlock(in, out, 3);
    	in += 3;
    	out += 4;
    }
    if ((input+inputlen)-in == 1)
    {
    	LibEncodeBlock(in, out, 1);
    	out += 4;
    }
    else
    if ((input+inputlen)-in == 2)
    {
    	LibEncodeBlock(in, out, 2);
    	out += 4;
    }
    *out = 0;

    return (LONG)(out-*output);
}

/* Decode 4 '6-bit' characters into 3 8-bit binary bytes */
static VOID LibDecodeBlock( unsigned CHAR in[4], unsigned CHAR out[3] )
{
	out[ 0 ] = (unsigned CHAR ) (in[0] << 2 | in[1] >> 4);
	out[ 1 ] = (unsigned CHAR ) (in[1] << 4 | in[2] >> 2);
	out[ 2 ] = (unsigned CHAR ) (((in[2] << 6) & 0xc0) | in[3]);
}

/* decode a base64 encoded stream discarding padding, line breaks and noise */
LONG LibBase64_Decode(unsigned CHAR* input, const LONG inputlen, unsigned CHAR** output)
{
	unsigned CHAR* inptr;
	unsigned CHAR* out;
	unsigned CHAR v;
	unsigned CHAR in[4];
	LONG i, len;

	if (input == NULL || inputlen == 0)
	{
		*output = NULL;
		return 0;
	}

	*output = (unsigned CHAR*)malloc(((inputlen * 3) / 4) + 4);
	out = *output;
	inptr = input;

	while( inptr <= (input+inputlen) )
	{
		for( len = 0, i = 0; i < 4 && inptr <= (input+inputlen); i++ )
		{
			v = 0;
			while( inptr <= (input+inputlen) && v == 0 ) {
				v = (unsigned CHAR) *inptr;
				inptr++;
				v = (unsigned CHAR) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if( v ) {
					v = (unsigned CHAR) ((v == '$') ? 0 : v - 61);
				}
			}
			if( inptr <= (input+inputlen) ) {
				len++;
				if( v ) {
					in[ i ] = (unsigned CHAR) (v - 1);
				}
			}
			else {
				in[i] = 0;
			}
		}
		if( len )
		{
			LibDecodeBlock( in, out );
			out += len-1;
		}
	}
	*out = 0;
	return (LONG)(out-*output);
}


/*
 *  Local strings
 */
// All the names are case insensitive
static char *HTTPDigest_FieldNames[] =
{
    "realm",                // The coresponding value is case-sensitive
    "nonce",                // The coresponding value is case-sensitive
    "qop",                  // The coresponding value is case-sensitive
    "username",         // The coresponding value is case-sensitive
    "uri",                   // The coresponding value is case-sensitive
    "cnonce",             // The coresponding value is case-sensitive
    "nc",                   // The coresponding value is case-sensitive
    "response",         // The coresponding value is case-sensitive
    "algorithm",        // The coresponding value is case-sensitive
    "opaque",           // The coresponding value is case-sensitive
    "stale",              // TRUE: digest is OK, but username/password not OK; FALSE: username/password not OK
    NULL
};

/*
 *  Local definitions
 */
// Case insensitive
#define HTTPDigest_ALGORITHM_MD5                    "MD5"
#define HTTPDigest_ALGORITHM_MD5_SESS               "MD5-sess"

#define HTTPDigest_QOP_AUTH                         "auth"
#define HTTPDigest_QOP_AUTH_INT                     "auth_int"

#define HTTPAUTH_TYPE_BASIC_STR             "Basic"         // Case-insensitive
#define HTTPAUTH_TYPE_BASIC_STR_LEN     5                 // Const length, calculated manually for efficiency
#define HTTPAUTH_TYPE_DIGEST_STR           "Digest"
#define HTTPAUTH_TYPE_DIGEST_STR_LEN   6                 // Const length, calculated manually for efficiency

#define HTTPAuth_FieldNameMaxLen           32
#define HTTPAuth_FieldValueMaxLen          128


#define HTTPDigest_SkipSpace(x) while ((isspace((unsigned char)(*(x)))) && ('\0' != (*(x)))) {(x)++;} \
                                        if ('\0' == (*(x))) {return E_HTTPAUTH_ERR_SYNTAX;}

static E_HTTPAUTH_ERR_CODE HTTPAuth_GenNonce(OUT char **ppcNonce)
{
	static HASHHEX pcNonce={0};
    long  ulNow = 0;
	char  acTimeStr[32+1] = {0};

    HASH acHash;
    MD5_CTX stMd5Ctx;
    CHAR acRand[32+1] = {0};

	//printf("HTTPAuth_GenNonce 1\n");
    if (NULL == ppcNonce)
    {
        return E_HTTPAUTH_ERR_PARA;
    }
	//printf("HTTPAuth_GenNonce 2\n");
    // Time as the seed of random data
    //time(&ulNow);
	ulNow = time(NULL);
    snprintf(acTimeStr, 32, "%ld", ulNow);

    MD5Init(&stMd5Ctx);
    MD5Update(&stMd5Ctx, (unsigned char *)acTimeStr, strlen(acTimeStr),NULL);
    MD5Update(&stMd5Ctx, (unsigned char *)":", 1,NULL);
    snprintf(acRand, 32, "%ld", rand());

    MD5Update(&stMd5Ctx, (unsigned char *)acRand, strlen(acRand),NULL);
    MD5Update(&stMd5Ctx, (unsigned char *)":", 1,NULL);
    MD5Update(&stMd5Ctx, (unsigned char *)HTTPAUTH_DEFAULT_REALM, strlen(HTTPAUTH_DEFAULT_REALM),NULL);

    MD5Final(acHash, &stMd5Ctx);
	
	
	//printf("HTTPAuth_GenNonce 3\n");
    CvtHex(acHash, pcNonce);
	*ppcNonce=pcNonce;

	
    return ((NULL == (*ppcNonce)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE);
}

E_HTTPAUTH_ERR_CODE HTTPDigest_ParseAuthHeader(
                                                    IN const CHAR *pcSrc,
                                                    INOUT HTTPDigest_Fields pstDigestFields)
{
    const CHAR *pcPos;
    int idx;

    char pcKey[HTTPAuth_FieldNameMaxLen]={0};
    char pcValue[HTTPAuth_FieldValueMaxLen]={0};

    if ((NULL == pcSrc) || (NULL == pstDigestFields))
    {
        return E_HTTPAUTH_ERR_PARA;
    }
    memset(pstDigestFields, 0, HTTPDigest_Field_End*sizeof(CHAR *));

    pcPos = pcSrc;
    while ('\0' != (*pcPos))
    {
        // Parse field name
        HTTPDigest_SkipSpace(pcPos);
        idx = 0;
        while (('\0' != (*pcPos)) && ('=' != (*pcPos)) && (!isspace(*pcPos)) && (idx < HTTPAuth_FieldNameMaxLen))
        {
            pcKey[idx] = (*pcPos);
            pcPos++;
            idx++;
        }
        if (('\0' == (*pcPos)) && (idx >= HTTPAuth_FieldNameMaxLen))
        {
            return E_HTTPAUTH_ERR_SYNTAX;
        }
        pcKey[idx] = '\0';

        if ('=' == (*pcPos))
        {
            pcPos++;
        }

        // Parse the field value
        HTTPDigest_SkipSpace(pcPos);
        if ('"' == (*pcPos))            // Skip the "
        {
            pcPos++;
        }
        idx = 0;   //  (!isspace(*pcPos)) &&
        while (('\0' != (*pcPos)) && ('"' != (*pcPos)) && (',' != (*pcPos)) &&
                   (idx < HTTPAuth_FieldValueMaxLen))
        {
            pcValue[idx] = (*pcPos);
            pcPos++;
            idx++;
        }
        if (idx >= HTTPAuth_FieldValueMaxLen)
        {
            return E_HTTPAUTH_ERR_SYNTAX;
        }
        pcValue[idx] = '\0';

        if ('"' == (*pcPos))
        {
            pcPos++;
            while (isspace(*pcPos))
            {
                pcPos++;
            }
        }

        if (',' == (*pcPos))
        {
            pcPos++;
        }

        for (idx = 0; idx < HTTPDigest_Field_End; idx++)
        {
            if (NULL != pstDigestFields[idx])
            {
                continue;
            }
			//printf("index=%d,pcKey=%s,pcValue=%s\n",idx,pcKey,pcValue);
			
            if (0 == strncasecmp(HTTPDigest_FieldNames[idx], pcKey, strlen(HTTPDigest_FieldNames[idx])))
            {
                pstDigestFields[idx] = strdup(pcValue);
                if (NULL == pstDigestFields[idx])
                {
                    return E_HTTPAUTH_ERR_MEMORY;
                }
                break;
            }
        }

        if ('\0' == (*pcPos))
        {
            break;
        }
    }

    return E_HTTPAUTH_ERR_NONE;
}

void HTTPDigest_FreeFields(IN HTTPDigest_Fields pstDigestFields)
{
    int i;
    for (i = 0; i < HTTPDigest_Field_End; i++)
    {
        if (NULL != pstDigestFields[i])
        {
            free(pstDigestFields[i]);
            pstDigestFields[i] = NULL;
        }
    }
}

static E_HTTPAUTH_ERR_CODE HTTPDigest_ServerCheckRequiredFileds(IN HTTPAuth_Config *pstConfig,
                                                            IN HTTPDigest_Fields pstDigestFields)
{
    if ((NULL == pstConfig) || (NULL == pstDigestFields))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    // Check realm field
    if ((NULL == pstConfig->pcRealm) || (NULL == pstDigestFields[HTTPDigest_Field_realm]))  // Must be exist
    {
        return E_HTTPAUTH_DIGEST_ERR_realm;
    }
    if (0 != strcmp(pstConfig->pcRealm, pstDigestFields[HTTPDigest_Field_realm]))
    {
        return E_HTTPAUTH_DIGEST_ERR_realm;
    }

    // Check username field
    if ((NULL == pstConfig->pcUserName) || (NULL == pstDigestFields[HTTPDigest_Field_username]))  // Must be exist
    {
        return E_HTTPAUTH_DIGEST_ERR_username;
    }
    if (0 != strcmp(pstConfig->pcUserName, pstDigestFields[HTTPDigest_Field_username]))
    {
        return E_HTTPAUTH_DIGEST_ERR_username;
    }

    // Check nonce field
    if ((NULL == pstConfig->pcNonce) || (NULL == pstDigestFields[HTTPDigest_Field_nonce]))  // Must be exist
    {
        return E_HTTPAUTH_DIGEST_ERR_nonce;
    }
    if (0 != strcmp(pstConfig->pcNonce, pstDigestFields[HTTPDigest_Field_nonce]))
    {
        return E_HTTPAUTH_DIGEST_ERR_nonce;
    }

    // Check uri
    if ((NULL == pstConfig->pcUri) || (NULL == pstDigestFields[HTTPDigest_Field_uri]))  // Must be exist
    {
        return E_HTTPAUTH_DIGEST_ERR_uri;
    }
    if (0 != strcmp(pstConfig->pcUri, pstDigestFields[HTTPDigest_Field_uri]))
    {
        return E_HTTPAUTH_DIGEST_ERR_uri;
    }

    // Check response field
    if (NULL == pstDigestFields[HTTPDigest_Field_response])  // Must be exist
    {
        return E_HTTPAUTH_DIGEST_ERR_response;
    }

    // Check algorithm field
    if (NULL == pstDigestFields[HTTPDigest_Field_algorithm])
    {
        return E_HTTPAUTH_DIGEST_ERR_algorithm;
    }

    // Check qop field, if qop exists, cnonce and nc fields must also be existing
    if (NULL != pstDigestFields[HTTPDigest_Field_qop])
    {
        if ((NULL == pstDigestFields[HTTPDigest_Field_cnonce]) || (NULL == pstDigestFields[HTTPDigest_Field_nc]))
        {
            return E_HTTPAUTH_DIGEST_ERR_cnonce;
        }
    }

    return E_HTTPAUTH_ERR_NONE;
}

static E_HTTPAUTH_ERR_CODE HTTPDigest_ClientCalcResponse_server(
                                    IN HTTPAuth_Config *pstConfig,
                                    IN HTTPDigest_Fields pstDigestFields,
                                    IN char *pcNonceCount,
                                    OUT char **ppcResponse)
{
    HASHHEX hSessionKey;
    HASHHEX Response;

	//printf("HTTPDigest_ClientCalcResponse 1!\n");
	
    if ((NULL == pstConfig) || (NULL == pstDigestFields) || (NULL == pcNonceCount) || (NULL == ppcResponse))
    {
        return E_HTTPAUTH_ERR_PARA;
    }
    (*ppcResponse) = NULL;

  ///	printf("HTTPDigest_ClientCalcResponse 2.4!\n");

    DigestCalcHA1(pstDigestFields[HTTPDigest_Field_algorithm],
                        pstConfig->pcUserName,
                        pstDigestFields[HTTPDigest_Field_realm],
                        pstConfig->pcPassword,
                        pstDigestFields[HTTPDigest_Field_nonce],
                        pstConfig->pcNonce,
                        hSessionKey);
	//printf("HTTPDigest_ClientCalcResponse 4!\n");

    DigestCalcResponse(
                        hSessionKey,
                        pstDigestFields[HTTPDigest_Field_nonce],
                        pcNonceCount,
                        pstConfig->pcNonce,
                        pstDigestFields[HTTPDigest_Field_qop],
                        pstConfig->pcMethod,
                        pstConfig->pcUri,
                        "",               // auth-int not implemented yet, Entity should be an empty string
                        Response);

    (*ppcResponse) = strdup(Response);

	//printf("HTTPDigest_ClientCalcResponse 5\n");
	
    return (NULL == (*ppcResponse)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
}
static E_HTTPAUTH_ERR_CODE HTTPDigest_ClientCalcResponse(
                                    IN HTTPAuth_Config *pstConfig,
                                    IN HTTPDigest_Fields pstDigestFields,
                                    IN char *pcNonceCount,
                                    OUT char **ppcResponse)
{
    HASHHEX hSessionKey;
    HASHHEX Response;

	//printf("HTTPDigest_ClientCalcResponse 1!\n");
	
    if ((NULL == pstConfig) || (NULL == pstDigestFields) || (NULL == pcNonceCount) || (NULL == ppcResponse))
    {
        return E_HTTPAUTH_ERR_PARA;
    }
//	printf("HTTPDigest_ClientCalcResponse 2,pstConfig->pcNonce=%s!\n",pstConfig->pcNonce);
    (*ppcResponse) = NULL;

    //modify by lijingchao 2014-02-08
    /*因为psConfig->pcNonce 是指向static E_HTTPAUTH_ERR_CODE HTTPAuth_GenNonce(OUT char **ppcNonce) 里面static HASHHEX pcNonce={0};
      在这里判断不为空，并且释放静态变量内存，会存在死机风险。该注释掉。
    */
    /*
    if (NULL != pstConfig->pcNonce)     // If any previous cnonce, release the memory
    {
        free(pstConfig->pcNonce);
        pstConfig->pcNonce = NULL;
    }
    */
    //modify end
    if (E_HTTPAUTH_ERR_NONE != HTTPAuth_GenNonce(&(pstConfig->pcNonce)))       // Client gen cnonce
    {
        return E_HTTPAUTH_ERR_SYS;
    }
//	printf("HTTPDigest_ClientCalcResponse 3!\n");

    DigestCalcHA1(pstDigestFields[HTTPDigest_Field_algorithm],
                        pstConfig->pcUserName,
                        pstDigestFields[HTTPDigest_Field_realm],
                        pstConfig->pcPassword,
                        pstDigestFields[HTTPDigest_Field_nonce],
                        pstConfig->pcNonce,
                        hSessionKey);
//	printf("HTTPDigest_ClientCalcResponse 4!\n");

    DigestCalcResponse(
                        hSessionKey,
                        pstDigestFields[HTTPDigest_Field_nonce],
                        pcNonceCount,
                        pstConfig->pcNonce,
                        pstDigestFields[HTTPDigest_Field_qop],
                        pstConfig->pcMethod,
                        pstConfig->pcUri,
                        "",               // auth-int not implemented yet, Entity should be an empty string
                        Response);

    (*ppcResponse) = strdup(Response);

//	printf("HTTPDigest_ClientCalcResponse 5\n");
	
    return (NULL == (*ppcResponse)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
}

static E_HTTPAUTH_ERR_CODE HTTPDigest_ServerCalcAndCheckResponse(
                                    IN HTTPAuth_Config *pstConfig,
                                    IN HTTPDigest_Fields pstDigestFields)
{
    HASHHEX hSessionKey;
    HASHHEX Response;

    if ((NULL == pstConfig) || (NULL == pstDigestFields) || (NULL == Response))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    DigestCalcHA1(pstDigestFields[HTTPDigest_Field_algorithm],
                        pstConfig->pcUserName,
                        pstConfig->pcRealm,
                        pstConfig->pcPassword,
                        pstConfig->pcNonce,
                        pstDigestFields[HTTPDigest_Field_cnonce],
                        hSessionKey);

    DigestCalcResponse(
                        hSessionKey,
                        pstConfig->pcNonce,
                        pstDigestFields[HTTPDigest_Field_nc],
                        pstDigestFields[HTTPDigest_Field_cnonce],
                        pstDigestFields[HTTPDigest_Field_qop],
                        pstConfig->pcMethod,
                        pstConfig->pcUri,
                        "",               // auth-int not implemented yet, Entity should be an empty string
                        Response);

    if (0 != strcmp(Response, pstDigestFields[HTTPDigest_Field_response]))
    {
        return E_HTTPAUTH_DIGEST_ERR_response;
    }

    return E_HTTPAUTH_ERR_NONE;
}

static E_HTTPAUTH_ERR_CODE HTTPDigest_CheckCredential(IN HTTPAuth_Config *pstConfig,
                                                           IN const char *pcAuthorization)
{
    E_HTTPAUTH_ERR_CODE errCode;
    HTTPDigest_Fields authenticateFields;

    if ((NULL == pstConfig) || (NULL == pcAuthorization))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    errCode = HTTPDigest_ParseAuthHeader(pcAuthorization, authenticateFields);
    if (errCode != E_HTTPAUTH_ERR_NONE)
    {
        HTTPDigest_FreeFields(authenticateFields);
        return errCode;
    }

    errCode = HTTPDigest_ServerCheckRequiredFileds(pstConfig, authenticateFields);
    if (errCode != E_HTTPAUTH_ERR_NONE)
    {
        HTTPDigest_FreeFields(authenticateFields);
        return errCode;
    }

    errCode = HTTPDigest_ServerCalcAndCheckResponse(pstConfig, authenticateFields);
    HTTPDigest_FreeFields(authenticateFields);

    return errCode;
}


/*
 *      HTTP Basic Authorization function utilities
 */

//============================================================
// Function:       HTTPBasic_BuildCredential
// Description:
//                     Build credential from username and password for HTTP Basic authorization
// Input:
//                      pcUsername:            Username for authorization
//                      pcPassword:             Password for authorization
// Output:
//                      ppcAuthorization:      Credential built for authorization
// Return:
//                     E_HTTPAUTH_ERR_CODE
//============================================================
static E_HTTPAUTH_ERR_CODE HTTPBasic_BuildCredential(IN char *pcUsername,
                                                        IN char *pcPassword, OUT char **ppcAuthorization)
{
    int iSize;
    char acBuffer[HTTPAuth_FieldValueMaxLen];

    if ((NULL == pcUsername) || (NULL || pcPassword) || (NULL == ppcAuthorization))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    (*ppcAuthorization) = NULL;

    // Using Base64 to encode
    iSize = snprintf(acBuffer, sizeof(acBuffer), "%s:%s", pcUsername, pcPassword);
    if (iSize >= sizeof(acBuffer))
    {
        return E_HTTPAUTH_ERR_MEMORY;
    }
    LibBase64_Encode(acBuffer, iSize, ppcAuthorization);

    (*ppcAuthorization) = strdup(acBuffer);

    return ((NULL == (*ppcAuthorization)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE);
}

static E_HTTPAUTH_ERR_CODE HTTPBasic_CheckCredential(IN HTTPAuth_Config *pstConfig,
                                                          IN const char *pcAuthorization)
{
    E_HTTPAUTH_ERR_CODE retCode;
    char *pcCredentical = NULL;

    if ((NULL == pstConfig) || (NULL == pcAuthorization))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    retCode = HTTPBasic_BuildCredential(pstConfig->pcUserName, pstConfig->pcPassword, &pcCredentical);
    if (E_HTTPAUTH_ERR_NONE != retCode)
    {
        return retCode;
    }

    while (isspace(*pcAuthorization))           // Skip the white spaces
    {
        pcAuthorization++;
    }

    if (0 == strcmp(pcAuthorization, pcCredentical))
    {
        free(pcCredentical);
        return E_HTTPAUTH_ERR_NONE;
    }

    free(pcCredentical);
    return E_HTTPAUTH_ERR_CREDENTIAL;
}

E_HTTPAUTH_ERR_CODE HTTPAuth_GetAuthResponse(IN HTTPAuth_Config *pstConfig,HTTPDigest_Fields digestFields,
                                                        OUT char **ppResponse)
{
	E_HTTPAUTH_ERR_CODE errCode;
    
    char nonceCnt[9]={0};      // nonce count value is 8LHEX

	*ppResponse=NULL;
	//printf("aaaaaaaaaaaaaa\n");
	snprintf(nonceCnt, sizeof(nonceCnt), "%08x", pstConfig->lNonceCount);
	//printf("bbbbbbbbbbbb\n");


    /*printf("pstConfig.pcMethod:%s\n",pstConfig->pcMethod);
	printf("pstConfig.pcUserName:%s\n",pstConfig->pcUserName);
   	printf("pstConfig.pcPassword:%s\n",pstConfig->pcPassword);
   	printf("pstConfig.pcNonce:%s\n",pstConfig->pcNonce);
   	printf("pstConfig.pcUri:%s\n",pstConfig->pcUri);
	*/

	errCode = HTTPDigest_ClientCalcResponse_server(pstConfig, digestFields, nonceCnt, ppResponse);
    if (E_HTTPAUTH_ERR_NONE != errCode)
    {
//		printf("ccccccccccccccc\n");
        HTTPDigest_FreeFields(digestFields);
        return errCode;
    }
//	printf("dddddddddddddd\n");
//	HTTPDigest_FreeFields(digestFields);
//	printf("eeeeeeeeeeeeeeeeee\n");
 	return (NULL == (*ppResponse)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
}



/*
 *      Interfaces for HTTP client for authorization
 */

//============================================================
// Function:       HTTPAuth_BuildAuthorizationHeader
// Description:
//                     Build Authorization header for HTTP clients. For public usage
//                              Support both Basic and Digest
// Input:
//                     pstConfig:           Configuration structure for HTTP clients
//                     pcAuthenticate:      WWW-Authenticate header sent by HTTP server
// Output:
//                     ppcAuthorization:    The Authorization header build for the response
// Return:
//                     E_HTTPAUTH_ERR_CODE
//============================================================

E_HTTPAUTH_ERR_CODE HTTPAuth_BuildAuthorizationHeader_sj(IN HTTPAuth_Config *pstConfig,
                                                        IN const char *pcAuthenticate,
                                                        OUT char **ppcAuthorization)
{
    StrFile *pstBuffer = NULL;
    E_HTTPAUTH_ERR_CODE errCode = E_HTTPAUTH_ERR_NONE;
    char *pcCredentical = NULL ;
    HTTPDigest_Fields digestFields = {0};
    char nonceCnt[9]= {0};      // nonce count value is 8LHEX
   // hjadgahlgkj;
   //evcpe_error(__func__, "HTTPAuth_BuildAuthorizationHeader to add header: \n");
    if ((NULL == pstConfig) || (NULL == pcAuthenticate) || (NULL == ppcAuthorization))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

	//printf("HTTPAuth_BuildAuthorizationHeader 2!\n");	
	
    (*ppcAuthorization) = NULL;

    //1 Basic authorization
    if (0 == strncasecmp(pcAuthenticate, HTTPAUTH_TYPE_BASIC_STR, HTTPAUTH_TYPE_BASIC_STR_LEN))   // Basic authorization
    {
        errCode = HTTPBasic_BuildCredential(pstConfig->pcUserName, pstConfig->pcPassword, &pcCredentical);
        if (E_HTTPAUTH_ERR_NONE != errCode)
        {
            return errCode;
        }

        pstBuffer = LibStrFile_Open(-1);
        if (NULL == pstBuffer)
        {
            return E_HTTPAUTH_ERR_MEMORY;
        }

        LibStrFile_Printf(pstBuffer, "%s %s", HTTPAUTH_TYPE_BASIC_STR, pcCredentical);
        (*ppcAuthorization) = strdup(LibStrFile_Buffer(pstBuffer));
        LibStrFile_Close(pstBuffer);
        return (NULL == (*ppcAuthorization)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
    }
	//printf("HTTPAuth_BuildAuthorizationHeader 3!\n");
    if (0 != strncasecmp(pcAuthenticate, HTTPAUTH_TYPE_DIGEST_STR, HTTPAUTH_TYPE_DIGEST_STR_LEN))   // Digest authorization
    {
        return E_HTTPAUTH_ERR_SYNTAX;
    }
	//printf("HTTPAuth_BuildAuthorizationHeader 4!\n");
    //1 Digest authorization
    pcAuthenticate += HTTPAUTH_TYPE_DIGEST_STR_LEN;

    //2 Parse the WWW-Authenticate header
    errCode = HTTPDigest_ParseAuthHeader(pcAuthenticate, digestFields);
    if (E_HTTPAUTH_ERR_NONE != errCode)
    {
        return errCode;
    }

	//printf(" digestFields[HTTPDigest_Field_realm]=%s\n", digestFields[HTTPDigest_Field_realm]);
	//printf(" digestFields[HTTPDigest_Field_nonce]=%s\n", digestFields[HTTPDigest_Field_nonce]);
		
	//printf("HTTPAuth_BuildAuthorizationHeader 5!\n");
    //2 Check required fields
    if ((NULL == digestFields[HTTPDigest_Field_realm]) ||
        (NULL == digestFields[HTTPDigest_Field_nonce]) ||
        (NULL == pstConfig->pcMethod) ||
        (NULL == pstConfig->pcUri) ||
        (NULL == pstConfig->pcUserName) ||
        (NULL == pstConfig->pcPassword))
    {

		//printf(" digestFields[HTTPDigest_Field_realm]=%s\n", digestFields[HTTPDigest_Field_realm]);
		//printf(" digestFields[HTTPDigest_Field_nonce]=%s\n", digestFields[HTTPDigest_Field_nonce]);

		//printf(" pstConfig->pcMethod=%s\n", pstConfig->pcMethod);
		//printf(" pstConfig->pcUri=%s\n", pstConfig->pcUri);
		//printf(" pstConfig->pcUserName=%s\n", pstConfig->pcUserName);
		//printf(" pstConfig->pcPassword=%s\n", pstConfig->pcPassword);
		


	
        HTTPDigest_FreeFields(digestFields);
        return E_HTTPAUTH_ERR_SYNTAX;
    }
	//printf("HTTPAuth_BuildAuthorizationHeader 6!\n");
    //2 Calculate the response field
    snprintf(nonceCnt, sizeof(nonceCnt), "%08x", pstConfig->lNonceCount);

    errCode = HTTPDigest_ClientCalcResponse(pstConfig, digestFields, nonceCnt, &pcCredentical);
    if (E_HTTPAUTH_ERR_NONE != errCode)
    {
        HTTPDigest_FreeFields(digestFields);
        return errCode;
    }
	//printf("HTTPAuth_BuildAuthorizationHeader 7!\n");
    pstBuffer = LibStrFile_Open(-1);
    if (NULL == pstBuffer)
    {
        HTTPDigest_FreeFields(digestFields);
        free(pcCredentical);
        return E_HTTPAUTH_ERR_MEMORY;
    }
	//printf("HTTPAuth_BuildAuthorizationHeader 8!\n");	
    //2 Build username, realm, nonce and uri fields
    LibStrFile_Printf(pstBuffer, "%s %s=\"%s\", %s=\"%s\", %s=\"%s\", %s=\"%s\", ",
                        HTTPAUTH_TYPE_DIGEST_STR,
                        HTTPDigest_FieldNames[HTTPDigest_Field_username], pstConfig->pcUserName,
                        HTTPDigest_FieldNames[HTTPDigest_Field_realm], digestFields[HTTPDigest_Field_realm],
                        HTTPDigest_FieldNames[HTTPDigest_Field_nonce], digestFields[HTTPDigest_Field_nonce],
                        HTTPDigest_FieldNames[HTTPDigest_Field_uri], pstConfig->pcUri);

    //2 Build the qop, nc and cnonce fields
    if (NULL != digestFields[HTTPDigest_Field_qop])    // Need to fill the nc, cnonce and qop fields
    {
        LibStrFile_Printf(pstBuffer, "%s=%s, %s=\"%s\", %s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_nc], nonceCnt,
                HTTPDigest_FieldNames[HTTPDigest_Field_cnonce], pstConfig->pcNonce,
                HTTPDigest_FieldNames[HTTPDigest_Field_qop], digestFields[HTTPDigest_Field_qop]);
    }

    //2 Build the algorithm field
    if (NULL != digestFields[HTTPDigest_Field_algorithm])    // Need to fill the algorithm field
    {
        LibStrFile_Printf(pstBuffer, "%s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_algorithm], digestFields[HTTPDigest_Field_algorithm]);
    }

	  if (NULL != digestFields[HTTPDigest_Field_opaque])    //opaque
    {
        LibStrFile_Printf(pstBuffer, "%s=\"%s\", ",               
                HTTPDigest_FieldNames[HTTPDigest_Field_opaque], digestFields[HTTPDigest_Field_opaque]);
    }
    // Build the response field
    LibStrFile_Printf(pstBuffer, "%s=\"%s\"",
                HTTPDigest_FieldNames[HTTPDigest_Field_response], pcCredentical);
	//printf("HTTPAuth_BuildAuthorizationHeader 8!\n");
	
    HTTPDigest_FreeFields(digestFields);
    free(pcCredentical);

    (*ppcAuthorization) = strdup(LibStrFile_Buffer(pstBuffer));
    LibStrFile_Close(pstBuffer);
    return (NULL == (*ppcAuthorization)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
}

/*
 *      Interfaces for HTTP server
 *          HTTPAuth_BuildChallenge: Build challenge for HTTP client
 *              for the first HTTP request with no Authorization information
 *          HTTPAuth_CheckAuthorization: Check the Authorization header of HTTP client
 */

E_HTTPAUTH_ERR_CODE HTTPAuth_BuildChallengeHeader(IN HTTPAuth_Config *pstConfig,
                                                  OUT char **ppcAuthenticate)
{
    StrFile *pstBuffer;
    E_HTTPAUTH_ERR_CODE errCode;

    if ((NULL == ppcAuthenticate) || (NULL == pstConfig))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    pstBuffer = LibStrFile_Open(-1);
    if (NULL == pstBuffer)
    {
        return E_HTTPAUTH_ERR_MEMORY;
    }

    //1 Basic authorization
    if (E_HTTPAUTH_TYPE_BASIC == pstConfig->eAuthType)      // Basic authorization
    {
        LibStrFile_Printf(pstBuffer, "%s %s=\"%s\"", HTTPAUTH_TYPE_BASIC_STR,
                        HTTPDigest_FieldNames[HTTPDigest_Field_realm], HTTPAUTH_DEFAULT_REALM);
        (*ppcAuthenticate) = strdup(LibStrFile_Buffer(pstBuffer));
        LibStrFile_Close(pstBuffer);
        return (NULL == (*ppcAuthenticate)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
    }

    //1 Digest authorization

    //2 Build realm field
    LibStrFile_Printf(pstBuffer, "%s %s=\"%s\", ", HTTPAUTH_TYPE_DIGEST_STR,
                        HTTPDigest_FieldNames[HTTPDigest_Field_realm], HTTPAUTH_DEFAULT_REALM);

    //2 Build nonce field
    if (NULL != pstConfig->pcNonce)
    {
        free(pstConfig->pcNonce);
    }
    errCode = HTTPAuth_GenNonce(&(pstConfig->pcNonce));
    if (E_HTTPAUTH_ERR_NONE != errCode)
    {
        LibStrFile_Close(pstBuffer);
        return errCode;
    }
    LibStrFile_Printf(pstBuffer, "%s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_nonce], pstConfig->pcNonce);

    //2 Build qop field
    if (0 != (pstConfig->eAbility & HTTPDigest_Config_Qop_Auth))
    {
        LibStrFile_Printf(pstBuffer, "%s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_qop], HTTPDigest_QOP_AUTH);
    }
    else if (0 != (pstConfig->eAbility & HTTPDigest_Config_Qop_Auth_Int))
    {
        LibStrFile_Printf(pstBuffer, "%s=\"%s\", ",
                HTTPDigest_FieldNames[HTTPDigest_Field_qop], HTTPDigest_QOP_AUTH_INT);
    }

    //2 Build algorithm field
    if (0 != (pstConfig->eAbility & HTTPDigest_Config_Algorithm_MD5_sess))
    {
        LibStrFile_Printf(pstBuffer, "%s=\"%s\"",
                HTTPDigest_FieldNames[HTTPDigest_Field_algorithm], HTTPDigest_ALGORITHM_MD5_SESS);
    }
    else
    {
        LibStrFile_Printf(pstBuffer, "%s=\"%s\"",
                HTTPDigest_FieldNames[HTTPDigest_Field_algorithm], HTTPDigest_ALGORITHM_MD5);
    }

    (*ppcAuthenticate) = strdup(LibStrFile_Buffer(pstBuffer));
    LibStrFile_Close(pstBuffer);
    return (NULL == (*ppcAuthenticate)) ? E_HTTPAUTH_ERR_MEMORY : E_HTTPAUTH_ERR_NONE;
}

//============================================================
// Function:       HTTPAuth_CheckAuthorization
// Description:
//                     Check the Authorization header sent from an HTTP client
//                              Support both Basic and Digest
// Input:
//                      pstConfig:                  Configuration structure for HTTP server
//                      pcAuthorization:          The Authorization header sent from HTTP client
// Output:
// Return:
//                     E_HTTPAUTH_ERR_CODE
//============================================================
E_HTTPAUTH_ERR_CODE HTTPAuth_CheckAuthorization(IN HTTPAuth_Config *pstConfig,
                                                      IN const char *pcAuthorization)
{
    if ((NULL == pcAuthorization) || (NULL == pstConfig))
    {
        return E_HTTPAUTH_ERR_PARA;
    }

    if (E_HTTPAUTH_TYPE_NONE == pstConfig->eAuthType)
    {
        return E_HTTPAUTH_ERR_NONE;
    }

    if (0 == strncasecmp(pcAuthorization, HTTPAUTH_TYPE_DIGEST_STR, HTTPAUTH_TYPE_DIGEST_STR_LEN))
    {
        if (E_HTTPAUTH_TYPE_DIGEST != pstConfig->eAuthType)   // Should be configured as Digest authorization
        {
            return E_HTTPAUTH_ERR_SYNTAX;
        }
        pcAuthorization += HTTPAUTH_TYPE_DIGEST_STR_LEN;
        return HTTPDigest_CheckCredential(pstConfig, pcAuthorization);
    }
    else if (0 == strncasecmp(pcAuthorization, HTTPAUTH_TYPE_BASIC_STR, HTTPAUTH_TYPE_BASIC_STR_LEN))
    {
        if (E_HTTPAUTH_TYPE_BASIC != pstConfig->eAuthType)    // Should be configured as Basic authorization
        {
            return E_HTTPAUTH_ERR_SYNTAX;
        }
        pcAuthorization += HTTPAUTH_TYPE_BASIC_STR_LEN;
        return HTTPBasic_CheckCredential(pstConfig, pcAuthorization);
    }

    return E_HTTPAUTH_ERR_SYNTAX;
}



