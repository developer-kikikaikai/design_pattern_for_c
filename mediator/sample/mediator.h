#ifndef MEDIATOR_
#define MEDIATOR_

#include "colleague.h"

struct mediator_t;
typedef struct mediator_t *Mediator;
typedef enum mediator_status_e {
	GANGAN_IKOZE,
	INOCHI_DIJINI,
} mediator_status_e;

Mediator mediator_new(mediator_status_e status);
//add colleague
void mediator_add_colleague(Mediator this, Colleague colleague);
//own attack turn
int mediator_ownturn(Mediator this);
//member report warning
void mediator_member_warning(Mediator this);
//damage value
void mediator_damage(Mediator this, int value);
//check
int mediator_is_member(Mediator this);
//free
void mediator_free(Mediator this);
#endif
