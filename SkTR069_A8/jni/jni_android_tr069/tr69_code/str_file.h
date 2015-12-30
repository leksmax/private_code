#ifndef _STRFILE_H_
#define _STRFILE_H_

#ifdef WIN32
#include <stdio.h>
#define strncasecmp _strnicmp
#define snprintf    _snprintf
#define vsnprintf _vsnprintf
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct tagStrFile;

typedef struct tagStrFile StrFile;

StrFile *LibStrFile_Open(LONG lInitSize);

StrFile *LibStrFile_OpenEx(CHAR *pcInitBuf);

StrFile *LibStrFile_FOpen(IN CHAR *pcFileName);

XINLINE VOID LibStrFile_Close(StrFile *file);

int LibStrFile_ReSize(StrFile *file, ULONG ulNewSize);

int LibStrFile_WriteBin(StrFile *file, CHAR *pcSrc, ULONG ulLen);

LONG LibStrFile_Printf(StrFile *file, CHAR *fmt, ...);

XINLINE LONG LibStrFile_Tell(StrFile *file);

XINLINE CHAR* LibStrFile_Buffer(StrFile *file);

XINLINE VOID LibStrFile_Flush(StrFile *file);

XINLINE ULONG LibStrFile_FileToBuf(CHAR *pcFileName, CHAR** ppcBuf);

#ifdef __cplusplus
}
#endif

#endif

