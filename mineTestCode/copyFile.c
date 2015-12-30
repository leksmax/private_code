#include "stdio.h"
#include "myHeadFile.h"

int fileCopy(
    IN char* srcPath,
    IN char* dstPath)
{
    FILE* pFileSrc = NULL;
    FILE* pFileDst = NULL;
    char buf[2048] = {0};
    int ret = 0;
    int n = 0;


    if (0 != access(srcPath, 0))
        ERROUT("src does not exits\n");

    pFileSrc = fopen(srcPath, "rb");
    pFileDst = fopen(dstPath, "wb");

    while(1){
        n= fread(buf, 1, sizeof(buf), pFileSrc);
        if (n > 0)
            fwrite(buf, 1, n, pFileDst);
        else
            break;
    }

    fclose(pFileSrc);
    fclose(pFileDst);
    return 0;
Err:
    return -1;
}

int main(int argc, char *argv[])
{
    if (3 != argc)
        ERROUT("Please check input argcs.\nInput as ./a.out srcFile dstFile\n");
    fileCopy(argv[1], argv[2]);
    return 0;
Err:
    return -1;
}
