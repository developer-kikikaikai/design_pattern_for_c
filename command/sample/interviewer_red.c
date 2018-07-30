#include "interviewer.h"
#include "stdio.h"
#define TAG "[interview_red]"
static Answer answer_for_interviewer_red_g;
static void interview_red_for_ir() {
	printf("インフラレッドへのインタビュー\n");
	printf(TAG"市民、車は持っていますか？\n");
	printf("\t%s\n", answer_for_interviewer_red_g->answer(DO_YOU_HAVE_CAR));
	printf(TAG"ではそれは今から私のものです。いいですね。\n");
	printf(TAG"ところで市民、ミュータントの居所を知りませんか？\n");
	printf("\t%s\n", answer_for_interviewer_red_g->answer(DO_YOU_KNOW_MUTANT));
	printf(TAG"もう一度聞きます、市民%s。ミュータントの居所か私の500クレジットを知りませんか？\n", answer_for_interviewer_red_g->getname());
	printf("\t%s\n", answer_for_interviewer_red_g->answer(HIGH_CLEARANCE));
	printf(TAG"ありがとう、インフラレッド。(ZAPをしてその場を去る\n");
}

static void interview_red_for_red() {
	printf("レッドへのインタビュー\n");
	printf(TAG"市民%s, あなたは車を持っていますか？\n", answer_for_interviewer_red_g->getname());
	printf("\t%s\n", answer_for_interviewer_red_g->answer(DO_YOU_HAVE_CAR));
	printf(TAG"ではそれでブリ―フィングルームへ向かいましょう。\n");
}

static void interview_red_for_blue() {
	printf("ブルー様へのインタビュー\n");
	printf(TAG"ブルー様、ミュータントの居所についてご存知ないですか？\n");
	printf("\t%s\n", answer_for_interviewer_red_g->answer(DO_YOU_KNOW_MUTANT));
	printf(TAG"(ぐぬぬ)それではブルー様、お車をお持ちではないですか？ミュータントの居所を探るために使用したいのですが。\n");
	printf("\t%s\n", answer_for_interviewer_red_g->answer(DO_YOU_HAVE_CAR));
	printf(TAG"ですが%sさｍ\n", answer_for_interviewer_red_g->getname());
	printf("\t%s\n", answer_for_interviewer_red_g->answer(HIGH_CLEARANCE));
}

static void (*interview_table_g[])()={
	interview_red_for_ir,
	interview_red_for_red,
	interview_red_for_blue,
};

static void interview_red() {
	interview_table_g[answer_for_interviewer_red_g->color()]();
}

static interviewer_t red_g;

Interviewer interviewer_red_new(Answer answer) {
	printf("%s\n",__func__);
	answer_for_interviewer_red_g=answer;
	red_g.interview=interview_red;
	return &red_g;
}
