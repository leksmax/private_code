#include "stdio.h"
#include "ftp.h"
#include "pthread.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include "myHeadFile.h"

#define FILEPATH "wangjie.txt"
#define USER "wangjie"
#define PASSWD "1"
#define SERVERIP "192.168.52.128"
#define SERVERCMDPORT 21

static int ftpInit(
    OUT FTPNODE* ftpStruct)
{
    char* p = (char*)malloc(100 * 1024);
    ftpStruct->filepath1 = FILEPATH;
    ftpStruct->user = USER;
    ftpStruct->passwd = PASSWD;
    ftpStruct->serverDataport = -1;
    ftpStruct->serverCmdport = SERVERCMDPORT;
    ftpStruct->serverip = SERVERIP;
    ftpStruct->replyBuf = p;
    ftpStruct->sockcmd = -1;
    ftpStruct->sockdata = -1;
    return 0;
}

static int ftpUnload(
    IN FTPNODE* ftpStruct)
{
    if (ftpStruct->replyBuf){
        free(ftpStruct->replyBuf);
        ftpStruct->replyBuf = NULL;
    }
    if (ftpStruct){
        free(ftpStruct);
        ftpStruct = NULL;
    }
    return 0;
}

static void* ftpTask(
    IN void* arg)
{
    FTPNODE* ftpNode = (FTPNODE*)arg;
    ftpInit(ftpNode);
    while (1){
        printf("[%s,%d]\n", __FILE__, __LINE__);
        ftpPost(ftpNode);
        sleep(5); 
    }
    ftpUnload(ftpNode);
}

static int ftpOpen(
    IN FTPNODE * ftpStruct)
{
    char tmpBuf[1024] = {0};
    int ret = -1;
    
    ftpStruct->sockcmd = socket(AF_INET, SOCK_STREAM, 0);
    if (ftpStruct->sockcmd < 0)
        ERROUT("socket failed\n");
    ftpStruct->sockdata = socket(AF_INET, SOCK_STREAM, 0);
    if (ftpStruct->sockdata < 0)
        ERROUT("socket failed\n");
    ret = tcpConnect(ftpStruct->sockcmd, ftpStruct->serverip, ftpStruct->serverCmdport);
    if (ret < 0)
        ERROUT("connect failed\n");
    recvFtpServerReply(ftpStruct);
    sendFtpCmd(ftpStruct, "USER", ftpStruct->user);
    recvFtpServerReply(ftpStruct);
    sendFtpCmd(ftpStruct, "PASS", ftpStruct->passwd);
    recvFtpServerReply(ftpStruct);
Err:
    return -1;
}

static int sendFtpCmd(
    IN FTPNODE * ftpStruct, 
    IN char * cmd, 
    IN char * arg)
{
    char tmpcmd[100] = {0};
    int ret = -1;
    
    sprintf(tmpcmd, "%s %s\n", cmd, arg);
    printf("[%s,%d] tmpcmd = %s\n", __FILE__, __LINE__, tmpcmd);    
    ret = send(ftpStruct->sockcmd, tmpcmd, sizeof(tmpcmd), 0);
    if (ret < 0)
        ERROUT("send err\n");
    return 0;
Err:
    return -1;
}

static int recvFtpServerReply(
    IN FTPNODE* ftpStruct)
{
    int ret = -1;
    int i = 0;
    char tmpBuf[1024] = {0};
    char t = 0;
    char* p = tmpBuf;
    do{
        ret = recv(ftpStruct->sockcmd, &t, 1, 0);
        p[i++] = t;
        if (t == '\n'){
            strcpy(ftpStruct->replyBuf, p);
            memset(p, 0, sizeof(tmpBuf));
            printf("[%s,%d] ret = %d, tmpBuf = %s", __FILE__, __LINE__, ret, ftpStruct->replyBuf);
            i = 0;
            if (ftpStruct->replyBuf[3] != '-')
                break;
        }
    }while (1);
    return 0;
}


