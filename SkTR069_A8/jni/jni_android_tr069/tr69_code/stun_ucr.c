#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <ctype.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

//#include "base64.h"
#include "stun_ucr.h"


#include "log.h"
#include "header.h"


#define MAX_UCR_VALUE_LEN (64)
#define DBG_TRACE

typedef struct _tag_UDP_Connection_Request 
{
	char ts[MAX_UCR_VALUE_LEN];	// timestamp, uses to valid ucr.
	char id[MAX_UCR_VALUE_LEN];	// message_id, uses to valid ucr.
	char un[MAX_UCR_VALUE_LEN];	// SerialNumber, uses to valid ucr.
	char cn[MAX_UCR_VALUE_LEN];	// Random string, uses to auth.	
	char sig[MAX_UCR_VALUE_LEN];	// SHA-1 digest, uses to auth.
}STUN_UCR;						// parse from http header.

static STUN_UCR s_lastOKUCR;		// cache the last UCR, uses to valid the incoming one.

static void STUN_UCR_Dump(STUN_UCR *pUCR, char *pchTitle);
static char *STUN_UCR_TrimAll(char *str);
static char *STUN_UCR_ToCase(char *str, bool LowerOrUpper);

static void STUN_UCR_DigestToString(unsigned char *pucDigest, char *pszDigest, int lenDigestString);
static void STUN_UCR_Cache(STUN_UCR *pUCR);
static bool STUN_UCR_Parse(char *buf, int buf_len, STUN_UCR *pUCR);
static bool STUN_UCR_ExtractValue(char *szName, char *szValue, STUN_UCR *pUCR);
static bool STUN_UCR_ParseValuePair(char *pchStart, char *pchEnd, STUN_UCR *pUCR);
static void STUN_UCR_CalcSig(STUN_UCR *pUCR, const char *pszUserPwd, char *pchDigest, int len);
static bool STUN_UCR_Valid(STUN_UCR *pUCR, const char *pszUserName, const char *pszUserPwd);


static void STUN_UCR_Dump(STUN_UCR *pUCR, char *pchTitle)
{
	evcpe_info(__func__, "[%s]", pchTitle);
	evcpe_info(__func__, "\t ts value = [%s]", pUCR->ts);
	evcpe_info(__func__, "\t id value = [%s]", pUCR->id);
	evcpe_info(__func__, "\t un value = [%s]", pUCR->un);
	evcpe_info(__func__, "\t cn value = [%s]", pUCR->cn);
	evcpe_info(__func__, "\t sig value = [%s]", pUCR->sig);
}

static char *STUN_UCR_TrimAll(char *str)
{
        char *p = str;
        char *p1;
        if(p)
        {
                p1 = p + strlen(str) - 1;
                while(*p && isspace(*p)) p++;
                while(p1 > p && isspace(*p1)) 
		{
			*p1 = '\0';
			p1--;
                }	
        }
        return p;
}

static char *STUN_UCR_ToCase(char *str, bool LowerOrUpper)
{
	char *p =str;
	while(p && *p)
	{
		if(LowerOrUpper) 
			*p = tolower(*p);
		else
			*p = toupper(*p);
		p++;
	}
	return str;
}

static void STUN_UCR_DigestToString(unsigned char *pucDigest, char *pszDigest, int lenDigestString)
{
	int i;
	char *p = &(pszDigest[0]);
	memset(pszDigest, '\0', lenDigestString);
		
	for(i = 0; i < 20; i++)
		sprintf(p + i *2, "%02X", pucDigest[i]);

	evcpe_info(__func__, "digest = [%s]", pszDigest);
}


static void STUN_UCR_Cache(STUN_UCR *pUCR)
{
	memcpy(&s_lastOKUCR, pUCR, sizeof(s_lastOKUCR));
	evcpe_info(__func__, "the new ucr is validate, and caches it now");
}

