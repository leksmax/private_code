#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/times.h>


#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
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
#include <sys/time.h>

#include "header.h"
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
#define TRACERT_DATABLOCK_SIZE_DEFAULT	64


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
static int 					nTTL;
static char 				szResponseHost[16];

static long 				ntransmitted, nreceived, nrepeats, pingcount;
static int 					myid, options;
static unsigned long 		tmin = ULONG_MAX, tmax, tsum;
static char 				rcvd_tbl[MAX_DUP_CHK / 8];

struct hostent 				*hostent;

static void 	sendping(int);
static void 	pingstats(int);
static int 		unpack(char *buf, int sz, int nSendTime, int nRecvTime);

extern int 		create_icmp_socket(void);


/**************************************************************************/

static void pingstats(int junk)
{
	LOGI("\n--- %s ping statistics ---\n", hostent->h_name);
	LOGI("%ld packets transmitted, ", ntransmitted);
	LOGI("%ld packets received, ", nreceived);
	if (nrepeats)
		LOGI("%ld duplicates, ", nrepeats);
	if (ntransmitted)
		LOGI("%ld%% packet loss\n",
			   (ntransmitted - nreceived) * 100 / ntransmitted);
	if (nreceived)
		LOGI("round-trip min/avg/max = %lu.%lu/%lu.%lu/%lu.%lu ms\n",
			   tmin / 10, tmin % 10,
			   (tsum / (nreceived + nrepeats)) / 10,
			   (tsum / (nreceived + nrepeats)) % 10, 
			   tmax / 10, tmax % 10);

	return ;
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
	LOGI("sendping send to i=:%d,errno:%d,pingsock:%d\n", i,errno,pingsock);


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
		&& icmppkt->icmp_id == myid)
		|| icmppkt->icmp_type == ICMP_TIME_EXCEEDED)
	{
	    	++nreceived;

		if (icmppkt->icmp_type == ICMP_ECHOREPLY)
		{
			tp = (struct timeval *) icmppkt->icmp_data;

			if ((tv.tv_usec -= tp->tv_usec) < 0) 
			{
				--tv.tv_sec;
				tv.tv_usec += 1000000;
			}
			tv.tv_sec -= tp->tv_sec;

			triptime = tv.tv_sec * 10000 + (tv.tv_usec / 100);
		}
		else
		{
			triptime = (nRecvTime - nSendTime) * 10;
		}
		
		tsum += triptime;
		if (triptime < tmin)
			tmin = triptime;
		if (triptime > tmax)
			tmax = triptime;

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


		//printf("%d bytes from %s: icmp_seq=%u", sz,	szResponseHost,	icmppkt->icmp_seq);
		//printf(" ttl=%d", iphdr->ttl);
		//printf(" time=%lu.%lu ms", triptime / 10, triptime % 10);
		//if (dupflag)
		//	printf(" (DUP!)");
		//printf("\n");

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

int create_icmp_socket(void)
{
	struct protoent *proto;
	int 			sock;

	proto = getprotobyname("icmp");
    LOGI("create_icmp_socket getprotobyname proto = %d\n",proto);
	if ((sock = socket(AF_INET, SOCK_RAW,
					1)) < 0)	
			//(proto ? proto->p_proto : 1))) < 0) 
	{        
		LOGI("create_icmp_socket begin==pingsock:%d,errno:%d , strerror:%s\n",sock,errno,strerror(errno));
		return -1;
	}

	// drop root privs if running setuid 
	setuid(getuid());
	return sock;
}



 void sk_ping( char *host)
{
	char packet[datalen + MAXIPLEN + MAXICMPLEN];
	int t0, t1, i, nSendTime, nRecvTime;
	int nErr;

	LOGD("sk_ping begin====...\n");
	
	pingsock = create_icmp_socket();
    LOGI("sk_ping begin====== pingsock:%d,errno:%d\n",pingsock,errno);
	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	pingaddr.sin_family = AF_INET;
	hostent = gethostbyname(host);
//	PING:The parameter is not legal
   
	if (hostent == NULL)
	{
		printf("Ping request could not find host '%s'. Please check the name and try again.\n", host);
		LOGD("Ping request could not find host '%s'. Please check the name and try again.\n", host);
		return ;
	}
	
	memcpy(&pingaddr.sin_addr, hostent->h_addr, sizeof(pingaddr.sin_addr));

	//设置DSCP
	setsockopt(pingsock, IPPROTO_IP, IP_TOS, (char *)&nDscp, sizeof(int));

	//设置TTL
	setsockopt(pingsock, IPPROTO_IP, IP_TTL, (char *)&nTTL, sizeof(int));
/*
	printf("PING %s (%s): %d data bytes\n",
	           hostent->h_name,
		   inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
		   datalen);
*/
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
					//printf("Request timed out.\n");
					break;
				}
			}

			nRecvTime = gettime_ms ();
			
			nErr = unpack(packet, c, nSendTime, nRecvTime);
			if (nErr == ERR_SUCCESS)
			{
				break;
			}
		}
	}
	
	LOGI("ping   pingstats begin======1112.\n");
	pingstats(0);
	if(pingsock>0)
	{
	 close(pingsock);
	}

   	LOGI("ping   pingstats end======1112.\n");
	return ;
}


