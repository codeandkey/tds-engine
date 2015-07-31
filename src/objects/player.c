#include "player.h"

#include "../engine.h"

#include "../input.h"
#include "../log.h"
#include "../game/game_input.h"

struct tds_object_type tds_obj_player_type = {
	"player",
	"player",
	sizeof(struct tds_obj_player_data),
	tds_obj_player_init,
	tds_obj_player_destroy,
	tds_obj_player_update,
	tds_obj_player_draw,
	tds_obj_player_msg
};

void tds_obj_player_init(struct tds_object* ptr) {
	struct tds_obj_player_data* data = (struct tds_obj_player_data*) ptr->object_data;

	data->xspeed = data->yspeed = 0.0f;
}

void tds_obj_player_destroy(struct tds_object* ptr) {
}

void tds_obj_player_update(struct tds_object* ptr) {
	struct tds_input* input = tds_engine_global->input_handle;
	struct tds_object* obj_cursor = tds_engine_get_object_by_type(tds_engine_global, "cursor");
	struct tds_obj_player_data* data = (struct tds_obj_player_data*) ptr->object_data;

	ptr->angle = atan2f(obj_cursor->y - ptr->y, obj_cursor->x - ptr->x);

	int flag_moving_h = 0, flag_moving_v = 0;

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_LEFT), 0)) {
		data->xspeed = fmax(data->xspeed - TDS_OBJ_PLAYER_ACCEL, -TDS_OBJ_PLAYER_MAX_SPEED);
		flag_moving_h = 1;
	}

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_DOWN), 0)) {
		data->yspeed = fmax(data->yspeed - TDS_OBJ_PLAYER_ACCEL, -TDS_OBJ_PLAYER_MAX_SPEED);
		flag_moving_v = 1;
	}

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_RIGHT), 0)) {
		data->xspeed = fmin(data->xspeed + TDS_OBJ_PLAYER_ACCEL, TDS_OBJ_PLAYER_MAX_SPEED);
		flag_moving_h = 1;
	}

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_UP), 0)) {
		data->yspeed = fmin(data->yspeed + TDS_OBJ_PLAYER_ACCEL, TDS_OBJ_PLAYER_MAX_SPEED);
		flag_moving_v = 1;
	}

	if (!flag_moving_h) {
		data->xspeed /= TDS_OBJ_PLAYER_DECEL;
	}

	if (!flag_moving_v) {
		data->yspeed /= TDS_OBJ_PLAYER_DECEL;
	}

	if (flag_moving_h || flag_moving_v) {
		ptr->anim_running = 1;
	} else {
		ptr->current_frame = 0;
		ptr->anim_running = 0;
	}

	ptr->x += data->xspeed;
	ptr->y += data->yspeed;
}

void tds_obj_player_draw(struct tds_object* ptr) {
	tds_object_anim_update(ptr);
}

void tds_obj_player_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* param) {
}
