#include "player_arms.h"
#include "../engine.h"
#include "../log.h"
#include "../util.h"
#include "../collision.h"

#include "../game/game_input.h"
#include "../game/game_msg.h"

struct tds_object_type tds_obj_player_arms_type = {
	"player_arms",
	"player_arms_katana",
	sizeof(struct tds_obj_player_arms_data),
	tds_obj_player_arms_init,
	tds_obj_player_arms_destroy,
	tds_obj_player_arms_update,
	tds_obj_player_arms_draw,
	tds_obj_player_arms_msg,
};

void tds_obj_player_arms_init(struct tds_object* ptr) {
	struct tds_obj_player_arms_data* data = (struct tds_obj_player_arms_data*) ptr->object_data;

	ptr->current_frame = 0;
	ptr->anim_running = 0;
	data->flag_swing = 0;
	ptr->visible = 0;
	ptr->anim_oneshot = 1;
	ptr->layer = 9;

	ptr->cbox_width *= 0.75f;
	ptr->cbox_height *= 0.75f;
}

void tds_obj_player_arms_destroy(struct tds_object* ptr) {
}

void tds_obj_player_arms_update(struct tds_object* ptr) {
	struct tds_obj_player_arms_data* data = (struct tds_obj_player_arms_data*) ptr->object_data;

	if (tds_input_map_get_mouse_button_pressed(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_ATTACK), 0) && !data->flag_swing) {
		data->flag_swing = 1;
		ptr->visible = 1;
		tds_object_anim_start(ptr);
	}

	if (data->flag_swing == 1) {
		struct tds_engine_object_list list = tds_engine_get_object_list_by_type(tds_engine_global, "enemy_basic");

		for (int i = 0; i < list.size; ++i) {
			if (!tds_collision_get_overlap(ptr, list.buffer[i])) {
				continue;
			}

			float obj_angle = atan2f(list.buffer[i]->y - ptr->y, list.buffer[i]->x - ptr->x);
			float swing_angle;
			float swing_threshold = 3.141f / 6.0f;

			switch(ptr->current_frame) {
			case 0:
				swing_angle = -3.141f / 2.0f;
				break;
			case 1:
				swing_angle = -3.141f / 4.0f;
				break;
			case 2:
				swing_angle = -3.141f / 6.0f;
				break;
			case 3:
				swing_angle = 3.141f / 5.0f;
				break;
			case 4:
				swing_angle = 3.141f / 3.0f;
				break;
			}

			tds_logf(TDS_LOG_DEBUG, "obj_angle = %f, swing_handle + ptr->angle = %f\n", obj_angle, swing_angle + ptr->angle);

			if (fmod(fabs(obj_angle - (ptr->angle + swing_angle)), 2.0f * 3.141f) <= swing_threshold) {
				/* We will hit this object. */

				list.buffer[i]->func_msg(list.buffer[i], ptr, TDS_GAME_MSG_KILL_MELEE, NULL);
			}
		}
	}
}

void tds_obj_player_arms_draw(struct tds_object* ptr) {
	struct tds_object* obj_player = tds_engine_get_object_by_type(tds_engine_global, "player");
	struct tds_obj_player_arms_data* data = (struct tds_obj_player_arms_data*) ptr->object_data;

	ptr->x = obj_player->x;
	ptr->y = obj_player->y;
	ptr->angle = obj_player->angle;

	tds_object_anim_update(ptr);

	if (tds_object_anim_oneshot_finished(ptr) && data->flag_swing) {
		data->flag_swing = 0;
		ptr->visible = 0;
		ptr->anim_running = 0;
	}
}

void tds_obj_player_arms_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
}