void sk_pingbyjava( char *host)
{
	bindermsg msg = {0};
	char cmdkey[1024] = {0};
	sprintf(cmdkey , "ping -c %d -s %d -t %d %s" , pingcount , datalen , nTTL , host);
	LOGI("\sk_pingbyjava...destip:%s \n" , cmdkey);
	//sk_func_porting_params_set("tr069_ping", cmdkey , sizeof(cmdkey));

	strcpy(msg.user,"TR069");
	strcpy(msg.msg, cmdkey);
	msg.cmd = 0;
	msg.type = START_PING;
	msg.len = strlen(msg.msg);

    LOGD("[sk_pingbyjava] enter\n");
    sk_binder_send(&msg,sizeof(msg));	
    LOGD("[sk_pingbyjava] exit\n");
}


void sk_traceroute_by_java(char *host , int max_ttl , int timeout , int data_size)
{
    bindermsg msg = {0};
    char cmdkey[1024] = {0};
    int ts = timeout / 1000 ;
    if(ts <= 0)
    {
        ts = 1;
    }
    
	sprintf(cmdkey , "su -c /system/xbin/traceroute -m %d -w %d %s %d" , max_ttl ,ts , host , data_size);
	LOGI("\sk_traceroute_by_java...destip:%s \n" , cmdkey);
	//sk_func_porting_params_set("tr069_route", cmdkey , sizeof(cmdkey));

	strcpy(msg.user,"TR069");
	strcpy(msg.msg, cmdkey);
	msg.cmd = 0;
	msg.type = START_TRACEROUTE;
	msg.len = strlen(msg.msg);

    LOGD("[sk_traceroute_by_java] enter\n");
    sk_binder_send(&msg,sizeof(msg));	
    LOGD("[sk_traceroute_by_java] exit\n");

}


