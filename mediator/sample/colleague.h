#ifndef COLLEAGUE_
#define COLLEAGUE_
typedef enum action_e {
	ATTACK,
	DEFENSE,
	HEAL,
} action_e;

typedef enum job_e {
	WARRIOR,
	PRIEST,
	WITCH,
} job_e;


struct colleague_t;
typedef struct colleague_t *Colleague;

struct colleague_t {
	job_e (*getjob)(Colleague this);
	int (*gethp)(Colleague this);
	int (*getcurhp)(Colleague this);
	int (*action)(Colleague this, action_e action);
	void (*damage)(Colleague this, int value);
};

#define COLLEAGUE_IF\
	job_e (*getjob)(Colleague this);\
	int (*gethp)(Colleague this);\
	int (*getcurhp)(Colleague this);\
	int (*action)(Colleague this, action_e action);\
	void (*damage)(Colleague this, int value);

#endif
