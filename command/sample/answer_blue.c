#include "stdio.h"
#include "answer.h"

static color_e color_blue() {
	return BLUE;
}

static char * getname_blue() {
	return "ジョニー";
}

static char * answer_blue(int id) {
	switch(id) {
	case DO_YOU_HAVE_CAR:
		return "持っていますが、なにか？";
	case DO_YOU_KNOW_MUTANT:
		return "それを探し当てるのがあなたの仕事でしょう";
	case ARE_YOU_HAPPY:
		return "もちろん、幸福ですよ。";
	default:
		return "どこで私の情報を得たのだ！？ミュータントだな貴様！ZAP!ZAP!ZAP!";
	}
}

static answer_t answer_blue_g={color_blue, getname_blue, answer_blue};
Answer answer_blue_new() {
	printf("%s\n",__func__);
	return &answer_blue_g;
}
