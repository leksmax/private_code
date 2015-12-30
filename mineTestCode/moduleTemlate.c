#define param  username //this is for outside param

typedef struct myStruct{
    int ..;
    char* a;//this struct used for this module, just as in c++ ,there is some private attribute
};

static int funca()
{
    
}//如果不带参数，说明用不到外部参数

static int funcb(int a)
{
    //a是外部参数，所以用到参数传递
}

int externFunc()
{
    //这个是外部接口
}

int main()
{
    //这个是用于模块内部测试，记住，模块的各个功能都是独立的
}
