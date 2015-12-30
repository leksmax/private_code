#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>

#include <stdio.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include "winsock2.h"
#else
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if.h>
#ifdef TR069_ANDROID
#include <cutils/properties.h>
#include <sys/reboot.h>
#else
#include <sys/ioctl.h>
#endif
#endif


#include "header.h"
#include "log.h"
#include "stun_bind.h"




static char szUserName[USER_NAME_LEN + 1];
const char szDslForum[] = "dslforum.org/TR-111";

static char *encode16(char* buf, UInt16 data)
{
	UInt16 ndata = htons(data);
	memcpy(buf, &ndata, sizeof(UInt16));
	return buf + sizeof(UInt16);
}

static char *encode32(char* buf, UInt32 data)
{
	UInt32 ndata = htonl(data);
	memcpy(buf, &ndata, sizeof(UInt32));
	return buf + sizeof(UInt32);
}

UInt128 createTransctionID (void)
{
	UInt128 id;
	UInt32 i,r;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	srand((int)time(0)+(int)tv.tv_usec);
	for(i=0; i<16; i+=4)
	{
		assert(i+3 < 16);
		r = rand();
		id.octet[i+0] = r>>0;
		id.octet[i+1] = r>>8;
		id.octet[i+2] = r>>16;
		id.octet[i+3] = r>>24;
	}
	return id;
}

void initIdArray(UInt128 *ID, int length)
{
	int i,j;
	
	for(i=0; i<length; i++)
	{
		for(j=0; j<16; j++)
		{
			ID->octet[j] = 0;
		}
		ID++;
	}
}

bool checkTransctionID(UInt128 id, UInt128 *ID, int count)
{

	int i;
	assert(count > 0);
//	printf("count is %d\n",count);
		
	for(i=0; i<count; i++)
	{
		if(memcmp(ID,&id,16) == 0)
			return true;
		else 
			ID++;
	}

	return false;
}
	
void buildSimBindingRequest(char *buf, UInt128 *ID)
{	
	StunMsgHdr header;
	char *p;

	header.msgType = BindingRequest;
	header.msgLength = 0;
	header.id = createTransctionID();
	*ID = header.id;
	p = encode16(buf, header.msgType);
	p = encode16(p, header.msgLength);
	memcpy(p, &header.id, sizeof(header.id));
}

void buildTestBindingRequest(char *buf, bool change_ip, bool change_port,
                              UInt128 *ID,char *usrname,bool change)
{
	StunMsgHdr header = {0};
	StunAtrHdr atr_head = {0};
	StunAtrChangeRequest atr = {0};
	char *p = NULL;
	int i = 0;
    
	//write stun header
	header.msgType = BindingRequest;

    //msgLength 是指 stun header 后 attributes 长度
	if(change)
		header.msgLength = MSG_IPCHANGE_LEN;
	else
		header.msgLength = MSG_LEN;

    
	header.id = createTransctionID();
	*ID = header.id;
	p = encode16(buf, header.msgType);
	p = encode16(p, header.msgLength);
	memcpy(p, &header.id, sizeof(header.id));
	p = p+sizeof(header.id);
    #if 0
	//write stun attributes
	atr_head.type = ChangeRequest;
	atr_head.length = 4;
	atr.value = 0x00000000;
	if(change_ip)
	{
		atr.value = atr.value|0x00000004;
	}
	if(change_port)
	{
		atr.value = atr.value|0x00000002;
	}
	atr.value = htonl(atr.value);
	p = encode16(p, atr_head.type);
	p = encode16(p, atr_head.length); 
	memcpy(p, &atr.value, sizeof(atr.value));
    #endif
	// write username
	#if 1
	atr_head.type = Username;
	atr_head.length = USER_NAME_LEN;
	p = encode16(p, atr_head.type);
	p = encode16(p, atr_head.length); 	
	memcpy(p, usrname, USER_NAME_LEN);
	p +=  USER_NAME_LEN;
	#else
	atr_head.type = Username;
	atr_head.length =22;
	p = encode16(p, atr_head.type);
	p = encode16(p, atr_head.length); 
	memcpy(p, usrname, 22);	
	p +=  strlen(usrname);;
	#endif


	// write 	ConnectionRequestBinding
	atr_head.type = ConnectionRequestBinding;
	atr_head.length = DSLFORUM_STR_LEN;
	p = encode16(p, atr_head.type);
	p = encode16(p, atr_head.length); 
	memcpy(p, szDslForum, DSLFORUM_STR_LEN);
	p += DSLFORUM_STR_LEN;


	if(change)
	{
		atr_head.type = BindingChange;
		atr_head.length = 0;
		p = encode16(p, atr_head.type);
		p = encode16(p, atr_head.length); 
	}
	

	//write unknow C800/C801
	for( i =0; i < 2; i++)
	{
		atr_head.type = 0xc800+i;
		atr_head.length = 4;
		atr.value = 0x00000001;
		atr.value = htonl(atr.value);
		p = encode16(p, atr_head.type);
		p = encode16(p, atr_head.length); 
		memcpy(p, &atr.value, sizeof(atr.value));
		p += 4;	
	}
	
}
	
