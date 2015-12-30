#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "myHeadFile.h"

#define MAXDATASIZE 3000

int readFile(
    IN char* filename,
    OUT char* fileTotalContent)
{
    int ret = -1;
    FILE* fileSrc = NULL;
    int n = -1;
    char readBuf[MAXDATASIZE] = {0};
    
    if ((NULL == filename) || (NULL == fileTotalContent))
        ERROUT("filename fileTotalContent is NULL\n");

    ret = access(filename, 0);
    if (ret < 0)
        ERROUT("%s is not exist\n", filename);

    fileSrc = fopen(filename, "r");
    if (fileSrc == NULL)
        ERROUT("fopen err\n");

    n = fread(readBuf, 1, sizeof(readBuf), fileSrc);
    if (n < 0)
        ERROUT("fread err\n");
    else if (n == MAXDATASIZE)
        ERROUT("n == 3000\n");
    strcpy(fileTotalContent, readBuf);

    fclose(fileSrc);
    
    return 0;
Err:
    return -1;
}

int main(int argc, char** argv)
{
    int sockfd = -1;
    struct sockaddr_in sa;
    char ipBuf[50] = {0};
    char transBuf[MAXDATASIZE] = {0};
    int dstPort = -1;
    int ret = -1;
    int clientSockfd = -1;
    int inSize = MAXDATASIZE;

    if (argc != 2)
        ERROUT("argc !=2\n");

    dstPort = atoi(argv[1]);
    strcpy(ipBuf, "127.0.0.1");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        ERROUT("socket error\n");

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(dstPort);
    inet_pton(AF_INET, &sa.sin_addr, ipBuf);

    if (bind(sockfd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0)
        perror("bind");

    if (listen(sockfd, 3))
        perror("listen");

    if ((clientSockfd = accept(sockfd, (struct sockaddr*)&sa, &inSize)) < 0)
        perror("accept");
    else{
        while(1){
            ret = recv(clientSockfd, transBuf, MAXDATASIZE, 0);
            if (ret < 0)
                perror("recv");
            memset(transBuf, 0, MAXDATASIZE);
            ret = readFile("ping", transBuf);
            if (ret < 0)
                ERROUT("readFlie err\n");
            ret = send(clientSockfd, transBuf, MAXDATASIZE, 0);
        }
    }
    close(sockfd);
      
    return 0;
Err:
    return -1;
}