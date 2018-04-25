#ifndef CULTURE_
#define CULTURE_

//interface definition
struct culture_if {
	char *(*introduce)();
	char *(*get_name)();
	char *(*answer)(int id);
};

#define IF_DEFINITION \
	char *(*get_name)();\
	char *(*reaction)(int id);

//we can ask about those question about culture
enum question_id_definition{
	FOOD,
	HAPPY,
	CONPUTER,
};

#define FOOD_QUESTION "あなたの国の食文化を教えてください。"
#define HAPPY_QUESTION "あなたは幸福ですか？"
#define CONPUTER_QUESTION "このプログラム、ちゃんと動いてますかね？"

//culture list getter, return length and set list of class id in *culture_if_ids
int get_any_people(int *peoples_id);
#endif
