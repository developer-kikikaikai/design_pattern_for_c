#include <stdlib.h>
#include "colleague_warrior.h"
struct warrior_colleague_t {
COLLEAGUE_IF
	int maxhp;
	int curhp;
	float guard;
	//脳筋なのでピンチなんて報告しない
};

#define DEFAULT_MAXHP (30)
typedef struct warrior_colleague_t * WarriorColleague;

static job_e warrior_getjob(Colleague instance) {
	return WARRIOR;
}

static int warrior_gethp(Colleague this) {
	WarriorColleague instance = (WarriorColleague)this;
	return instance->maxhp;
}

static int warrior_getcurhp(Colleague this) {
	WarriorColleague instance = (WarriorColleague)this;
	return instance->curhp;
}

static int warrior_action(Colleague this, action_e action) {
	WarriorColleague instance = (WarriorColleague)this;
	int damage=0;
	switch(action) {
	case DEFENSE:
		instance->guard = 0.4;
		break;
	case ATTACK:
		instance->guard = 0.8;
		damage = 10;
		break;
	default:
		break;
	}

	return damage;
}

static void warrior_damage(Colleague this, int value) {
	WarriorColleague instance = (WarriorColleague)this;
	instance->curhp -= value * instance->guard;
}

Colleague warrior_new(Mediator mediator) {
	WarriorColleague instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}
	instance->maxhp = DEFAULT_MAXHP + rand()%20;
	instance->curhp = instance->maxhp;
	instance->guard = 0.8;

	instance->getjob = warrior_getjob;
	instance->gethp = warrior_gethp;
	instance->getcurhp = warrior_getcurhp;
	instance->action = warrior_action;
	instance->damage = warrior_damage;
	return (Colleague)instance;
}
