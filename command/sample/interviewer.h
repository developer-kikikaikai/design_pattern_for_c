#ifndef INTERVIEWER_H
#define INTERVIEWER_H
#include "answer.h"

struct interviewer_t {
	void (*interview)(void);
};
typedef struct interviewer_t interviewer_t, *Interviewer;

Interviewer interviewer_red_new(Answer answer);
Interviewer interviewer_blue_new(Answer answer);

#endif
