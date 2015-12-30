
#define IN
#define OUT

#define ERROUT(X)  \
do {\
    printf("[%s, %d]", __FUNCTION__, __LINE__);\
    printf(X);\
    goto Err;\
} while(0)\

int func(
    IN int a,
    IN int b,
    OUT int *c)
{
    if (NULL == c)
        ERROUT("");
    
    *c = a + b;//以后我一律都用指针当返回值
    return 0;//这个也是标识成功与否的
Err:
    return -1;//这个是用于标识函数的成功与否的
}
