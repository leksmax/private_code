
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
    
    *c = a + b;//�Ժ���һ�ɶ���ָ�뵱����ֵ
    return 0;//���Ҳ�Ǳ�ʶ�ɹ�����
Err:
    return -1;//��������ڱ�ʶ�����ĳɹ�����
}
