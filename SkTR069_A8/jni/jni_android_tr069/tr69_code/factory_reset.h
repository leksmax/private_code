#ifndef EVCPE_FACTORY_RESET_H_
#define EVCPE_FACTORY_RESET_H_
struct evcpe_factory_reset
{
	int reserved;
};

struct evcpe_factory_reset * evcpe_factory_reset_new(void);
void evcpe_factory_reset_free(struct evcpe_factory_reset * method);
struct evcpe_factory_reset_response
{
    int reserved;
};

struct evcpe_factory_reset_response *evcpe_factory_reset_response_new(void);

void evcpe_factory_reset_response_free(struct evcpe_factory_reset_response *method);
#endif   // EVCPE_factory_reset_H_
