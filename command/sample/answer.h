#ifndef ANSWER_IF
#define ANSWER_IF
enum question_e {
	DO_YOU_HAVE_CAR,
	DO_YOU_KNOW_MUTANT,
	ARE_YOU_HAPPY,
	HIGH_CLEARANCE
};

typedef enum color_e {
	INFRA_RED,
	RED,
	BLUE,
} color_e;

struct answer_t {
	color_e (*color)();
        char *(*getname)();
        char *(*answer)(int id);
};
typedef struct answer_t answer_t, *Answer;

Answer answer_infra_red_new();
Answer answer_red_new();
Answer answer_blue_new();
#endif
