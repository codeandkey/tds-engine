#include "player.h"

#include "../engine.h"

#include "../input.h"
#include "../log.h"
#include "../game/game_input.h"
#include "../collision.h"

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

	ptr->layer = 10;
	ptr->cbox_width = 0.5f;
	ptr->cbox_height = 0.5f;

	data->flag_moving_h = data->flag_moving_v = 0;
	data->flag_swing = 0;
}

void tds_obj_player_destroy(struct tds_object* ptr) {
}

void tds_obj_player_update(struct tds_object* ptr) {
	struct tds_object* obj_cursor = tds_engine_get_object_by_type(tds_engine_global, "cursor");
	struct tds_obj_player_data* data = (struct tds_obj_player_data*) ptr->object_data;

	ptr->angle = atan2f(obj_cursor->y - ptr->y, obj_cursor->x - ptr->x);

	tds_camera_set(tds_engine_global->camera_handle, 10.0f, ptr->x, ptr->y);

	data->flag_moving_h = data->flag_moving_v = 0;

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_LEFT), 0)) {
		ptr->xspeed = fmax(ptr->xspeed - TDS_OBJ_PLAYER_ACCEL, -TDS_OBJ_PLAYER_MAX_SPEED);
		data->flag_moving_h = 1;
	}

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_DOWN), 0)) {
		ptr->yspeed = fmax(ptr->yspeed - TDS_OBJ_PLAYER_ACCEL, -TDS_OBJ_PLAYER_MAX_SPEED);
		data->flag_moving_v = 1;
	}

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_RIGHT), 0)) {
		ptr->xspeed = fmin(ptr->xspeed + TDS_OBJ_PLAYER_ACCEL, TDS_OBJ_PLAYER_MAX_SPEED);
		data->flag_moving_h = 1;
	}

	if (tds_input_map_get_key(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_MOVE_UP), 0)) {
		ptr->yspeed = fmin(ptr->yspeed + TDS_OBJ_PLAYER_ACCEL, TDS_OBJ_PLAYER_MAX_SPEED);
		data->flag_moving_v = 1;
	}

	if (!data->flag_moving_h) {
		ptr->xspeed /= TDS_OBJ_PLAYER_DECEL;
	}

	if (!data->flag_moving_v) {
		ptr->yspeed /= TDS_OBJ_PLAYER_DECEL;
	}

	if (!data->flag_swing) {
		if (data->flag_moving_h || data->flag_moving_v) {
			ptr->anim_running = 1;
		} else {
			ptr->current_frame = 0;
			ptr->anim_running = 0;
		}
	}

	/* Method of collision : test X collision, Y collision, move accordingly */
	struct tds_engine_object_list wall_list = tds_engine_get_object_list_by_type(tds_engine_global, "wall");
	int will_collide_x = 0, will_collide_y = 0, will_collide_xy = 0;

	float px = ptr->x, py = ptr->y;

	ptr->x = px + ptr->xspeed;
	ptr->y = py;

	for (int i = 0; i < wall_list.size; ++i) {
		will_collide_x |= tds_collision_get_overlap(ptr, wall_list.buffer[i]);
	}

	ptr->x = px;
	ptr->y = py + ptr->yspeed;

	for (int i = 0; i < wall_list.size; ++i) {
		will_collide_y |= tds_collision_get_overlap(ptr, wall_list.buffer[i]);
	}

	ptr->x = px + ptr->xspeed;
	ptr->y = py + ptr->yspeed;

	for (int i = 0; i < wall_list.size; ++i) {
		will_collide_xy |= tds_collision_get_overlap(ptr, wall_list.buffer[i]);
	}

	if (will_collide_xy && !will_collide_x && !will_collide_y) {
		will_collide_x = will_collide_y = 1;
	}

	if (will_collide_x) {
		ptr->xspeed = 0.0f;
	}

	if (will_collide_y) {
		ptr->yspeed = 0.0f;
	}

	ptr->x = px;
	ptr->y = py;

	/* Attack animation management */

	if (tds_input_map_get_mouse_button_pressed(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_ATTACK), 0) && !data->flag_swing) {
		data->flag_swing = 1;
		ptr->anim_oneshot = 1;
		tds_object_set_sprite(ptr, tds_sprite_cache_get(ptr->smgr, "player_body_swing"));
		tds_object_anim_start(ptr);
	}
}

void tds_obj_player_draw(struct tds_object* ptr) {
	struct tds_obj_player_data* data = (struct tds_obj_player_data*) ptr->object_data;

	if (tds_object_anim_oneshot_finished(ptr) && data->flag_swing) {
		ptr->anim_oneshot = 0;
		ptr->anim_running = 0;
		data->flag_swing = 0;
		tds_object_set_sprite(ptr, tds_sprite_cache_get(ptr->smgr, "player"));
	}
}

void tds_obj_player_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* param) {
}
