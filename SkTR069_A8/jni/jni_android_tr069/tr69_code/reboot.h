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
	char 	commandkey[32] ;	//��ǰ���񽻻������ݣ��ο� TR-069 ��˵��
 	
};

struct evcpe_reboot_response *evcpe_reboot_response_new(void);

void evcpe_reboot_response_free(struct evcpe_reboot_response *method);

#endif   // EVCPE_REBOOT_H_
