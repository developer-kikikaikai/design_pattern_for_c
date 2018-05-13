#include <stdlib.h>
#include "colleague_witch.h"
struct witch_colleague_t {
COLLEAGUE_IF
	int maxhp;
	int curhp;
	int mp;
	float guard;
	Mediator mediator;
};

#define DEFAULT_MAXHP (15)
#define DEFAULT_MAXMP (30)
#define MP_USE (2)
typedef struct witch_colleague_t * WitchColleague;

static job_e witch_getjob(Colleague this) {
	return WITCH;
}

static int witch_gethp(Colleague this) {
	WitchColleague instance = (WitchColleague)this;
	return instance->maxhp;
}

static int witch_getcurhp(Colleague this) {
	WitchColleague instance = (WitchColleague)this;
	return instance->curhp;
}

static int witch_action(Colleague this, action_e action) {
	WitchColleague instance = (WitchColleague)this;
	int damage=0;
	switch(action) {
	case DEFENSE:
		instance->guard = 0.7;
		break;
	case ATTACK:
		instance->guard = 1.2;
		if(MP_USE < instance->maxhp) {
			damage = 15;
			instance->maxhp -= MP_USE;
		} else {
			damage = 1;
		}
		break;
	default:
		break;
	}
	return damage;
}

static void witch_damage(Colleague this, int value) {
	WitchColleague instance = (WitchColleague)this;
	instance->curhp -= value * instance->guard;
	if(instance->curhp < value * instance->guard * 2) {
		//次2発で死ぬときはピンチ通知
		mediator_member_warning(instance->mediator);
	}
}

Colleague witch_new(Mediator mediator) {
	WitchColleague instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}
	instance->maxhp = DEFAULT_MAXHP + rand()%20;
	instance->curhp = instance->maxhp;
	instance->guard = 1;
	instance->mp = DEFAULT_MAXMP + rand()%20;
	instance->mediator = mediator;

	instance->getjob = witch_getjob;
	instance->gethp = witch_gethp;
	instance->getcurhp = witch_getcurhp;
	instance->action = witch_action;
	instance->damage = witch_damage;
	return (Colleague)instance;
}
