#include <stdio.h>
#include "members.h"
#include "flyweight.h"
#include "culture.h"

static char *jp_answer(int id ) {
	static char * answer[]={
		"お米と味噌汁、後はお寿司ですね。海外のお寿司は別物です。", 
		"一応仕事もあって休みも取れてますし、幸せだと思います…", 
		"どうなんでしょう、多分大丈夫じゃないでしょうか。", 
};

	return answer[id];
}

static char * jp_name() {
	return "鬼瓦";
}

static char * jp_introduce() {
	static char introduce[256]={0};
	sprintf(introduce, "あ、日本人です。%sといいます。", jp_name());
	return introduce;
}

static void jp_constructor(void *src) {
	struct culture_if *culture_if = (struct culture_if *)src;
	culture_if->introduce = jp_introduce;
	culture_if->get_name = jp_name;
	culture_if->answer = jp_answer;
}

int Japan_new() {
	struct flyweight_init_s method={
		jp_constructor,
		NULL,
		NULL
	};

	int id = flyweight_register_class(sizeof(struct culture_if), 0, &method);
	return id;
}