/********************************************/
/* buf: point to Resquest you want to send  */
/* fd : file descriptor returned by socket  */
/* ip : address to send.(Network-endian)    */
/*port: port to send.(Network-endian)       */
/*  l : length of request you send          */
/********************************************/
bool sendBindingRequest(char *buf, int fd, UInt32 ip, UInt16 port, UInt16 l)
{
	int nbyte = 0;
	struct sockaddr_in server_addr = {0};

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ip;
	server_addr.sin_port = port;	

	nbyte = sendto(fd, buf, l, 0, (struct sockaddr *)&server_addr,
                    sizeof(server_addr));
	if(nbyte <= 0)
	{
        evcpe_debug(__func__,"sendBindingRequest you want to send %d bytes,but send % bytes only!\n",l, nbyte);
	    evcpe_debug(__func__,"sendBindingRequest send failed:%s\n",strerror(errno));
		return false;
	}
	
	return true;
}

 /****************************************************/
 /* buf: point to Response you have received         */
 /* fd : file descriptor returned by socket          */
 /*  l : max length you can receive                  */
 /****************************************************/
bool receiveResponse(char *buf, int fd, UInt16 l)
{
	int nbyte;
	int addr_len;
	struct sockaddr_in server_addr;
	
	addr_len = sizeof(server_addr);
	memset(&server_addr,0,addr_len);


	nbyte = recvfrom(fd, buf, l, 0,(struct sockaddr *)&server_addr,&addr_len);
	if(nbyte == -1)
	{
		return false;
	}
	printf("receive data from %s/%d\n",inet_ntoa(server_addr.sin_addr),
            ntohs(server_addr.sin_port));
		
	return true;
}

//if ipv6,rewrite this function
bool parseAtrAddress(const char *p, UInt16 length, StunAtrAddress4 *addr)
{	
	if(length != 8)return false;

	memcpy(&addr->ipv4.port, p+2, 2);
	memcpy(&addr->ipv4.addr, p+4, 4);
	addr->ipv4.port = ntohs(addr->ipv4.port);
	addr->ipv4.addr = ntohl(addr->ipv4.addr);
	
	return true;
}


