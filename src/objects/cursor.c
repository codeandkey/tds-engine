#include "cursor.h"
#include "../engine.h"
#include "../log.h"
#include "../util.h"

struct tds_object_type tds_obj_cursor_type = {
	"cursor",
	"cursor",
	sizeof(struct tds_obj_cursor_data),
	tds_obj_cursor_init,
	tds_obj_cursor_destroy,
	tds_obj_cursor_update,
	tds_obj_cursor_draw,
	tds_obj_cursor_msg,
};

void tds_obj_cursor_init(struct tds_object* ptr) {
	struct tds_obj_cursor_data* data = (struct tds_obj_cursor_data*) ptr->object_data;

	ptr->current_frame = 0;
	ptr->anim_running = 0;
	ptr->layer = 100;

	data->color_step = 0;
}

void tds_obj_cursor_destroy(struct tds_object* ptr) {
}

void tds_obj_cursor_update(struct tds_object* ptr) {
	struct tds_obj_cursor_data* data = (struct tds_obj_cursor_data*) ptr->object_data;

	ptr->angle += 0.01f;

	if ((data->color_step += 64) >= 0x00FFFFFF) {
		data->color_step = 0;
	}

	tds_util_hsv_to_rgb((fmod(ptr->angle, 3.141f * 2.0f) / (3.141f * 2.0f)) * 360.0f, 1.0f, 0.5f, &ptr->r, &ptr->g, &ptr->b);
}

void tds_obj_cursor_draw(struct tds_object* ptr) {
	const float acceleration = 1.0f / 500.0f;

	ptr->x = tds_engine_global->input_handle->mx * acceleration;
	ptr->y = -tds_engine_global->input_handle->my * acceleration;
}

void tds_obj_cursor_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {

}
