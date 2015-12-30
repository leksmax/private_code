#ifndef __STUN_UCR__
#define __STUN_UCR__


#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif

int STUN_UCR_Check(char *buf, int buf_len, const char *pszUserName, const char *pszUserPwd);

#ifdef __cplusplus
}
#endif

#endif