/************************************************/
/*Unknown response return false                 */
/*incorrect response return false               */
/*correct response rertun true                  */
/************************************************/
bool parseBindingResponse(char *buf, Address4 *addr, UInt128 *ID, int count)
{	
	StunMessage msg = {0};
	char *p = buf;
	int len = 0;
	StunAtrHdr atr_head = {0};
	// init message
	msg.hasMappedAddress    = false;
	msg.hasSourceAddress    = false;
	msg.hasChangedAddress   = false;
	msg.hasMessageIntegrity = false;
	msg.hasReflectedFrom	= false;

	memcpy(&msg.msgHdr.msgType, p, 2);
	memcpy(&msg.msgHdr.msgLength, p+2, 2);
	memcpy(&msg.msgHdr.id, p+4, 16);
	/* sunjian  有时候解析 失败
	if(!checkTransctionID(msg.msgHdr.id,ID,count))
	{
		LOGI("parseBindingResponse incorrect ID!\n");
		return false;
	}
	*/
	p = p+20;
	msg.msgHdr.msgType = ntohs(msg.msgHdr.msgType);
	msg.msgHdr.msgLength = ntohs(msg.msgHdr.msgLength);
	len = msg.msgHdr.msgLength;
	//printf("message type:%x\t length:%d\n", msg.msgHdr.msgType,len);
	

	if(msg.msgHdr.msgType == BindingResponse)
	{	
	   //LOGI("parseBindingResponse recv BindingResponse ");
		while(len >0)
		{
			memcpy(&atr_head.type, p, 2);
			memcpy(&atr_head.length, p+2, 2);
			atr_head.type = htons(atr_head.type);
			atr_head.length = htons(atr_head.length);
			p = p+4;
			switch(atr_head.type)
			{
				case MappedAddress:
					msg.hasMappedAddress = true;
					if(!parseAtrAddress(p,atr_head.length,
							      &msg.mappedAddress))
					{
						evcpe_debug(__func__,"parse MappedAddress failed!\n");
						return false;
					}
					break;
				case SourceAddress:
					msg.hasSourceAddress = true;
					if(!parseAtrAddress(p,atr_head.length,
								&msg.sourceAddress))
					{
						evcpe_debug(__func__,"parse SourceAddress failed!\n");
						return false;
					}
					break;
				case ChangedAddress:
					msg.hasChangedAddress = true;
					if(!parseAtrAddress(p,atr_head.length,
								&msg.changedAddress))
					{
						evcpe_debug(__func__,"parse ChangedAddress failed!\n");
						return false;
				}
					break;
				case MessageIntegrity:break;
				case ReflectedFrom:break;
				default:
					evcpe_debug(__func__,"Unknown attribute:%x\n",atr_head.type);
					break;
			}
			len = len - 4 -atr_head.length;
			p = p+atr_head.length;
		}
		//output information to file
		if(msg.hasMappedAddress)
		{	char buf[128]={0};
			unsigned char *q = NULL;
			addr->mapped_ip = msg.mappedAddress.ipv4.addr;
			addr->mapped_port = msg.mappedAddress.ipv4.port;
			q = (unsigned char *)&msg.mappedAddress.ipv4.addr;
			evcpe_info(__func__,"MappedAddress:%d.%d.%d.%d:%d\n",*(q+3),*(q+2),*(q+1),
                    *q,msg.mappedAddress.ipv4.port);

		}
		else
		{
			evcpe_info(__func__ , "no MappedAddress attributes\n");
		}

		if(msg.hasSourceAddress)
		{
			UInt8 *q = (UInt8 *)&msg.sourceAddress.ipv4.addr;
			evcpe_info(__func__,"SourceAddress:%d.%d.%d.%d:%d\n",*(q+3),*(q+2),*(q+1),
				*q,msg.sourceAddress.ipv4.port);
		
		}
		else
		{
			evcpe_error(__func__,"no SourceAddress attribute!\n");
		}

		if(msg.hasChangedAddress)
		{
			unsigned char * q = NULL;
			addr->changed_ip = msg.changedAddress.ipv4.addr;
			addr->changed_port = msg.changedAddress.ipv4.port;
			q = (unsigned char *)&msg.changedAddress.ipv4.addr;
			evcpe_info(__func__,"ChangedAddress:%d.%d.%d.%d:%d\n",*(q+3),*(q+2),*(q+1),
				*q,msg.changedAddress.ipv4.port);
		}
		else
		{
			evcpe_debug(__func__,"no ChangedAddress attribute!");
		}
		
		if(msg.hasMessageIntegrity)
		{
			//do something
		}
		
		if(msg.hasReflectedFrom)
		{
			//do something
		}
	}//end of parse binding response
	
	else if(msg.msgHdr.msgType == BindingErrorResponse)
	{
		evcpe_debug(__func__,"error code\n");		
	}
	else
	{		
		evcpe_debug(__func__,"parseBindingResponseUnknown stun type!\n");
		return false;
	}
	printf("\n");

	return true;
}

