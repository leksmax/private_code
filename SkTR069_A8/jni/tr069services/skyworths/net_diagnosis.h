#ifndef __NET_DIAGNOSIS_H__
#define __NET_DIAGNOSIS_H__

#define MAX_TTL							256
#define TRACERT_MAX_HOP_COUNT_DEFAULT	30

# ifdef __cplusplus
extern "C"
{
#endif

typedef struct ping_attribute_s
{
	char szHost[512];				//用于ping诊断的主机名或地址
	int nNumberOfRepetitions;		//在报告结果之前，ping诊断重复的次数
	int nTimeout;					//用毫秒表示的ping诊断超时时间
	int nDataBlockSize;				//每个ping命令发送的数据大小，以字节为单位，要求固定大小为32字节
	int nTTL;						//TTL值，默认为64
	int nDSCP;						//测试包中用于DiffServ的码点，默认值为0
}sk_ping_attribute_t;

typedef struct ping_results_s
{
	char szHost[64];				//应答ping诊断的主机地址；
	int nTotalCount;				//在最近的ping测试中的总次数；
	int nSuccessCount;				//在最近的ping测试中成功的次数
	int nFailureCount;				//在最近的ping测试中失败的次数
	int nAverageResponseTime;		//以毫秒为单位的最近一次ping测试所有成功响应的平均时间
	int nMinimumResponseTime;		//以毫秒为单位的最近一次ping测试所有成功响应中的最短时间
	int nMaximumResponseTime;		//以毫秒为单位的最近一次ping测试所有成功响应中的最长时间
	char nStatus[64];               // "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"
}sk_ping_results_t;
 


typedef struct hop_host_s
{
	int	 nTTL;
	char szHost[64];
	int  flag;
}hop_host_t;

typedef struct traceroute_attribute_s
{
	char szHost[512];				//用于路由诊断的主机名或地址
	int nTimeout;					//用毫秒表示的路由诊断超时时间
	int nDataBlockSize;				//每个路由诊断发送的数据大小，以字节为单位，要求固定大小为32字节
	int nMaxHopCount;				//发送的测试数据包的最大跳数(最大TTL数)，默认为30跳
	int nDSCP;						//测试包中用于DiffServ的码点，默认值为0
}sk_traceroute_attribute_t;

typedef struct traceroute_results_s
{
	char szHost[64];				//用于路由诊断的主机地址
	int nResponseTime;				//以毫秒表示的最近一次路由主机测试的响应时间，如果无法决定具体路由，则默认为0
	int nNumberOfRouteHops;			//用于发现路由的跳数，如果无法决定路由，则默认为0
	hop_host_t gHopHost[MAX_TTL];	//路由路径
	char nStatus[64];               // "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"
}sk_traceroute_results_t;



int sk_start_ping(const sk_ping_attribute_t* pPingAttribute, sk_ping_results_t* pPingResults);
int sk_start_traceroute(const sk_traceroute_attribute_t* pTracerouteAttribute, sk_traceroute_results_t* pTracerouteResults);


# ifdef __cplusplus
}
#endif


#endif

