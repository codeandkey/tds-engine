#include "enemy_basic.h"
#include "../engine.h"
#include "../log.h"
#include "../util.h"

#include "../game/game_msg.h"
#include "all.h"

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

static int _tds_obj_enemy_basic_los(struct tds_object* ptr, struct tds_object* player);

void tds_obj_enemy_basic_init(struct tds_object* ptr) {
	struct tds_obj_enemy_basic_data* data = (struct tds_obj_enemy_basic_data*) ptr->object_data;

	ptr->layer = 8;
	data->state = 0;
	data->state_draw = 0;
	data->xspeed = data->yspeed = 0.0f;
}

void tds_obj_enemy_basic_destroy(struct tds_object* ptr) {
}

void tds_obj_enemy_basic_update(struct tds_object* ptr) {
	struct tds_obj_enemy_basic_data* data = (struct tds_obj_enemy_basic_data*) ptr->object_data;
	struct tds_object* player = tds_engine_get_object_by_type(tds_engine_global, "player");

	int los = _tds_obj_enemy_basic_los(ptr, player);

	switch(data->state) {
	case TDS_OBJ_ENEMY_STATE_IDLE:
		if (los) {
			data->state = TDS_OBJ_ENEMY_STATE_ALARM;
		}
		break;
	case TDS_OBJ_ENEMY_STATE_ALARM:
		if (los && !data->state_draw) {
			data->state_draw = 1;
			tds_object_set_sprite(ptr, tds_sprite_cache_get(ptr->smgr, "enemy_basic_aim"));
			data->clock_fire = tds_clock_get_point();
		} else if (data->state_draw && !los) {
			tds_object_set_sprite(ptr, tds_sprite_cache_get(ptr->smgr, "enemy_basic"));
			data->state_draw = 0;
			data->state = TDS_OBJ_ENEMY_STATE_SEARCHING;
			data->clock_cooldown = tds_clock_get_point();
		}
		break;
	case TDS_OBJ_ENEMY_STATE_SEARCHING:
		if (los) {
			data->state = TDS_OBJ_ENEMY_STATE_ALARM;
		} else {
			if (tds_clock_get_ms(data->clock_cooldown) >= TDS_OBJ_ENEMY_COOLDOWN * 1000.0f) {
				data->state = TDS_OBJ_ENEMY_STATE_IDLE;
			}
		}
		break;
	}

	if (data->state_draw) {
		ptr->angle = atan2f(player->y - ptr->y, player->x - ptr->x);

		if (tds_clock_get_ms(data->clock_fire) >= 1000.0f * TDS_OBJ_ENEMY_FIRERATE) {
			data->clock_fire = tds_clock_get_point();
			tds_object_create(&tds_obj_bullet_type, ptr->hmgr, ptr->smgr, ptr->x, ptr->y, 0.0f, NULL)->angle = ptr->angle;
		}
	}
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

int _tds_obj_enemy_basic_los(struct tds_object* ptr, struct tds_object* player) {
	struct tds_obj_enemy_basic_data* data = (struct tds_obj_enemy_basic_data*) ptr->object_data;
	float x1 = ptr->x, y1 = ptr->y, x2 = player->x, y2 = player->y;

	float angle_to_player = atan2f(player->y - ptr->y, player->x - ptr->x);

	if (tds_util_angle_distance(angle_to_player, ptr->angle) > TDS_OBJ_ENEMY_FOV / 2.0f) {
		return 0;
	}

	struct tds_engine_object_list wall_list = tds_engine_get_object_list_by_type(tds_engine_global, "wall");

	for (int i = 0; i < wall_list.size; ++i) {
		if (tds_util_get_intersect(x1, y1, x2, y2, wall_list.buffer[i])) {
			return 0;
		}
	}

	return 1;
}