/****************************************************************/
/*    bufReq : point to the content of Request                  */
/*    bufRes : point to the memory receive Response             */
/*      fd   : file descriptor returned by socket               */
/* change_ip : change ip or not                                 */
/*change_port: change port or not                               */
/*      ip   : address to send or receive                       */
/*     port  : port to send or receive                          */
/*      l    : data length to send                              */
/*    addr   : save useful ip or port                           */
/****************************************************************/
bool sendMessage(char *bufReq, char *bufRes,  int fd, bool change_ip,
		 bool change_port, UInt32 ip, UInt16 port, UInt16 l, Address4 *addr)
{
	int flag;
	int state;
	int args;
	int count = 0;
	const int timePoint[]={0,100,300,700,1500,3100,4700,6300,7900};
	UInt128 ID[10];
	int timeuse = 0;
	struct timeval tv1,tv2;

	initIdArray(ID,10);		
	flag = 1;
	state = Build;
	gettimeofday(&tv1,NULL);
	while(flag)
	{
		switch(state)
		{
			case Build:
				buildTestBindingRequest(bufReq,change_ip,change_port,&ID[count],szUserName,false);
				sendBindingRequest(bufReq,fd,ip,port,l);
				count++;
				state = WaitResponse;	
				break;
			case WaitResponse:	
				#ifdef WIN32
				{
					int blocking = 1;
					ioctlsocket(fd, FIONBIO, (unsigned long*)&blocking);
				}
				#else
				args = fcntl(fd, F_GETFL,0);
				fcntl(fd,F_SETFL,args|O_NONBLOCK);	
				#endif
				if(receiveResponse(bufRes,fd,256))
				{
					printf("LGL case WaitResponse: line462 receiveResponse!");
					state = ParseResponse;
					break;
				}
				gettimeofday(&tv2,NULL);
				timeuse = (tv2.tv_sec-tv1.tv_sec)*1000+
					    (tv2.tv_usec-tv1.tv_usec)/1000;
	
				if(timeuse > 9500)
				{
					printf("didn't receive response in 9.5s\n");
					return false;
				}
				else if(count < 9 && timeuse > timePoint[count])
				{
					state = Build;
				}
				break;
			case ParseResponse:
				if(parseBindingResponse(bufRes,addr,ID,count))
				{
					state = EndProcess;
				}
				else
				{
					printf("receive error response!\n");
					state = WaitResponse;
				}
				break;
			case EndProcess:
				flag = 0;
				break;
			default:
				break;
		}
	}
	return true;
}



//open a UDP socket,return FILE DESCRIPTOR
int openSocket(void)
{
	int fd;
	
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		fprintf(stderr,"open socket failed:%s\n",strerror(errno));
		exit(1);
	}
	
	return fd;                     
}

//close FILE DESCRIPTOR
void closeSocket(int fd)
{
	close(fd);
}

/*fd for FILE DESCRIPTOR                 */
/*return localhost's IP(local endian)    */
unsigned int getLocalIP(int fd)
{
	unsigned int ip = 0;
#if 0
	struct ifconf conf;
	struct ifreq *ifr;
	char buff[512];
	int num;
	int i;

	conf.ifc_len = sizeof(buff);
	conf.ifc_buf = buff;
	ioctl(fd, SIOCGIFCONF, &conf);
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;

	for(i=0; i<num; i++)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

		ioctl(fd, SIOCGIFFLAGS, ifr);
		if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
		{
			ip = ntohl(sin->sin_addr.s_addr);
			break;
		}
		ifr++;
	}
#else
    char buffer[128] = {0};
    memset(buffer , 0 ,sizeof(buffer));
	sk_func_porting_params_get("lan_ip",buffer,sizeof(buffer));
    if('\0' != buffer[0])
    {
        ip = ntohl(inet_addr(buffer));
    }
    else
    {
        ip = 0;
    }
    evcpe_info(__func__ , "sk_func_porting_params_get lan_ip = 0x%x\n",ip);
#endif
	return ip;
}




