#include <sys/socket.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>	
#include <arpa/nameser.h>
#include <resolv.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#ifdef TR069_ANDROID 
#include <linux/icmp.h>
#endif

#include "net_diagnosis.h"


#ifndef ERR_SUCCESS
#define ERR_SUCCESS		0 				//返回成功
#define ERR_FAILURE		-1 				//返回错误，普通错误



#endif

#define	MAX_DUP_CHK	(8 * 128)
#define MAX_DATA(a, b)   						( ((a) > (b)) ? (a) : (b) )


#define TRACERT_PING_ONCE_COUNT			1
#define TRACERT_DATABLOCK_SIZE_DEFAULT	32


static const int MAXIPLEN = 60;
static const int DEFDATALEN = 32;
static const int MAXICMPLEN = 76;
static const int MAXPACKET = 65468;


#define	A(bit)		rcvd_tbl[(bit)>>3]			//identify byte in array
#define	B(bit)		(1 << ((bit) & 0x07))		//identify bit in byte
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))


static struct sockaddr_in 	pingaddr;
static int 					pingsock = -1;
static int 					datalen; 			// intentionally uninitialized to work around gcc bug 
static int 					nTimeout;
static int 					nDscp;
static int 					nTTL = 0;
static long int 					loss_rate;
static long int 			thread_flag = 0;
static char 				szResponseHost[16];
static char 				host_st[32];

static long 				ntransmitted, nreceived, nrepeats, pingcount;
static int 					myid, options;
static unsigned long 		tmin = ULONG_MAX, tmax, tsum;
static char 				rcvd_tbl[MAX_DUP_CHK / 8];

struct hostent 				*hostent;

static void 	sendping(int);
static void 	pingstats(int);
static int 		unpack(char *buf, int sz, int nSendTime, int nRecvTime);

extern int 		create_icmp_socket(void);

int create_icmp_socket(void)
{
	struct protoent *proto;
	int 			sock;

	proto = getprotobyname("icmp");
	
	if ((sock = socket(AF_INET,SOCK_RAW ,
			(proto ? proto->p_proto : 1))) < 0) 
	{        
		printf("err = %d\n",errno);
		return -1;
	}

	// drop root privs if running setuid 
	setuid(getuid());

	return sock;
}
void GetCurTime(char *time1)
{	     
	char *curtime = NULL;    
	time_t nowtime;     
	struct tm *timeinfo;    

	curtime = time1;
	time( &nowtime );    
	timeinfo = localtime( &nowtime );    
	int year, month, day,hour,minute;    
	year = timeinfo->tm_year + 1900;    
	month = timeinfo->tm_mon + 1;     
	day = timeinfo->tm_mday;     
	hour = timeinfo->tm_hour;    
	minute = timeinfo->tm_min;     
	sprintf(curtime,"%d%d%d%d%d",year,month,day,hour,minute);    
	printf("curtime = %s\n",curtime);	
}
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}



static void sendping(int junk)
{
	struct icmp *pkt;
	int i;
	char packet[datalen + 8];

	pkt = (struct icmp *) packet;

	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	CLR(pkt->icmp_seq % MAX_DUP_CHK);

	gettimeofday((struct timeval *) &packet[8], NULL);
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	i = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
//	printf("sendping-i=%d\n",i);


}

static int unpack(char *buf, int sz, int nSendTime, int nRecvTime)
{
	struct icmp *icmppkt;
	struct iphdr *iphdr;
	struct in_addr sInAddr;
	struct timeval tv, *tp;
	int hlen, dupflag;
	unsigned long triptime;

	gettimeofday(&tv, NULL);

	// check IP header
	iphdr = (struct iphdr *) buf;
	
	hlen = iphdr->ihl << 2;
	
	// discard if too short
	if (sz < (datalen + ICMP_MINLEN))
	{
		return ERR_FAILURE;
	}
	
	sz -= hlen;
	icmppkt = (struct icmp *) (buf + hlen);
	
	if ((icmppkt->icmp_type == ICMP_ECHOREPLY
		&& icmppkt->icmp_id == myid))
	{
	    	++nreceived;

		if (TST(icmppkt->icmp_seq % MAX_DUP_CHK) 
			&& icmppkt->icmp_type == ICMP_ECHOREPLY) 
		{
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} 
		else 
		{
			SET(icmppkt->icmp_seq % MAX_DUP_CHK);
			dupflag = 0;
		}

		sInAddr.s_addr = (uint32_t)(iphdr->saddr);

		memset(szResponseHost, 0, 16);
		strcpy(szResponseHost, inet_ntoa(sInAddr));


		return ERR_SUCCESS;
	} 

	return ERR_FAILURE;
}


