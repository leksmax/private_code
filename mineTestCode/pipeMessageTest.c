#include "stdio.h"
#include "myHeadFile.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"

#define IN
#define OUT

typedef struct msgqueue {
    int size;
    int fds[2];
}MSGQUEUE;

int msgqCcreate(
    IN int msg_size, 
    OUT MSGQUEUE **msgq)
{
    MSGQUEUE *p = NULL;
    if (NULL == msgq){
        printf("msgq is NULL\n");
        goto Err;
    }
    
    p  = (MSGQUEUE*)malloc(sizeof(MSGQUEUE));
    if (p == NULL)
        ERROUT("malloc\n");

    if (pipe(p->fds)) {
        free(p);
        printf("pipe\n");
        goto Err;
    }
    
    fcntl(p->fds[0], F_SETFL, fcntl(p->fds[0], F_GETFL) | O_NONBLOCK);
    fcntl(p->fds[1], F_SETFL, fcntl(p->fds[1], F_GETFL) | O_NONBLOCK);
    p->size = msg_size;
    *msgq = p;

    return 0;
Err:
    return -1;
}

int msgqDelete(
    IN MSGQUEUE * msgq)
{
    if (msgq == NULL){
        printf("msgq = %p\n", msgq);
        goto Err;
    }
    close(msgq->fds[0]);
    close(msgq->fds[1]);
    free(msgq);
    return 0;
Err:
    return -1;
}

int msgqReadFdGet(
    IN MSGQUEUE* msgq,
    OUT int* fd)
{
    if (msgq == NULL){
        printf("msgq = %p\n", msgq); 
        goto Err;
    }
    
    *fd = msgq->fds[0];
    return 0;
Err:
    return -1;
}

int msgqWriteFdGet(
    IN MSGQUEUE* msgq,
    OUT int* fd)
{
    if (msgq == NULL){
        printf("msgq = %p\n", msgq); 
        goto Err;
    }
    *fd = msgq->fds[1];
    return 0;
Err:
    return -1;
}

int msgqPutMsg(
    IN MSGQUEUE* msgq, 
    IN char *msg)
{
    int ret;

    if (NULL == msgq || NULL == msg){
        printf("msgq ||msg = %p\n", msgq); 
        goto Err;
    }

    ret = write(msgq->fds[1], msg, msgq->size);
    if (ret != msgq->size){
        printf("ret = %d, msgq->size = %d\n", ret, msgq->size);
        goto Err;
    }

    return 0;
Err:
    return -1;
}

int msgqGetMsg(
    IN MSGQUEUE* msgq, 
    OUT char *msg)
{
    int ret;

    if (NULL == msg || NULL == msgq) {
        printf("msg || msgq is NULL\n");
        goto Err;
    }

    ret = read(msgq->fds[0], msg, msgq->size);
    if (ret != msgq->size){
        printf("ret = %d, msgq->size = %d\n", ret, msgq->size);
        goto Err;
    }

    return 0;
Err:
    return -1;
}

int main()
{
    MSGQUEUE* testQueue = NULL;
    int fd = -1;
    char msgPutBuf[90] = "testetstsets";
    char msgGetBuf[100] = {0};
    
    msgqCcreate(100, &testQueue);
    msgqReadFdGet(testQueue, &fd);
    printf("fd = %d\n", fd);
    msgqWriteFdGet(testQueue, &fd);
    printf("fd = %d\n", fd);

    
    msgqPutMsg(testQueue, msgPutBuf);
    msgqGetMsg(testQueue, msgGetBuf);
    printf("msgGetBuf = %s\n", msgGetBuf);

    msgqDelete(testQueue);
    
    return 0;
}