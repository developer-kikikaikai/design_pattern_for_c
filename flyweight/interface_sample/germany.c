#include "members.h"
#include "flyweight.h"
#include "culture.h"

#define GNAME "シュバインシュタイガー"

static char * gm_introduce() {
	return "ドイツ人の"GNAME"です。";
}

static char * gm_name() {
	return GNAME;
}

static char * gm_answer(int id) {
	static char *answer[]={
		"ビールにポテト、旨いソーセージがあれば十分だね。", 
		"幸せだよ。自分の能力を発揮して働いてるし、素敵な家族がいる。最高だね", 
		"ここまでは平気だね、利用されるエラーケースもテストしたかい？",
	};
	return answer[id];
}

static void gm_constructor(void *src) {
	struct culture_if *culture_if = (struct culture_if *)src;
	culture_if->introduce = gm_introduce;
	culture_if->get_name = gm_name;
	culture_if->answer = gm_answer;
}

int Germany_new() {
	struct flyweight_init_s method={
		gm_constructor,
		NULL,
		NULL
	};

	int id = flyweight_register_class(sizeof(struct culture_if), 0, &method);
	return id;
}

