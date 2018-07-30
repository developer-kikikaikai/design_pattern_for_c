#include "stdio.h"
#include "answer.h"

static color_e color_red() {
	return RED;
}

static char * getname_red() {
	return "スミス";
}

static char * answer_red(int id) {
	switch(id) {
	case DO_YOU_HAVE_CAR:
		return "いいえ、持っていません。そこのインフラレッドのそばにありますね。";
	case DO_YOU_KNOW_MUTANT:
		return "あそこのインフラレッドがミュータントです！ZAPしましょう！";
	case ARE_YOU_HAPPY:
		return "はい、幸福は義務です。";
	case HIGH_CLEARANCE:
		return "100クレジットしか落ちていませんでした～(以降無言で靴をなめ続ける";
	default:
		break;
	}
}

static answer_t answer_red_g={color_red, getname_red, answer_red};

Answer answer_red_new() {
	printf("%s\n",__func__);
	return &answer_red_g;
}