static int tcpConnect(
    IN int sockfd, 
    IN char* ip, 
    IN int port)
{
    struct sockaddr_in sa;
    int ret = -1;
    
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(ip);
    printf("[%s,%d] sockfd = %d, ip = %s, port = %d\n", __FILE__, __LINE__, sockfd, ip, port);
    ret = connect(sockfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
    if (ret < 0)
        ERROUT("connect failed\n");
    else
        printf("connect success\n");
    return 0;
Err:
    return -1;
}

static int tcpDisconnect(
    IN int fd)
{
    close(fd);
}

static int parseReplyToCode(
    IN FTPNODE* ftpStruct)
{
    int code = -1;
    char codeBuf[10] = {0};
    memcpy(codeBuf, ftpStruct->replyBuf, 3);
    code = atoi(codeBuf);
    return code;
}

static int parseReplyToPort(
    IN FTPNODE* ftpStruct)
{
    int port = -1;
    char portBuf[10] = {0};
    char* p = NULL;
    int a1 = 0;
    int a2 = 0;
    int a3 = 0;
    int a4 = 0;
    int a5 = 0;
    int a6 = 0;
    p = strchr(ftpStruct->replyBuf, '(');
    p += 1;
    sscanf(p, "%d,%d,%d,%d,%d,%d", &a1, &a2, &a3, &a4, &a5, &a6);
    printf("[%s,%d] replyBuf = %s, %d,%d,%d,%d,%d,%d \n", __FILE__, __LINE__, ftpStruct->replyBuf, a1, a2, a3, a4, a5, a6);
    port = a5 * 256 + a6;
    return port;
}

static int ftpPutBegin(
    IN FTPNODE* ftpStruct)
{
    char tmpBuf[1024] = {0};
    int ret = -1;
    
    sendFtpCmd(ftpStruct, "TYPE", "I");
    recvFtpServerReply(ftpStruct);
    sendFtpCmd(ftpStruct, "PASV", "");
    recvFtpServerReply(ftpStruct);
    ret = parseReplyToPort(ftpStruct, &(ftpStruct->serverDataport));
Err:
    return -1;
}

static int ftpTransferData(
    IN FTPNODE* ftpStruct, 
    IN char* buf, 
    IN int len)
{
    printf("sockdata = %d, serverip = %s, serverDataport = %d\n", ftpStruct->sockdata, ftpStruct->serverip, ftpStruct->serverDataport);
    tcpConnect(ftpStruct->sockdata, ftpStruct->serverip, ftpStruct->serverDataport);
    sendFtpCmd(ftpStruct, "STOR", "wangjie.txt");
    recvFtpServerReply(ftpStruct);
    tcpSendData(ftpStruct->sockdata, buf, len);
    tcpDisconnect(ftpStruct->sockdata);
    tcpDisconnect(ftpStruct->sockcmd);
}

static int tcpSendData(
    IN int sockfd, 
    IN char* buf, 
    IN int len)
{
    send(sockfd, buf, len, 0);
    return 0;
}

/************api****************/
int ftpPost(
    IN FTPNODE* ftpStruct)
{
    ftpOpen(ftpStruct);
    ftpPutBegin(ftpStruct);
    ftpTransferData(ftpStruct, "wangjie nihao", strlen("wangjie nihao"));
    printf("ftpPost end\n");
    return 0;
}

int ftpStart(
    IN FTPNODE* ftpStruct)
{
    int gid = -1;
    pthread_create(&gid, NULL, ftpTask, ftpStruct);
    perror("pthread_create");
    return gid;
}

int ftpNodeCreate(
    OUT FTPNODE** p)
{
    p = (FTPNODE*)malloc(sizeof(FTPNODE));
    p->filepath1 = NULL;
    p->passwd = NULL;
    p->serverDataport = -1;
    p->serverCmdport = -1;
    p->serverip = NULL;
    p->replyBuf = NULL;
    p->user = NULL;
    p->sockcmd = -1;
    p->sockdata = -1;
        
    return 0;
}

