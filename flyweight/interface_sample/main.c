#include <stdio.h>
#include "culture.h"
#include "members.h"
#include "flyweight.h"

int main() {
	struct {
		int id;
		char * question;
	} questions[] =
	{
		{FOOD, FOOD_QUESTION},
		{HAPPY, HAPPY_QUESTION},
		{CONPUTER, CONPUTER_QUESTION},
	};

	int max_members[10];
	int i=0;
	int member_num = get_any_people(max_members);

	printf("[インタビューアー] 皆さんこんにちは。本日はよろしくお願いします。まずは軽く自己紹介をお願いします。\n");

	//interface
	struct culture_if *cultureif;

	for( i = 0 ; i < member_num; i ++ ) {
		printf("  [%d番目の方の自己紹介]\n", i+1);
       		cultureif = (struct culture_if *) flyweight_get(max_members[i]);
		printf("\t%s\n", cultureif->introduce() );
	}

	printf("\n");
	printf("[インタビューアー] ありがとうございます。それではこれから順次質問をしていきたいと思います。\n気軽にお答えください。\n\n");

	int j=0;
	for ( i = 0 ; i < sizeof(questions)/sizeof(questions[0]); i ++ ) {
		printf("Q %d: %s\n", i+1, questions[i].question);
		for( j = 0 ; j < member_num ; j ++ ) {
			cultureif = (struct culture_if *) flyweight_get(max_members[j]);
			printf("  [A:%sさん]\n", cultureif->get_name());
			printf("\t%s\n", cultureif->answer(i));
		}
		printf("\n");
	}

	printf("[インタビューアー]以上で質問は終わりです。皆さんありがとうございました！\n");
	flyweight_exit();
	return 0;
}
