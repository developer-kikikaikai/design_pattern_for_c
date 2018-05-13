#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "colleagues.h"

typedef struct moster {
	int hp;
	int base_damage;
	int area;
} moster_t;

void get_monster(moster_t * moster) {
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);

	moster->hp = 100+ time.tv_nsec%50;
	moster->base_damage = 10 + time.tv_nsec%10;
}

Colleague get_colleage(Mediator mediator) {
	static Colleague (*members[3])(Mediator) ={
		warrior_new,
		priest_new,
		witch_new
	};

	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return members[time.tv_nsec%3](mediator);
}

int main(int argc, char *argv[]) {
	if(argc<3) {
		printf("Usage: %s <mode(0:GANGAN_IKOZE!, other:INOCHI_DIJINI)> <member_count>\n", argv[0]);
		return -1;
	}

	int status = atoi(argv[1]);
	int member_count = atoi(argv[2]);

	printf("status:%d, member=%d\n", status, member_count);

	Mediator mediator = mediator_new(status);

	int i=0;
	for(i=0;i<member_count;i++) {
		mediator_add_colleague(mediator, get_colleage(mediator));
	}

	moster_t moster;
	get_monster(&moster);
	int damage;
	struct timespec time;
	printf("|moster\t|hp:%d, base damage:%d|\n", moster.hp, moster.base_damage);
	while(mediator_is_member(mediator)) {
		printf("|moster\t|hp:%d|\n", moster.hp);
		moster.hp -= mediator_ownturn(mediator);
		if(moster.hp <= 0) {
			break;
		}

		mediator_damage(mediator, moster.base_damage + time.tv_nsec%5);
		printf("Please enter\n");
		getchar();
	}

	if(mediator_is_member(mediator)) {
		printf("Player is WIN!!\n");
	} else {
		printf("Player is LOSE..\n");
	}

	mediator_free(mediator);
	return 0;
}
