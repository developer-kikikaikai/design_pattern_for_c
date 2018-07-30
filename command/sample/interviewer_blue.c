#include "interviewer.h"
#include "stdio.h"
#define INTERVIEW_TAG "[interview_blue]"

static Answer answer_for_interviewer_blue_g;
static void interview_blue_for_ir() {
	printf("インフラレッドへのインタビュー\n");
	printf(INTERVIEW_TAG"(興味がなさそうだ)市民、幸福は義務です。あなたは幸福ですか？\n");
	printf("\t%s\n", answer_for_interviewer_blue_g->answer(ARE_YOU_HAPPY));
}

static void interview_blue_for_red() {
	printf("レッドへのインタビュー\n");
	printf(INTERVIEW_TAG"市民、幸福は義務です。あなたは幸福ですか？\n");
	printf("\t%s\n", answer_for_interviewer_blue_g->answer(ARE_YOU_HAPPY));
	printf(INTERVIEW_TAG"よろしい。さて、ミュータントは見つかりましたか？\n");
	printf("\t%s\n", answer_for_interviewer_blue_g->answer(DO_YOU_KNOW_MUTANT));
	printf(INTERVIEW_TAG"そうですか、では後でZAPしておきましょう。ところで市民%s、私の500クレジットを見かけませんでしたか？\n", answer_for_interviewer_blue_g->getname());
	printf("\t%s\n", answer_for_interviewer_blue_g->answer(HIGH_CLEARANCE));
	printf(INTERVIEW_TAG"…まあいいでしょう。さっさとミュータントを探してください。\n");
}

static void interview_blue_for_blue() {
	printf("ブルーへのインタビュー\n");
	printf(INTERVIEW_TAG"特に会話なし\n");
}

static void (*interview_table_g[])()={
	interview_blue_for_ir,
	interview_blue_for_red,
	interview_blue_for_blue,
};

static Answer answer_for_interviewer_blue_g;
static void interview_blue(void) {
	interview_table_g[answer_for_interviewer_blue_g->color()]();
}

static interviewer_t blue={interview_blue};

Interviewer interviewer_blue_new(Answer answer) {
	printf("%s\n",__func__);
	answer_for_interviewer_blue_g=answer;
	return &blue;
}
