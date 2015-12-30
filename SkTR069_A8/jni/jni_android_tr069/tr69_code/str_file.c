#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "http_author_util.h"
#include "str_file.h"

/*
 *  String File Structure
 */
#define DEFAULT_STRFILE_SIZE                1024
#define DEFAULT_STRFILE_INC_SIZE        1024

typedef enum
{
    STRFILE_STRING,
    STRFILE_FILE
}STRFILE_TYPE_E;

 struct tagStrFile
{
    STRFILE_TYPE_E eType;
    LONG                 lBytesWritten;
    union
    {
        FILE*             fFilePtr;
        struct
        {
            char*         pcStrBuf;
            LONG         lBytesAvail;
        }stStr;
    }uIO;
};

StrFile *LibStrFile_Open(LONG lInitSize)
{
    StrFile *RetVal = (StrFile *)malloc(sizeof(StrFile));

    if (RetVal == NULL)
    {
        return NULL;
    }

    RetVal->eType = STRFILE_STRING;
    RetVal->lBytesWritten = 0;

    RetVal->uIO.stStr.lBytesAvail = (lInitSize <= 0) ? DEFAULT_STRFILE_SIZE : lInitSize;
    RetVal->uIO.stStr.pcStrBuf = (CHAR *)malloc(RetVal->uIO.stStr.lBytesAvail);
    if (RetVal->uIO.stStr.pcStrBuf == NULL)
    {
        free(RetVal);
        return NULL;
    }

    return RetVal;
}

StrFile *LibStrFile_OpenEx(CHAR *pcInitBuf)
{
    StrFile *RetVal;

    if (pcInitBuf == NULL)
    {
        return NULL;
    }

    RetVal = (StrFile *)malloc(sizeof(StrFile));
    if (RetVal == NULL)
    {
        return NULL;
    }

    RetVal->eType = STRFILE_STRING;
    RetVal->lBytesWritten = strlen(pcInitBuf);

    RetVal->uIO.stStr.lBytesAvail = 0;
    RetVal->uIO.stStr.pcStrBuf = pcInitBuf;

    return RetVal;
}

StrFile *LibStrFile_FOpen(IN CHAR *pcFileName)
{
    StrFile *RetVal;

    if (pcFileName == NULL)
    {
        return NULL;
    }

    RetVal = (StrFile *)malloc(sizeof(StrFile));
    if (RetVal == NULL)
        return NULL;

    RetVal->eType = STRFILE_FILE;
    RetVal->lBytesWritten = 0;
    RetVal->uIO.fFilePtr = fopen(pcFileName, "wb");
    if (RetVal->uIO.fFilePtr == NULL)
    {
        free(RetVal);
        return NULL;
    }

    return RetVal;
}

int LibStrFile_ReSize(StrFile *file, ULONG ulNewSize)
{

    if ((file == NULL) || (file->eType == STRFILE_FILE))
    {
        return TRUE;
    }

    if (file->uIO.stStr.pcStrBuf != NULL)
    {
        free(file->uIO.stStr.pcStrBuf);
        //return FALSE;
    }

    file->uIO.stStr.pcStrBuf = malloc(ulNewSize);
    if (file->uIO.stStr.pcStrBuf == NULL)
    {
        printf("LibStrFile_ReSize Resize Failed...\n");
        return FALSE;
    }
    file->uIO.stStr.lBytesAvail = ulNewSize;
    file->lBytesWritten = 0;
    printf("LibStrFile_ReSize Resize To %d...\n", ulNewSize);

    return TRUE;
}

