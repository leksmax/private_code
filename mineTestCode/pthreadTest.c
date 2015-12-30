#include "stdio.h"
#include "pthread.h"

int g_tmp = 100;
pthread_mutex_t g_mutex;

void* pthreadEntry(void* arg)
{
    int i = -1;
    pthread_mutex_lock(&g_mutex);
    sleep(2);
    g_tmp = 2;
    printf("gid = %u, g_tmp = %d\n", pthread_self(), g_tmp);
    pthread_mutex_unlock(&g_mutex);
}

int main()
{
    pthread_t gid = -1;
    int arg = -1;
    pthread_mutex_init(&g_mutex);
    pthread_mutex_lock(&g_mutex);
    g_tmp = 1;
    printf("gid = %u, g_tmp = %d\n", pthread_self(), g_tmp);
    pthread_mutex_unlock(&g_mutex);
    gid = pthread_create(&gid, NULL, pthreadEntry, (void*)&arg);
    sleep(10);
    return 0;
}
