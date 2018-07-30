#include "stdio.h"
#include "answer.h"

static color_e color_ir() {
	return INFRA_RED;
}

static char * getname_ir() {
	return "トム";
}

static char * answer_ir(int id) {
	switch(id) {
	case DO_YOU_HAVE_CAR:
		return "はい、持っています。";
	case DO_YOU_KNOW_MUTANT:
		return "いいえ、知りません。";
	case ARE_YOU_HAPPY:
		return "はい、幸福は義務です。";
	case HIGH_CLEARANCE:
		return "…私の足元の500クレジットがおちておりました。";
	default:
		break;
	}
}

static answer_t answer_ir_g={color_ir, getname_ir, answer_ir};
Answer answer_infra_red_new() {
	printf("%s\n",__func__);
	return &answer_ir_g;
}