int LibStrFile_WriteBin(StrFile *file, CHAR *pcSrc, ULONG ulLen)
{
    if ((file == NULL) || (pcSrc == NULL))
    {
        return FALSE;
    }

    if (file->eType == STRFILE_FILE)
    {
        if (file->uIO.fFilePtr == NULL)
        {
            return FALSE;
        }
        if (fwrite(pcSrc, ulLen, 1, file->uIO.fFilePtr) == 1)
        {
            file->lBytesWritten += ulLen;
            return TRUE;
        }
        else
        {
            printf("Write Bin Failed...\n");
            return FALSE;
        }
    }
    else if (file->eType == STRFILE_STRING)
    {
        if (file->uIO.stStr.lBytesAvail <= (int)ulLen)
        {
            return FALSE;
        }
        else
        {
            memcpy(file->uIO.stStr.pcStrBuf+file->lBytesWritten, pcSrc, ulLen);
            file->lBytesWritten += ulLen;
            file->uIO.stStr.lBytesAvail -= ulLen;
            //printf("Write Bin %d\n", ulLen);
            return TRUE;
        }
    }

    return TRUE;
}

LONG LibStrFile_Printf(StrFile *file, CHAR *fmt, ...)
{
    va_list ap;
    LONG len, n = 0;
    CHAR *buf;

    if (file == NULL)
    {
        return 0;
    }

    va_start(ap, fmt);
    if (file->eType == STRFILE_STRING)
    {
        while (1)
        {
            buf = &(file->uIO.stStr.pcStrBuf[file->lBytesWritten]);
            len = file->uIO.stStr.lBytesAvail - file->lBytesWritten;
            n = vsnprintf(buf, len, fmt, ap);
            if (n < 0 || n >= len)
            {
                /* we tried to overwrite the end of the buffer */
                buf = realloc(file->uIO.stStr.pcStrBuf, file->uIO.stStr.lBytesAvail
                                    + DEFAULT_STRFILE_INC_SIZE);
                if (buf)
                {
                    file->uIO.stStr.pcStrBuf = buf;
                    file->uIO.stStr.lBytesAvail += DEFAULT_STRFILE_INC_SIZE;
                    continue;
                }
            }
            else
            {
                file->lBytesWritten += n;
            }
            break;
        } /* end while */
    }
    else if (file->eType == STRFILE_FILE)
    {
        n = vfprintf(file->uIO.fFilePtr, fmt, ap);
        if (n > 0)
        {
            file->lBytesWritten += n;
        }
    }
    va_end(ap);

    return n;
}

XINLINE VOID LibStrFile_Close(StrFile *file)
{
    if (file)
    {
        if (file->eType == STRFILE_STRING)
        {
            if (file->uIO.stStr.pcStrBuf)
            {
                free(file->uIO.stStr.pcStrBuf);
            }
        }
        else if (file->eType == STRFILE_FILE)
        {
            fflush(file->uIO.fFilePtr);
            fclose(file->uIO.fFilePtr);
        }
        free(file);
    }
}

XINLINE LONG LibStrFile_Tell(StrFile *file)
{
    if (file == NULL)
    {
        return 0;
    }

    return file->lBytesWritten;
}

XINLINE CHAR* LibStrFile_Buffer(StrFile *file)
{
    if ((file != NULL) && (file->eType == STRFILE_STRING))
    {
        return file->uIO.stStr.pcStrBuf;
    }

    return NULL;
}

XINLINE VOID LibStrFile_Flush(StrFile *file)
{
    if (file == NULL)
    {
        return;
    }

    if (file->eType == STRFILE_STRING)
    {
        file->lBytesWritten = 0;
    }
    else if (file->eType == STRFILE_FILE)
    {
        fflush(file->uIO.fFilePtr);
    }
}

XINLINE ULONG LibStrFile_FileToBuf(CHAR *pcFileName, CHAR** ppcBuf)
{
    FILE* pfFile = NULL;
    ULONG   ulLen = 0;
    
    if (pcFileName == NULL)
    {
        return ulLen;
    }
    pfFile = fopen(pcFileName, "r");
    if(NULL == pfFile)
    {
        return ulLen;
    }
    fseek(pfFile, 0, SEEK_END);
    ulLen = ftell(pfFile);
    fseek(pfFile, 0, SEEK_SET);
    
    *ppcBuf = malloc(ulLen+8);
    memset(*ppcBuf, 0, ulLen+8);
    
    if(*ppcBuf)
    {
        fread(*ppcBuf, ulLen, 1, pfFile);
        (*ppcBuf)[ulLen] = 0;
    }
    fclose(pfFile);
    return ulLen+8;
}

