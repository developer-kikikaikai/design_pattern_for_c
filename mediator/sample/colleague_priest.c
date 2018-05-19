#include <stdio.h>
#include <stdlib.h>
#include "colleague_priest.h"
struct priest_colleague_t {
COLLEAGUE_IF
	int maxhp;
	int curhp;
	int mp;
	float guard;
	Mediator mediator;
};

#define DEFAULT_MAXHP (20)
#define DEFAULT_MAXMP (10)
#define MP_USE (2)
typedef struct priest_colleague_t * PriestColleague;

static job_e priest_getjob(Colleague this) {
	return PRIEST;
}

static int priest_gethp(Colleague this) {
	PriestColleague instance = (PriestColleague)this;
	return instance->maxhp;
}

static int priest_getcurhp(Colleague this) {
	PriestColleague instance = (PriestColleague)this;
	return instance->curhp;
}

static int priest_action(Colleague this, action_e action) {
	PriestColleague instance = (PriestColleague)this;
	int damage=0;
	switch(action) {
	case DEFENSE:
		instance->guard = 0.5;
		break;
	case ATTACK:
		instance->guard = 1;
		damage = 3;
		break;
	default:
		if(MP_USE < instance->maxhp) {
			damage = -2;
			instance->mp-=MP_USE;
		} else {
			printf("priest, no MP\n");
		}
		break;
	}
	return damage;
}

static void priest_damage(Colleague this, int value) {
	PriestColleague instance = (PriestColleague)this;
	instance->curhp -= value * instance->guard;
	if(instance->curhp < value * instance->guard ) {
		//If he/she may be dead next turn, notify to mediator
		mediator_member_warning(instance->mediator);
	}
}

Colleague priest_new(Mediator mediator) {
	PriestColleague instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}
	instance->maxhp = DEFAULT_MAXHP + rand()%20;
	instance->curhp = instance->maxhp;
	instance->guard = 1;
	instance->mp = DEFAULT_MAXMP + rand()%5;
	instance->mediator = mediator;

	instance->getjob = priest_getjob;
	instance->gethp = priest_gethp;
	instance->getcurhp = priest_getcurhp;
	instance->action = priest_action;
	instance->damage = priest_damage;
	return (Colleague)instance;
}
