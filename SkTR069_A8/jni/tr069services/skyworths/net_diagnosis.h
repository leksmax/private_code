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
	char szHost[512];				//����ping��ϵ����������ַ
	int nNumberOfRepetitions;		//�ڱ�����֮ǰ��ping����ظ��Ĵ���
	int nTimeout;					//�ú����ʾ��ping��ϳ�ʱʱ��
	int nDataBlockSize;				//ÿ��ping����͵����ݴ�С�����ֽ�Ϊ��λ��Ҫ��̶���СΪ32�ֽ�
	int nTTL;						//TTLֵ��Ĭ��Ϊ64
	int nDSCP;						//���԰�������DiffServ����㣬Ĭ��ֵΪ0
}sk_ping_attribute_t;

typedef struct ping_results_s
{
	char szHost[64];				//Ӧ��ping��ϵ�������ַ��
	int nTotalCount;				//�������ping�����е��ܴ�����
	int nSuccessCount;				//�������ping�����гɹ��Ĵ���
	int nFailureCount;				//�������ping������ʧ�ܵĴ���
	int nAverageResponseTime;		//�Ժ���Ϊ��λ�����һ��ping�������гɹ���Ӧ��ƽ��ʱ��
	int nMinimumResponseTime;		//�Ժ���Ϊ��λ�����һ��ping�������гɹ���Ӧ�е����ʱ��
	int nMaximumResponseTime;		//�Ժ���Ϊ��λ�����һ��ping�������гɹ���Ӧ�е��ʱ��
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
	char szHost[512];				//����·����ϵ����������ַ
	int nTimeout;					//�ú����ʾ��·����ϳ�ʱʱ��
	int nDataBlockSize;				//ÿ��·����Ϸ��͵����ݴ�С�����ֽ�Ϊ��λ��Ҫ��̶���СΪ32�ֽ�
	int nMaxHopCount;				//���͵Ĳ������ݰ����������(���TTL��)��Ĭ��Ϊ30��
	int nDSCP;						//���԰�������DiffServ����㣬Ĭ��ֵΪ0
}sk_traceroute_attribute_t;

typedef struct traceroute_results_s
{
	char szHost[64];				//����·����ϵ�������ַ
	int nResponseTime;				//�Ժ����ʾ�����һ��·���������Ե���Ӧʱ�䣬����޷���������·�ɣ���Ĭ��Ϊ0
	int nNumberOfRouteHops;			//���ڷ���·�ɵ�����������޷�����·�ɣ���Ĭ��Ϊ0
	hop_host_t gHopHost[MAX_TTL];	//·��·��
	char nStatus[64];               // "None" "Requested" "Complete" "Error_CannotResolveHostName" "Error_Internal" "Error_Other"
}sk_traceroute_results_t;



int sk_start_ping(const sk_ping_attribute_t* pPingAttribute, sk_ping_results_t* pPingResults);
int sk_start_traceroute(const sk_traceroute_attribute_t* pTracerouteAttribute, sk_traceroute_results_t* pTracerouteResults);


# ifdef __cplusplus
}
#endif


#endif

