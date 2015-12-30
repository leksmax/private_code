#include "ftp.h"
#include "main.h"

int main()
{
    int tid = -1;
    void* status;
    FTPNODE* ftpNode = NULL;
    
    tid = ftpStart(ftpNodeCreate(&ftpNode));
    printf("[%s,%d]tid = %d\n", __FILE__, __LINE__, tid);
    pthread_join(tid, &status);
    return 0;
}
