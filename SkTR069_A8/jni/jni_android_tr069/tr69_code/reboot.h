#ifndef EVCPE_REBOOT_H_
#define EVCPE_REBOOT_H_
struct evcpe_reboot
{
	char command_key[32];
};

struct evcpe_reboot * evcpe_reboot_new(void);
void evcpe_reboot_free(struct evcpe_reboot * method);
struct evcpe_reboot_response
{
	char 	commandkey[32] ;	//当前事务交互的内容，参考 TR-069 的说明
 	
};

struct evcpe_reboot_response *evcpe_reboot_response_new(void);

void evcpe_reboot_response_free(struct evcpe_reboot_response *method);

#endif   // EVCPE_REBOOT_H_