static int gettime_ms (void)
{					 
	struct timeval tv;
	struct timezone tz;
	gettimeofday (&tv, &tz);
	return (tv.tv_sec * 1000 + tv.tv_usec/1000);
}

void* ping_thrd(void * argu)
{
	char packet[datalen + MAXIPLEN + MAXICMPLEN];
	int t0, t1, i, nSendTime, nRecvTime;
	int nErr=ERR_FAILURE;
	
	pingsock = create_icmp_socket();
	printf("pingsock = %d\n",pingsock);
	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin_family = AF_INET;
	hostent = gethostbyname(host_st);
	if (hostent == NULL)
	{
		printf("Ping request could not find host '%s'. Please check the name and try again.\n", host_st);
		return NULL;
	}
	char   str[32];

	memcpy(&pingaddr.sin_addr, hostent->h_addr, sizeof(pingaddr.sin_addr));

	//设置DSCP
//	setsockopt(pingsock, IPPROTO_IP, IP_TOS, (char *)&nDscp, sizeof(int));

	//设置TTL
	setsockopt(pingsock, IPPROTO_IP, IP_TTL, (char *)&nTTL, sizeof(int));


while(1)
{
for (i=0; i<pingcount; i++)
	{
		// start the ping's going ... 
		sendping(0);
		nSendTime = gettime_ms ();
	
		t0 = gettime_ms ();
		
		//listen for replies 
		while (1) 
		{
			struct sockaddr_in from;
			socklen_t fromlen = (socklen_t) sizeof(from);
			int c;

			if ((c = recvfrom(pingsock, packet, sizeof(packet), MSG_DONTWAIT,
							  (struct sockaddr *) &from, &fromlen)) < 0) 
			{
				t1 = gettime_ms ();
				if (t1 - t0 < nTimeout)
				{
					continue ;
				}
				else
				{	
					printf("ping timed out...........nTimeout=%d\n",nTimeout);
					break;
				}
			}

			nRecvTime = gettime_ms ();
			//printf("unpack-C = %d\n",c);
			nErr = unpack(packet, c, nSendTime, nRecvTime);
			if (nErr == ERR_SUCCESS)
			{
				break;
			}
		}
	}
		if (ntransmitted)
		{
			loss_rate = (ntransmitted - nreceived) * 100 / ntransmitted;
			printf("%ld%% packet loss\n",loss_rate);
		}
}	
	
		
			   
	
	close(pingsock);
	return NULL;
}


static int ping(const char *host)
{

	int ret = 0;
	
	strcpy(host_st,host);
	memset(szResponseHost, 0, 16);
	
	myid 			= getpid() & 0xFFFF;
	ntransmitted 	= 0;
	nreceived 		= 0;
	nrepeats 		= 0;

	tmin 			= ULONG_MAX;
	tmax 			= 0; 
	tsum 			= 0;

	nDscp 			= 0;
	nTTL 			= IPDEFTTL;
	datalen 		= DEFDATALEN; // initialized here rather than in global scope to work around gcc bug 
	options 		= 0;
	
	
	pingcount=100;
	

	//Timeout
	nTimeout=200;
	
	//DataBlockSize

		datalen=32;
	
		nTTL=64;
		
	pthread_t ping_thread;
	ret = pthread_create(&ping_thread,NULL,ping_thrd,NULL);
	return ret;
}
#if 0
void main(void)
{
	int ret;
	ret = ping("172.28.17.197");
	printf("main_ret = %d\n",ret);
}
#endif