#include "mediator.h"
#include "dp_util.h"
struct mediator_colleague_list;
typedef struct mediator_colleague_list * ColleagueList;

struct mediator_colleague_list {
	ColleagueList next;
	ColleagueList prev;
	Colleague colleague;
};

struct mediator_t {
	ColleagueList head;
	ColleagueList tail;
	int (*action)(Mediator this);
	int notify_cnt;
};

typedef int (*mediator_action)(Mediator);
static mediator_action get_action(mediator_status_e status);
static int mediator_ownturn_inochi_daijini(Mediator this);
static int mediator_ownturn_gangan(Mediator this);

static ColleagueList mediator_pop(Mediator this) {
	return (ColleagueList)dputil_list_pop((DPUtilList)this);
}

static void mediator_push(Mediator this, ColleagueList data) {
	dputil_list_push((DPUtilList)this, (DPUtilListData)data);
}

#define JOBNAME(job) 	(job)==WARRIOR?"WARRIOR "\
			:(job)==PRIEST?" PRIEST "\
			              :" WITCH  "

static void show_order(Mediator this) {
	printf("|   job   \t|HP\t|\n");
	ColleagueList member=this->head;
	while(member) {
		printf("|%s\t| %d/%d\t|\n",JOBNAME(member->colleague->getjob(member->colleague)), member->colleague->getcurhp(member->colleague), member->colleague->gethp(member->colleague));
		member=member->next;
	}
}

static mediator_action get_action(mediator_status_e status) {
	if(status == GANGAN_IKOZE) {
		return mediator_ownturn_gangan;
	} else {
		return mediator_ownturn_inochi_daijini;
	}
}

static int get_heal(Mediator this) {
	int heal=0;
	ColleagueList member = this->head;
	while(member) {
		heal += member->colleague->action(member->colleague, HEAL);
		member = member->next;
	}
	return heal;
}

static void heal_all(Mediator this) {
	int heal = get_heal(this);
	printf("heal\n");
	ColleagueList member = this->head;
	while(member) {
		member->colleague->damage(member->colleague, heal);
		member = member->next;
	}
}

static int mediator_ownturn_inochi_daijini(Mediator this) {
	printf("inochi-daijini\n");

	//if report, call heal_all
	if(this->notify_cnt) {
		heal_all(this);
		this->notify_cnt=0;
	}

	show_order(this);
	//guard if HP <= 5
	int damage=0;
	ColleagueList member = this->head;
	while(member) {
		if(member->colleague->getcurhp(member->colleague) <= 5) {
			printf("%s DEFENSE!\n", JOBNAME(member->colleague->getjob(member->colleague)));
			member->colleague->action(member->colleague, DEFENSE);
		} else {
			printf("%s ATTACK!\n", JOBNAME(member->colleague->getjob(member->colleague)));
			damage += member->colleague->action(member->colleague, ATTACK);
		}
		member = member->next;
	}
	return damage;
}

static int mediator_ownturn_gangan(Mediator this) {
	printf("gangan-ikoze!\n");
	show_order(this);
	//don't call heal_all without 4 time notify
	if(this->notify_cnt==4) {
		heal_all(this);
		this->notify_cnt=0;
	}

	//always attack!
	int damage=0;
	ColleagueList member = this->head;
	while(member) {
		damage += member->colleague->action(member->colleague, ATTACK);
		printf("%s ATTACK!\n", JOBNAME(member->colleague->getjob(member->colleague)));
		member = member->next;
	}
	return damage;
}

Mediator mediator_new(mediator_status_e status) {
	Mediator instance = calloc(1, sizeof(*instance));
	if(!instance) {
		return NULL;
	}

	instance->action = get_action(status);
	return instance;
}

void mediator_add_colleague(Mediator this, Colleague colleague) {
	ColleagueList member = calloc(1, sizeof(*member));
	member->colleague = colleague;

	mediator_push(this, member);
}

int mediator_ownturn(Mediator this) {
	return this->action(this);
}

void mediator_member_warning(Mediator this) {
	printf("Member is pintch!!\n");
	this->notify_cnt++;
}

void mediator_damage(Mediator this, int value) {
	ColleagueList member=this->head;
	while(member) {
		member->colleague->damage(member->colleague, value);
		if(member->colleague->getcurhp(member->colleague) <= 0) {
			printf("%s is dead\n", JOBNAME(member->colleague->getjob(member->colleague)));
			ColleagueList prev_member = member;
			member=member->next;
			dputil_list_pull((DPUtilList)this, (DPUtilListData)prev_member);
			free(prev_member->colleague);
			free(prev_member);
		} else {
			member=member->next;
		}
	}
}

int mediator_is_member(Mediator this) {
	return (this->head!=NULL);
}

void mediator_free(Mediator this) {
	ColleagueList member = mediator_pop(this);
	while(member) {
		free(member->colleague);
		free(member);
		member=mediator_pop(this);
	}
}
