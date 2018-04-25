#include <stdio.h>
#include "flyweight.h"
#include "members.h"
#include "culture.h"

static char * us_answer(int id) {
	static char *answer[]={
		"ここには何でもあるよ、ハンバーガー、ピザ。最近はJapanese Sushiも人気だね！HAHAHA", 
		"幸せかいだって？どうこの笑顔、これが不幸な人間に見えるかい！", 
		"ここまでちゃんと俺に質問出来てるんだろ？十分じゃないかい？", 
	};

	return answer[id];
}

static char * us_name() {
	return "ジョージ";
}

static char * us_introduce() {
	static char introduce[256]={0};
	sprintf(introduce, "俺かい？アメリカ人の%sってんだ。よろしくな！", us_name());

	return introduce;
}

static void us_constructor(void *src) {
	struct culture_if *culture_if = (struct culture_if *)src;
	culture_if->introduce = us_introduce;
	culture_if->get_name = us_name;
	culture_if->answer = us_answer;
}

int US_new() {
	struct flyweight_init_s method={
		us_constructor,
		NULL,
		NULL
	};

	int id = flyweight_register_class(sizeof(struct culture_if), 0, &method);
	return id;
}