#if 0
//int main(int argc, char *argv[])
int stun(char *server_ip, char *user_name)
{
/*
	if(argc != 2)
	{
		printf("Usage:%s address",argv[1]);
		exit(1);
	}
*/	
	printf("Usage:%s address, %s username", server_ip, user_name);
	if(server_ip == NULL || user_name == NULL)
		return -1;
		
	Address4 addr;
	int fd,fd1;
	UInt32 ip,local_ip;
	char bufReq[REQ_LEN];
	char bufRes[RES_LEN];
	int num;
	unsigned int first_ip;
	unsigned short port,p;
	p = htons(PORT);
	ip = (UInt32)inet_addr(server_ip);//(UInt32)inet_addr(argv[1]);
	fd = openSocket();
	local_ip = getLocalIP(fd);

	memset(szUserName, '\0', USER_NAME_LEN);
	strncpy(szUserName, user_name, USER_NAME_LEN - 1);
		
	if(sendMessage(bufReq,bufRes,fd,false,false,ip,p,REQ_LEN,&addr))
	{	
		first_ip = addr.mapped_ip;
		port = addr.mapped_port;

		if(local_ip == addr.mapped_ip)
		{
			if(sendMessage(bufReq,bufRes,fd,true,true,ip,p,REQ_LEN,&addr))
			{
				printf("Open Internet!\n");
			}
			else printf("Symmetric UDP firewall!\n");
		}
		else
		{
			if(sendMessage(bufReq,bufRes,fd,true,true,ip,p,REQ_LEN,&addr))
			{
				printf("NAT type:Full Cone!\n");
			}
			else 
			{
				
				printf("changed ip is %x\n",htonl(addr.changed_ip));
				sendMessage(bufReq,bufRes,fd,false,false,
	  			        	htonl(addr.changed_ip),htons(addr.changed_port),
							REQ_LEN,&addr);
				printf("mapped_ip is %x\n",htonl(addr.mapped_ip));
				if(port == addr.mapped_port)
				{
					
					if(sendMessage(bufReq,bufRes,fd,false,true,ip,p,REQ_LEN,&addr))
					{
						printf("NAT type:Restricted NAT\n");
					}
					else printf("NAT type:Port Restricted NAT\n");
				}
				else printf("NAT type:Symmetric NAT\n");
				
			}
		}
	}
	else printf("Firewall that blocks UDP or UDP unreachable!\n ");

	closeSocket(fd);

	return 0;
}

int stun(char *server_ip, char *user_name)
{
/*
	if(argc != 2)
	{
		printf("Usage:%s address",argv[1]);
		exit(1);
	}
*/	
	Address4 addr;
	int fd;
	UInt32 ip,local_ip;
	char bufReq[REQ_LEN];
	char bufRes[RES_LEN];
	int num;
	unsigned int first_ip;
	unsigned short port,p;

	if(server_ip == NULL || user_name == NULL)
		return -1;
	printf("Usage:%s address, %s username", server_ip, user_name);
	p = htons(PORT);
	ip = (UInt32)inet_addr(server_ip);//(UInt32)inet_addr(argv[1]);
	fd =  openSocket();
	local_ip = getLocalIP(fd);
	memset(szUserName, '\0', USER_NAME_LEN + 1);
	strncpy(szUserName, user_name, USER_NAME_LEN);
 
#if 1
	sendMessage(bufReq,bufRes,fd,false,false,ip,p,REQ_LEN,&addr);
#else	
	if(sendMessage(bufReq,bufRes,fd,false,false,ip,p,REQ_LEN,&addr))
	{	
		first_ip = addr.mapped_ip;
		port = addr.mapped_port;

		if(local_ip == addr.mapped_ip)
		{
			if(sendMessage(bufReq,bufRes,fd,true,true,ip,p,REQ_LEN,&addr))
			{
				printf("Open Internet!\n");
			}
			else printf("Symmetric UDP firewall!\n");
		}
		else
		{
			if(sendMessage(bufReq,bufRes,fd,true,true,ip,p,REQ_LEN,&addr))
			{
				printf("NAT type:Full Cone!\n");
			}
			else 
			{
				
				printf("changed ip is %x\n",htonl(addr.changed_ip));
				sendMessage(bufReq,bufRes,fd,false,false,
	  			        	htonl(addr.changed_ip),htons(addr.changed_port),
							REQ_LEN,&addr);
				printf("mapped_ip is %x\n",htonl(addr.mapped_ip));
				if(port == addr.mapped_port)
				{
					
					if(sendMessage(bufReq,bufRes,fd,false,true,ip,p,REQ_LEN,&addr))
					{
						printf("NAT type:Restricted NAT\n");
					}
					else printf("NAT type:Port Restricted NAT\n");
				}
				else printf("NAT type:Symmetric NAT\n");
				
			}
		}
	}
	else printf("Firewall that blocks UDP or UDP unreachable!\n ");
#endif
	closeSocket(fd);

	return 0;
}
#endif


