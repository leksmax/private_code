#define IN
#define OUT

#define ERROUT(X)  \
do {\
    printf("[%s, %d]", __FUNCTION__, __LINE__);\
    printf(X);\
    goto Err;\
} while(0)\