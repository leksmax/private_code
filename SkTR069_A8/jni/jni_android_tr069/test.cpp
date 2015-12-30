
#include <time.h>
#include <errno.h>

//#include "accessor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>


#ifdef TR069_ANDROID
#include <cutils/properties.h>
#include <sys/reboot.h>
#else
#include <sys/ioctl.h>
#endif

#include <android/log.h>

#define LOG_PARA_TAG    "JNILOG"
#define LOGD(...)  __android_log_print(ANDROID_LOG_INFO,LOG_PARA_TAG,__VA_ARGS__)

int test_socketinit()
{
		int fd  = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd == -1)
		{
			LOGD("[test_socketinit] socket creating err in udptalk\n");
			return -1;
		}
		return fd;
}

int test_bind(int s ,char* ipaddr,int port)
{
		LOGD("[test_bind] socket bind err in \n");
		struct sockaddr_in s_add,c_add;
		bzero(&s_add,sizeof(struct sockaddr_in));
		s_add.sin_family=AF_INET;
		s_add.sin_addr.s_addr=htonl(INADDR_ANY);
		s_add.sin_port=htons(port);

   //
   if(-1 == bind(s,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
   {
    LOGD("bind fail !\r\n");
    return -1;
   }
   LOGD("bind okokok sunjian sunjian !\r\n");
   return 0;
}

int test_entry()
{
		int sk=-1;
		LOGD("[main] begin  \n");
		sk=test_socketinit();
		if(sk>0)
		{
				if(test_bind(sk ,"172.28.17.143",8080)>=0)
				{
						 LOGD("test_bind okokok sunjian sunjian !\r\n");
						 return 0;
				}
				
		}
		LOGD("test_bind failure sunjian sunjian !\r\n");
		return 1;
}