static bool STUN_UCR_Parse(char *buf, int buf_len, STUN_UCR *pUCR)
{
	char *p  = buf, *pEnd;
	if(buf == NULL || buf_len == 0 || pUCR == NULL)
	{
		evcpe_debug(__func__ ,"buf = %p, buf_len = %d, pUCR = %p,  parameter error.", buf, buf_len);
		return false;
	}
	memset(pUCR, '\0', sizeof(*pUCR));
	
	if((p = strstr(p, "GET")) == NULL)	// Don't care the case.
	{
		evcpe_debug(__func__,"cannot find 'GET',  input error.");
		return false;
	}

	if((p = strchr(p, '?')) == NULL)
	{
		evcpe_debug(__func__,"cannot find '?',  input error.");
		return false;
	}
	else
		p++;		// Skip "?"

	while(p < buf +buf_len)
	{
		if((pEnd = strchr(p, '&')) != NULL)
		{
			STUN_UCR_ParseValuePair(p, pEnd, pUCR);	// Don't care the result.
			p = pEnd + 1;
		}
		else
		{
			STUN_UCR_ParseValuePair(p, buf +buf_len, pUCR);	// Don't care the result.
			p = buf +buf_len;
		}
	}
	return true;
}
#if UCR_CALC
static void STUN_UCR_CalcSig(STUN_UCR *pUCR, const char *pszUserPwd, char *pchDigest, int lenDigest)
{
	char in[256], *p;
	unsigned char out[256];
	int len= 0, offset = 0;
	memset(in, '\0', sizeof(in));
	memset(out, '\0', sizeof(out));
	p = &(in[0]);
	// ts
	len = strlen(pUCR->ts);
	memcpy(p +offset, pUCR->ts, len);
	offset += len;
	// id
	len = strlen(pUCR->id);
	memcpy(p + offset, pUCR->id, len);
	offset += len;
	// un
	len = strlen(pUCR->un);
	memcpy(p + offset, pUCR->un, len);
	offset += len;
	// cn
	len = strlen(pUCR->cn);
	memcpy(p + offset, pUCR->cn, len);
	offset += len;
	// HMAC-SHA1
	evcpe_info(__func__, "hmac_sha1 parament: key = [%s], in = [%s] ", pszUserPwd, in);
	if(hmac_sha1(pszUserPwd, strlen(pszUserPwd), in, offset, out) == 0)
	{
		STUN_UCR_DigestToString(out, pchDigest, lenDigest);
	}
}
#endif
static bool STUN_UCR_Valid(STUN_UCR *pUCR, const char *pszUserName, const char *pszUserPwd)
{
	bool bFirstTime = (strlen(s_lastOKUCR.ts) == 0);			// Indicats the first time copmare a incoming UCR or not.
	char szDigest[64];

	if(!bFirstTime)
	{
		if(strcmp(pUCR->ts, s_lastOKUCR.ts) <= 0)
		{
			evcpe_info(__func__,"new ts = %s, last ts = %s,  ignore the UCR.", pUCR->ts, s_lastOKUCR.ts);
			return false;				
		}
		if(strcmp(pUCR->id, s_lastOKUCR.id) == 0)
		{
			evcpe_info(__func__,"new id = %s, last id = %s,  ignore the UCR.", pUCR->id, s_lastOKUCR.id);
			return false;				
		}
	}

	if((pszUserName != NULL) && (strcmp(pUCR->un, pszUserName) != 0))
	{
		evcpe_info(__func__,"new un = %s, user un = %s,  ignore the UCR.", pUCR->un, 
			(pszUserName == NULL) ? "": pszUserName);
		//return false;		
		//modify by lijingchao
		return true;
		//modify end
	}
#if UCR_CALC
	STUN_UCR_CalcSig(pUCR, pszUserPwd, szDigest, 64);
	STUN_UCR_ToCase(szDigest, false);
	STUN_UCR_ToCase(pUCR->sig, false);
	if(strcmp(pUCR->sig, szDigest) != 0)
	{
		evcpe_info(__func__,"sig = %s, calc = %s,  ignore the UCR.", pUCR->sig, szDigest);
		//return false;				
		return true;
	}
#endif
	return true;
}