int sk_parse_ping_file(char *file_name , sk_ping_results_t* pPingResults)
{
    FILE *fp = NULL;
    int file_len = 0;
	int fpno = 0;
    unsigned char *buf = NULL;
    unsigned int read_size = 0;
	struct stat  stat0[1] = {0};
   
    if(NULL == file_name || NULL == pPingResults)
    {
        goto LAB_ERR;
    }
    
    fp = fopen(file_name , "rb");
    
    if(NULL == fp )
    {
        LOGI("fopen file failed , filename = %s\n",file_name);
        goto LAB_ERR;
    }

    fpno = fileno(fp);
	fstat(fpno , stat0);
	file_len = stat0->st_size;

    if( file_len <= 0)
    {
        LOGI("file is not right , file_len = %d\n",file_len);
        goto LAB_ERR;
    }
    
    LOGI("[sk_parse_ping_file]file_len = %d\n",file_len);
    
    buf = calloc(1, file_len+1);
    if(NULL == buf)
        goto LAB_ERR;


    fseek(fp, 0, SEEK_SET); 

    read_size = fread(buf , 1 , file_len , fp);
    if(read_size != file_len)
        goto LAB_ERR;
    
    sk_tr069_get_pingresult( pPingResults , buf);

    if(NULL != buf)
    {
        free(buf);
        buf = NULL;
    }
    if(NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return 0;
    
LAB_ERR:
    if(NULL != buf)
    {
        free(buf);
        buf = NULL;
    }
    if(NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }
    LOGI("Error end\n" );
    return -1;
}


int sk_start_ping(const sk_ping_attribute_t* pPingAttribute, sk_ping_results_t* pPingResults)
{

	if (pPingAttribute == NULL || pPingResults == NULL)
	{
		goto LAB_ERR;
	}
    
    LOGI("\sk_start_ping...destip:%s \n",pPingAttribute->szHost);
    
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


	if (pPingAttribute->szHost[0] == '\0'
		|| pPingAttribute->nNumberOfRepetitions <= 0
		|| pPingAttribute->nTimeout <= 0)
	{
		LOGD("PING:The parameter is not legal ! szHost = %s , nNumberOfRepetitions = %d , nTimeout = %d \n",
                pPingAttribute->szHost , pPingAttribute->nNumberOfRepetitions , pPingAttribute->nTimeout );
	    goto LAB_ERR;
	}


	//NumberOfRepetitions；
	
	pingcount=pPingAttribute->nNumberOfRepetitions;
	

	//Timeout
	nTimeout=pPingAttribute->nTimeout;
	
	//DataBlockSize
	if(pPingAttribute->nDataBlockSize <= 32)
	{
		datalen = 32;
	}
    else
    {
		datalen=pPingAttribute->nDataBlockSize;
    }
    
	nDscp=pPingAttribute->nDSCP;

	if(pPingAttribute->nTTL<=0)
	{
		nTTL=64;
	}
    else
    {
		nTTL=pPingAttribute->nTTL;
    }
	
   LOGI("\sk_start_ping...destip11110:%s ,begin to sk_ping \n",pPingAttribute->szHost);
    #if 0
	sk_ping(pPingAttribute->szHost);
	
	 LOGI("\sk_start_ping...destip222 sunjian:%s \n",pPingAttribute->szHost);
	
	strcpy(pPingResults->szHost, szResponseHost);
	pPingResults->nTotalCount 			= ntransmitted;
	pPingResults->nSuccessCount 			= nreceived;
	pPingResults->nFailureCount			= ntransmitted - nreceived;

	if (nreceived)
	{
		pPingResults->nAverageResponseTime	= MAX_DATA((tsum / (nreceived + nrepeats)) / 10, 1);
		pPingResults->nMinimumResponseTime	= MAX_DATA(tmin / 10, 1);
		pPingResults->nMaximumResponseTime	= MAX_DATA(tmax / 10, 1);
	}
	else
	{
		pPingResults->nAverageResponseTime	= 0;
		pPingResults->nMinimumResponseTime	= 0;
		pPingResults->nMaximumResponseTime	= 0;	
	}
    #else
    
	 sk_pingbyjava(pPingAttribute->szHost);

     sk_parse_ping_file("/ping.txt" , pPingResults);

    #endif
	
	//printf("PING:Start PING Successful!\n");
	return ERR_SUCCESS;

LAB_ERR:
    return ERR_FAILURE;
}



int sk_parse_tracert_file(char *file_name , sk_traceroute_results_t* pTracerouteResults)
{
    FILE *fp = NULL;
    int file_len = 0;
	int fpno = 0;
    unsigned char *buf = NULL;
    unsigned int read_size = 0;
	struct stat  stat0[1] = {0};
    
   
    if(NULL == file_name || NULL == pTracerouteResults)
        goto LAB_ERR;
    fp = fopen(file_name , "rb");
     
    if(NULL == fp)
    {
        LOGI("fopen file failed , filename = %s\n",file_name);
        goto LAB_ERR;
    }

    fpno = fileno(fp);
	fstat(fpno , stat0);
	file_len = stat0->st_size;

    if(file_len <= 0)
    {
        LOGI("file is not right , file_len = %d\n",file_len);
        goto LAB_ERR;
    }
    
    buf = calloc(1, file_len+1);
    if(NULL == buf)
        goto LAB_ERR;


    fseek(fp, 0, SEEK_SET); 

    read_size = fread(buf , 1 , file_len , fp);
    if(read_size != file_len)
        goto LAB_ERR;
    

    sk_tr069_get_tracerouteresult(pTracerouteResults , buf);

    if(NULL != buf)
    {
        free(buf);
        buf = NULL;
    }
    if(NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }

    return 0;
    
LAB_ERR:
    if(NULL != buf)
    {
        free(buf);
        buf = NULL;
    }
    if(NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }
    LOGI("Error end\n" );
    return -1;
}


int sk_start_traceroute(const sk_traceroute_attribute_t* pTracertAttribute, sk_traceroute_results_t* pTracertResults)
{
	sk_ping_attribute_t 	sPingAttribute;
	sk_ping_results_t 		sPingResults;
	struct hostent 		*pHostEnt;
	struct in_addr 		sInAddr;
	char 				szTraceRouteHost[64];
	int 				nTempMaxHopCount;
    int                 nDataBlockSize;
    int                 nTimeout;
	int 				i,j;
	int 				nErr;
    int 				start_time,end_time;
	int					start_index;
	int					flag=0;
	int					cmp_retult=0;
	
	LOGI("sk_start_traceroute... host =:%s \n",pTracertAttribute->szHost);
	if (pTracertAttribute == NULL 
        || pTracertResults == NULL 
        || pTracertAttribute->szHost[0] == '\0'
		|| pTracertAttribute->nTimeout <= 0)
	{
		LOGI("TRACERT:The parameter is not legal!\n");	
		goto LAB_ERR;
	}

	memset(pTracertResults, 0, sizeof(sk_traceroute_results_t));

	//获取路由诊断主机的IP地址
	memset(szTraceRouteHost, 0, sizeof(szTraceRouteHost));
	
	if(inet_pton(AF_INET, pTracertAttribute->szHost, &sInAddr) == 0)
	{
		pHostEnt = gethostbyname(pTracertAttribute->szHost);
		if(pHostEnt == NULL)
		{
			LOGI("Unable to resolve target system name '%s'.", pTracertAttribute->szHost);
			goto LAB_ERR;
		}

		memcpy(&sInAddr, pHostEnt->h_addr, sizeof(struct in_addr));
		strcpy(szTraceRouteHost, inet_ntoa(sInAddr));
	}
	else
	{
		strcpy(szTraceRouteHost, pTracertAttribute->szHost);
	}

	//填充PING参数
	memset(&sPingAttribute, 0, sizeof(sk_ping_attribute_t));
	memset(&sPingResults, 0, sizeof(sk_ping_results_t));

	strcpy(sPingAttribute.szHost, szTraceRouteHost);
	sPingAttribute.nNumberOfRepetitions	= TRACERT_PING_ONCE_COUNT;


    if(pTracertAttribute->nTimeout <= 0)
    {
       	sPingAttribute.nTimeout	 = 1000;
        nTimeout = 1000;
    }
    else
    {
        sPingAttribute.nTimeout	= pTracertAttribute->nTimeout;
        nTimeout = pTracertAttribute->nTimeout;
    }   
    
	if (pTracertAttribute->nDataBlockSize <= 64)
	{
		sPingAttribute.nDataBlockSize	= TRACERT_DATABLOCK_SIZE_DEFAULT;
        nDataBlockSize = TRACERT_DATABLOCK_SIZE_DEFAULT;
	}
	else
	{
		sPingAttribute.nDataBlockSize	= pTracertAttribute->nDataBlockSize;
        nDataBlockSize	= pTracertAttribute->nDataBlockSize;
	}

	if (pTracertAttribute->nMaxHopCount <= 0)
	{
		nTempMaxHopCount			= TRACERT_MAX_HOP_COUNT_DEFAULT;
	}
	else
	{
		nTempMaxHopCount			= pTracertAttribute->nMaxHopCount;
	}
	
	sPingAttribute.nDSCP				= pTracertAttribute->nDSCP;
    
	

	for (i=0; i< nTempMaxHopCount; i++)
	{
		pTracertResults->gHopHost[i].flag=0;
		//设置TTL
		sPingAttribute.nTTL	= (i + 1);
		
		nErr = sk_start_ping(&sPingAttribute, &sPingResults);
		if (nErr != ERR_SUCCESS)
		{
			printf("TRACERT:Start PING Error!\n");
			
			return ERR_FAILURE;
		}

		pTracertResults->gHopHost[i].nTTL	=  sPingAttribute.nTTL;
		printf("szTraceRouteHost=%s,sPingResults.szHost=%s\n",szTraceRouteHost,sPingResults.szHost);
		strcpy(pTracertResults->gHopHost[i].szHost, sPingResults.szHost);
		pTracertResults->nNumberOfRouteHops = sPingAttribute.nTTL;
		if (strcmp(sPingResults.szHost, pTracertAttribute->szHost) == 0)
			break;
	}	
    
	start_time = gettime_ms ();
    
	//sk_traceroute_by_java(szTraceRouteHost , nTempMaxHopCount , nTimeout , nDataBlockSize);
    
	end_time = gettime_ms ();

    pTracertResults->nResponseTime = end_time - start_time;
    LOGI("[tr069]pTracertResults->nNumberOfRouteHops=%d\n",pTracertResults->nNumberOfRouteHops);

    //sk_parse_tracert_file("/traceroute.txt" , pTracertResults);
	return ERR_SUCCESS;

LAB_ERR:
    return ERR_FAILURE;
}


