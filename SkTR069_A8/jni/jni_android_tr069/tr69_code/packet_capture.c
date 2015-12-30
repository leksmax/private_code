#include <stdlib.h> 
#include <string.h>
#include "packet_capture.h"
#include "log.h"

static int sk_start_pcaket_capture_by_tcpdump(evcpe_packet_capture_t * p_packet_capture);

int sk_start_packet_capture_diagnostics(evcpe_packet_capture_t * p_packet_capture)
{   
    int ret = 0;
    
    p_packet_capture->state = 3;   

    ret = sk_start_pcaket_capture_by_tcpdump(p_packet_capture);

    if(-1 == ret)
    {
        evcpe_error(__func__ , "sk_start_pcaket_capture_by_tcpdump failed!");
        return -1;    
    }
    
    return sk_func_porting_params_set("tr069_pcap_start","");
        
}

#ifdef WIN32
static int sk_start_pcaket_capture_by_tcpdump(evcpe_packet_capture_t * p_packet_capture)
{
	return 0;
}
#else
static int sk_start_pcaket_capture_by_tcpdump(evcpe_packet_capture_t * p_packet_capture)
{
    char  ip_addr[64] = {0};
    int   ip_port = 0;
    char port_buf[32] = {0};
    bindermsg msg = {0};
	char cmdkey[1024] = {0};
    char duration_buf[32] = {0};
    
    strcpy(ip_addr , p_packet_capture->ip_addr);
    ip_port = p_packet_capture->ip_port;
    
	strcpy(cmdkey , "tcpdump -s 0 -C 1 -w tcpdump.pcap");
    if('\0' != ip_addr[0])
    {
        strcat(cmdkey ," host ");
        strcat(cmdkey , ip_addr);
    }

    if(0 != ip_port)
    {
        if('\0' != ip_addr[0])
        {
            strcat(cmdkey , " and");
        }
        sprintf(port_buf , "%d",ip_port);
        strcat(cmdkey , " port ");
        strcat(cmdkey , port_buf);
        
    }

    sprintf(duration_buf , "%d" ,p_packet_capture->duration);

    strcat(cmdkey , "|timeout=");
    strcat(cmdkey  , duration_buf);
    
    evcpe_info(__func__,"call cmd=%s \n" , cmdkey);
    
	strcpy(msg.user,"TR069");
	strcpy(msg.msg, cmdkey);
	msg.cmd = 0;
	msg.type = START_TCPDUMP;
	msg.len = strlen(msg.msg);

    evcpe_info(__func__,"sk_binder_send enter\n");
    sk_binder_send(&msg,sizeof(msg));	
    evcpe_info(__func__,"sk_binder_send exit\n");
	return 0;
}
#endif