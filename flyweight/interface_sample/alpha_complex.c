#include <stdio.h>
#include <string.h>
#include "culture.h"
#include "members.h"
#include "flyweight.h"

static char * wrap_answer="はい、アルファ・コンプレックスのすべては完璧で幸福です。";

static char * infra_red_answer(int id) {
	static char * answer[]={
		"心が爆発するほど素晴らしい食べ物ばかりです！", 
		"はい、幸福です！コンピューター様！", 
		"どうなんでしょう、私にはわかりません。", 
	};
	return answer[id];
}

static char *uv_namecheck(char *name) {
	return "インフラレッド";
}

#define INFRA_NAME "ジョージ"
static char * infra_red_introduce() {
	static char introduce[256]={0};
	snprintf(introduce, sizeof(introduce),"アルファ・コンプレックスの%sです。", uv_namecheck(INFRA_NAME));
	return introduce;
}

static char * infra_red_name() {
	return uv_namecheck(INFRA_NAME);
}

static char * infra_red_answer_uv_check(int id) {
	char * infra_answer = infra_red_answer(id);
	if(id == CONPUTER) {
		if(strstr( infra_answer , "完璧") == NULL ) {
#ifdef UV_COMMENT
			printf("__UV: ZAP!ZAP!インフラレッド!__\n");
#endif
			infra_answer = wrap_answer;
		}
	}
	return infra_answer;
}

static void infra_red_constructor(void *src) {
	struct culture_if *culture_if = (struct culture_if *)src;
	culture_if->introduce = infra_red_introduce;
	culture_if->get_name = infra_red_name;
	culture_if->answer = infra_red_answer_uv_check;
}

static char * uv_answer(int id) {
	static char * answer[]={
		"あなたのクリアランスには公開されていません。", 
		"幸福は義務です、市民。あなたは幸福ですか？", 
		"アルファ・コンプレックスのコンピューターは完璧です。今まで10名のインフラレッドが試しましたがバグを訴えたものは一つもいませんでした。", 
	};

	return answer[id];
}

static char * uv_introduce() {
	return "UV(ウルトラヴァイオレット)です。";
}

static char * uv_name() {
#ifdef UV_COMMENT
	printf("%s---あなたのクリアランスには公開されていません---\n", __FUNCTION__);
#endif
	return "UV";
}

static void uv_constructor(void *src) {
	struct culture_if *culture_if = (struct culture_if *)src;
	culture_if->introduce = uv_introduce;
	culture_if->get_name = uv_name;
	culture_if->answer = uv_answer;
}

int alpha_complex_new(int clearance) {
	struct flyweight_init_s method={
		NULL,
		NULL,
		NULL
	};

	if(clearance == ULTRA_VIOLET) {
		method.constructor = uv_constructor;
	} else {
		method.constructor = infra_red_constructor;
	}
	int id = flyweight_register_class(sizeof(struct culture_if), 0, &method);
	return id;
}


