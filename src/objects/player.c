#include "player.h"

#include "../engine.h"
#include "../input.h"

struct tds_object_type tds_obj_player_type = {
	"player",
	"player",
	0,
	tds_obj_player_init,
	tds_obj_player_destroy,
	tds_obj_player_update,
	tds_obj_player_draw,
	tds_obj_player_msg
};

void tds_obj_player_init(struct tds_object* ptr) {
}

void tds_obj_player_destroy(struct tds_object* ptr) {
}

void tds_obj_player_update(struct tds_object* ptr) {
	struct tds_input* input = tds_engine_global->input_handle;
	struct tds_object* obj_cursor = tds_engine_get_object_by_type(tds_engine_global, "cursor");

	ptr->angle = atan2f(obj_cursor->y - ptr->y, obj_cursor->x - ptr->x);
}

void tds_obj_player_draw(struct tds_object* ptr) {
}

void tds_obj_player_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* param) {
}
