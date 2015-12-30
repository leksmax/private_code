#include "http_author_util.h"
#include "md5.h"
#include "http_digest_calc.h"
#include <string.h>

void CvtHex(IN HASH Bin, OUT HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++)
    {
        // Higher 4 bits
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
        {
            Hex[i*2] = (j + '0');
        }
        else
        {
            Hex[i*2] = (j + 'a' - 10);
        }

        // Lower 4 bits
        j = Bin[i] & 0xf;
        if (j <= 9)
        {
            Hex[i*2+1] = (j + '0');
        }
        else
        {
            Hex[i*2+1] = (j + 'a' - 10);
        }
    };
    Hex[HASHHEXLEN] = '\0';
};

/* calculate H(A1) as per spec */
void DigestCalcHA1(
                                    IN char * pszAlg,
                                    IN char * pszUserName,
                                    IN char * pszRealm,
                                    IN char * pszPassword,
                                    IN char * pszNonce,
                                    IN char * pszCNonce,
                                    OUT HASHHEX SessionKey
                                    )
{
    MD5_CTX Md5Ctx;
    HASH HA1;

    MD5Init(&Md5Ctx);
    MD5Update(&Md5Ctx, pszUserName, strlen(pszUserName),NULL);
    MD5Update(&Md5Ctx, ":", 1,NULL);
    MD5Update(&Md5Ctx, pszRealm, strlen(pszRealm),NULL);
    MD5Update(&Md5Ctx, ":", 1,NULL);
    MD5Update(&Md5Ctx, pszPassword, strlen(pszPassword),NULL);
    MD5Final(HA1, &Md5Ctx);

    // Calculate SessionKey if use the MD5-sess algorithm
    if ((NULL != pszAlg) && (0 == strcmp(pszAlg, "md5-sess")))
    {
        MD5Init(&Md5Ctx);
        MD5Update(&Md5Ctx, HA1, HASHLEN,NULL);
        MD5Update(&Md5Ctx, ":", 1,NULL);
        MD5Update(&Md5Ctx, pszNonce, strlen(pszNonce),NULL);
        MD5Update(&Md5Ctx, ":", 1,NULL);
        MD5Update(&Md5Ctx, pszCNonce, strlen(pszCNonce),NULL);
        MD5Final(HA1, &Md5Ctx);
    };

    CvtHex(HA1, SessionKey);
};

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
                                    IN HASHHEX HA1,           /* H(A1) */
                                    IN char * pszNonce,       /* nonce from server */
                                    IN char * pszNonceCount,  /* 8 hex digits */
                                    IN char * pszCNonce,      /* client nonce */
                                    IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
                                    IN char * pszMethod,      /* method from the request */
                                    IN char * pszDigestUri,   /* requested URL */
                                    IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
                                    OUT HASHHEX Response      /* request-digest or response-digest */
                                    )
{
    MD5_CTX Md5Ctx;
    HASH HA2;
    HASH RespHash;
    HASHHEX HA2Hex;

    // Calculate H(A2)
    MD5Init(&Md5Ctx);
    MD5Update(&Md5Ctx, pszMethod, strlen(pszMethod),NULL);
    MD5Update(&Md5Ctx, ":", 1,NULL);
    MD5Update(&Md5Ctx, pszDigestUri, strlen(pszDigestUri),NULL);
    if ((NULL != pszQop) && (0 == strcmp(pszQop, "auth-int")))
    {
        MD5Update(&Md5Ctx, ":", 1,NULL);
        MD5Update(&Md5Ctx, HEntity, HASHHEXLEN,NULL);
    };
    MD5Final(HA2, &Md5Ctx);
    CvtHex(HA2, HA2Hex);

    // Calculate response
    MD5Init(&Md5Ctx);
    MD5Update(&Md5Ctx, HA1, HASHHEXLEN,NULL);
    MD5Update(&Md5Ctx, ":", 1,NULL);
    MD5Update(&Md5Ctx, pszNonce, strlen(pszNonce),NULL);
    MD5Update(&Md5Ctx, ":", 1,NULL);
    if ((NULL != pszQop) && ('\0' != (*pszQop)))     // If qop exist, need to digest nc and cnonce
    {
        MD5Update(&Md5Ctx, pszNonceCount, strlen(pszNonceCount),NULL);
        MD5Update(&Md5Ctx, ":", 1,NULL);
        MD5Update(&Md5Ctx, pszCNonce, strlen(pszCNonce),NULL);
        MD5Update(&Md5Ctx, ":", 1,NULL);
        MD5Update(&Md5Ctx, pszQop, strlen(pszQop),NULL);
        MD5Update(&Md5Ctx, ":", 1,NULL);
    };
    MD5Update(&Md5Ctx, HA2Hex, HASHHEXLEN,NULL);
    MD5Final(RespHash, &Md5Ctx);
    CvtHex(RespHash, Response);
};