static bool STUN_UCR_ExtractValue(char *szName, char *szValue, STUN_UCR *pUCR)
{
	char *pchName,  *pchValue, *pchEnd;
	pchName = STUN_UCR_TrimAll(szName);
	pchValue = STUN_UCR_TrimAll(szValue);
	STUN_UCR_ToCase(pchName, true);
	
	if(strcmp(pchName, "ts") == 0)
	{
		strncpy(pUCR->ts, pchValue, MAX_UCR_VALUE_LEN);
		return true;
	}
	if(strcmp(pchName, "id") == 0)
	{
		strncpy(pUCR->id, pchValue, MAX_UCR_VALUE_LEN);
		return true;
	}
	if(strcmp(pchName, "un") == 0)
	{
		strncpy(pUCR->un, pchValue, MAX_UCR_VALUE_LEN);
		return true;
	}
	if(strcmp(pchName, "cn") == 0)
	{
		strncpy(pUCR->cn, pchValue, MAX_UCR_VALUE_LEN);
		return true;
	}
	if(strcmp(pchName, "sig") == 0)
	{
		strncpy(pUCR->sig, pchValue, MAX_UCR_VALUE_LEN);
		return true;
	}
	evcpe_warn(__func__, "name = [%s] value = [%s],  input unknown.", pchName, pchValue);
	return false;
}

#define MAX_PAIR_ITEM_LEN (256)
static bool STUN_UCR_ParseValuePair(char *pchStart, char *pchEnd, STUN_UCR *pUCR)
{
	char szName[MAX_PAIR_ITEM_LEN/2], szValue[MAX_PAIR_ITEM_LEN/2], szPair[MAX_PAIR_ITEM_LEN];
	char *p, *pEnd;
	if((pchStart == NULL) ||(pchEnd == NULL) ||(pchStart >= pchEnd))
	{
		evcpe_debug(__func__,"start = %p, end = %p,  parameter error.", pchStart, pchEnd);
		return false;
	}

	memset(szName, '\0', MAX_PAIR_ITEM_LEN / 2);
	memset(szValue, '\0', MAX_PAIR_ITEM_LEN / 2);
	memset(szPair, '\0', MAX_PAIR_ITEM_LEN);
	strncpy(szPair, pchStart,  pchEnd - pchStart);

	evcpe_info(__func__, "[%s] now", szPair);
	p = szPair;
	if((p = strchr(p, '=')) != NULL)
	{
		strncpy(szName, &(szPair[0]), p -&(szPair[0]));
		strncpy(szValue, p + 1, MAX_PAIR_ITEM_LEN/2);
		if((pEnd = strstr(szValue, "\r\n")) != NULL)
			*pEnd = '\0'; 
		if((pEnd = strchr(szValue, ' ')) != NULL)
			*pEnd = '\0'; 		
		evcpe_info(__func__, "name = [%s] value = [%s]", szName, szValue);
		return STUN_UCR_ExtractValue(szName, szValue, pUCR);
	}
	else
	{
		evcpe_debug(__func__,"cannot find '=',  input error.");
		return false;
	}
}

int STUN_UCR_Check(char *buf, int buf_len, const char *pszUserName, const char *pszUserPwd)
{
	STUN_UCR ucr;
	if(STUN_UCR_Parse(buf, buf_len, &ucr))
	{
		#ifdef DBG_TRACE
			STUN_UCR_Dump(&ucr, "New UCR: ");
			STUN_UCR_Dump(&s_lastOKUCR, "Cached UCR: ");
		#endif	
		if(STUN_UCR_Valid(&ucr, pszUserName, pszUserPwd))
		{
			STUN_UCR_Cache(&ucr);
			return true;
		}
	}
	return false;
}



