#ifndef _FTP_H_
#define _FTP_H_

#define IN
#define OUT

typedef struct _ftpNode{
    int serverDataport;
    int serverCmdport;
    char* serverip;
    char* replyBuf;
    char* filepath1;
    char* user;
    char* passwd;
    int sockcmd;
    int sockdata;
} FTPNODE;

void ftpInit(FTPNODE*);
FTPNODE* ftpNodeCreate();
int ftpStart(FTPNODE*);
void* ftpTask(void* arg);
int ftpPost(FTPNODE*);
int ftpOpen(FTPNODE*);
int ftpPutBegin(FTPNODE*);
int ftpTransferData(FTPNODE*, char*, int);
int tcpConnect(int, char*, int);
int tcpDisconnect(int);
int tcpSendData(int, char*, int);
int sendFtpCmd(FTPNODE *, char *, char *);
int recvFtpServerReply(FTPNODE*);
int parseReplyToCode(FTPNODE*);

#endif