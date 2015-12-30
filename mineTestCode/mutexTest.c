#include "stdio.h"
#include "myHeadFile.h"
#include "stdlib.h"

struct mutexType {
    pthread_mutex_t id;
};

typedef struct mutexType* mutex_t;

int mutexCreate(
    OUT mutex_t* outMutex)
{
    mutex_t mutex;

    mutex = (mutex_t)malloc(sizeof(struct mutexType));
    if(mutex == NULL)
        ERROUT("\n"); 
    if (pthread_mutex_init(&mutex->id, NULL)) {
        free(mutex);
        ERROUT("\n"); 
    }
    printf("mutex = %x\n", mutex);
    *outMutex = mutex;
    
    return 0;
Err:
    return -1;
}

int mutexLock(
    IN mutex_t mutex)
{
    int ret = -1;
    if(mutex == NULL)
        ERROUT("\n"); 
    ret = pthread_mutex_lock(&mutex->id);
    if (0 != ret)
        ERROUT("lock err\n");
    return 0;
Err:
    return -1;
}

int mutexUnlock(
    IN mutex_t mutex)
{
    int ret = -1;
    if(mutex == NULL)
        ERROUT("\n"); 
    ret = pthread_mutex_unlock(&mutex->id);
    if (0 != ret)
        ERROUT("unlock err\n");
    return 0;
Err:
    return -1;
}

void task(
    IN void* arg)
{
    sleep(1);
    mutexUnlock((mutex_t)arg);
    printf("task start\n");
    return;
}

int main()
{
    int ret = -1;
    int tid = -1;
    mutex_t testMutex = NULL;
    ret = mutexCreate(&testMutex);
    pthread_create(&tid, 0, task, (void*)testMutex);
    ret = mutexLock(testMutex);
    printf("ret = %d\n", ret);
    sleep(2);
    return 0;
}
