
/* function parameter direction. */
#define IN
#define OUT

/*error deal*/
#define ERROUT(X, ...)  do {printf("[Err %s, %d]", __FUNCTION__, __LINE__);printf(X, ##__VA_ARGS__);goto Err;} while(0)
#define DBG(X, ...)  do {printf("[%s, %d]", __FUNCTION__, __LINE__);printf(X, ##__VA_ARGS__);} while(0)

/*flush: skip "enter"*/
#define FLUSH  do{int c;while( (c = getchar()) != '\n' && c != EOF);}while(0)