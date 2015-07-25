#include "system.h"

#include "../input_map.h"
#include "../key_map.h"
#include "../engine.h"

#include "../game/game_input.h"

struct tds_object_type tds_obj_system_type = {
	"system",
	0,
	0,

	tds_obj_system_init,
	tds_obj_system_destroy,
	tds_obj_system_update,
	tds_obj_system_draw,
	tds_obj_system_msg
};

void tds_obj_system_init(struct tds_object* ptr) {
}

void tds_obj_system_update(struct tds_object* ptr) {
	if (tds_input_map_get_key_pressed(tds_engine_global->input_map_handle, tds_engine_global->key_map_handle->entry_buffer[TDS_GAME_INPUT_QUIT].key, 0)) {
		tds_engine_terminate(tds_engine_global);
	}
}

void tds_obj_system_draw(struct tds_object* ptr) {
}

void tds_obj_system_destroy(struct tds_object* ptr) {
}

void tds_obj_system_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
}
