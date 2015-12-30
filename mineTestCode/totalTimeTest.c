#include "stdio.h"
#include "sys/time.h"

int totalTimeTest()
{
    struct timeval  start,end;
    float  total;
    long long int i = 0;
    gettimeofday(&start,NULL);
    
    do {
        i++;
    }while (i != 10000000);
    
    gettimeofday(&end, NULL);
    total = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    printf("use time is %f usec\n",total);
    return 0;
}

int main()
{
    totalTimeTest();
    return 0;
}
