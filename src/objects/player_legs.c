#include "player_legs.h"
#include "player.h"

#include "../engine.h"

struct tds_object_type tds_obj_player_legs_type = {
	"player_legs",
	"player_legs",
	0,
	tds_obj_player_legs_init,
	tds_obj_player_legs_destroy,
	tds_obj_player_legs_update,
	tds_obj_player_legs_draw,
	tds_obj_player_legs_msg
};

void tds_obj_player_legs_init(struct tds_object* ptr) {
	ptr->layer = 8;
}

void tds_obj_player_legs_destroy(struct tds_object* ptr) {
}

void tds_obj_player_legs_update(struct tds_object* ptr) {
	struct tds_object* obj_player = tds_engine_get_object_by_type(tds_engine_global, "player");
	struct tds_obj_player_data* data = (struct tds_obj_player_data*) obj_player->object_data;

	ptr->x = obj_player->x;
	ptr->y = obj_player->y;

	if (data->flag_moving_h || data->flag_moving_v) {
		ptr->angle = atan2f(data->yspeed, data->xspeed);
		ptr->anim_running = 1;
	} else {
		ptr->anim_running = 0;
		ptr->current_frame = 0;
	}
}

void tds_obj_player_legs_draw(struct tds_object* ptr) {
	tds_object_anim_update(ptr);
}

void tds_obj_player_legs_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* param) {
}
