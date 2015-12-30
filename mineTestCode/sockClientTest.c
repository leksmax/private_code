#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "myHeadFile.h"


#define MAXDATASIZE 3000

int tcpConnect(
    IN char* ipBuf,
    IN int port,
    OUT int* connectFd)
{
    int sockfd = -1;
    struct sockaddr_in sa;
    
    if (NULL == ipBuf || port < 0|| NULL == connectFd)
        ERROUT("tcpConnect param err\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        ERROUT("socket error\n");

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, &sa.sin_addr, ipBuf);

    if (connect(sockfd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0)
        ERROUT("connect err\n");

    *connectFd = sockfd;
    return 0;
Err:
    return -1;
}

int main(int argc, char** argv)
{
    int sockfd = -1;
    char ipBuf[50] = {0};
    char transBuf[MAXDATASIZE] = {0};
    int dstPort = -1;
    int ret = -1;

    if (argc != 3)
        ERROUT("argc != 3\n");

    memcpy(ipBuf, argv[1], sizeof(ipBuf));
    dstPort = atoi(argv[2]);

    if (tcpConnect(ipBuf, dstPort, &sockfd) < 0)
        ERROUT("tcpConnect err\n");
    while(1){
        strcpy(transBuf, "haha\r\n");
        ret = send(sockfd, transBuf, MAXDATASIZE, 0);
        memset(transBuf, 0, MAXDATASIZE);
        ret = recv(sockfd, transBuf, MAXDATASIZE, 0);
        if (ret < 0)
            ERROUT("recv err\n");
        DBG("transBuf = %s\n", transBuf);
        sleep(1);
    }
      
    return 0;
Err:
    close(sockfd);
    return -1;
}
