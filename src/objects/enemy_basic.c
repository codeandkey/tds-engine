#include "enemy_basic.h"
#include "../engine.h"
#include "../log.h"

#include "../game/game_msg.h"

struct tds_object_type tds_obj_enemy_basic_type = {
	"enemy_basic",
	"enemy_basic",
	sizeof(struct tds_obj_enemy_basic_data),
	tds_obj_enemy_basic_init,
	tds_obj_enemy_basic_destroy,
	tds_obj_enemy_basic_update,
	tds_obj_enemy_basic_draw,
	tds_obj_enemy_basic_msg,
};

void tds_obj_enemy_basic_init(struct tds_object* ptr) {
	struct tds_obj_enemy_basic_data* data = (struct tds_obj_enemy_basic_data*) ptr->object_data;

	ptr->layer = 8;
}

void tds_obj_enemy_basic_destroy(struct tds_object* ptr) {
}

void tds_obj_enemy_basic_update(struct tds_object* ptr) {
	struct tds_obj_enemy_basic_data* data = (struct tds_obj_enemy_basic_data*) ptr->object_data;
}

void tds_obj_enemy_basic_draw(struct tds_object* ptr) {
	tds_object_anim_update(ptr);
}

void tds_obj_enemy_basic_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
	switch(msg) {
	case TDS_GAME_MSG_KILL_MELEE:
		tds_object_free(ptr);
		break;
	}
}
