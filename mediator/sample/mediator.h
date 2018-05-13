#ifndef MEDIATOR_
#define MEDIATOR_

#include "colleague.h"

struct mediator_t;
typedef struct mediator_t *Mediator;
typedef enum mediator_status_e {
	GANGAN_IKOZE,
	INOCHI_DIJINI,
} mediator_status_e;

Mediator mediator_new(mediator_status_e status);
//colleagueの追加
void mediator_add_colleague(Mediator this, Colleague colleague);
//自身パーティーの攻撃
int mediator_ownturn(Mediator this);
//メンバーのピンチ報告
void mediator_member_warning(Mediator this);
//攻撃対象と人数
void mediator_damage(Mediator this, int value);
//メンバーチェック
int mediator_is_member(Mediator this);
//最後の解放
void mediator_free(Mediator this);
#endif
