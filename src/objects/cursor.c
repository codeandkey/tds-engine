#include "cursor.h"
#include "../engine.h"

struct tds_object_type tds_obj_cursor_type = {
	"cursor",
	"cursor",
	0,
	tds_obj_cursor_init,
	tds_obj_cursor_destroy,
	tds_obj_cursor_update,
	tds_obj_cursor_draw,
	tds_obj_cursor_msg,
};

void tds_obj_cursor_init(struct tds_object* ptr) {
}

void tds_obj_cursor_destroy(struct tds_object* ptr) {
}

void tds_obj_cursor_update(struct tds_object* ptr) {
	const float acceleration = 1.0f / 1000.0f;

	ptr->x += acceleration * (tds_engine_global->input_handle->mx_last - tds_engine_global->input_handle->mx);
	ptr->y += acceleration * (tds_engine_global->input_handle->my - tds_engine_global->input_handle->my_last);

	ptr->x = tds_engine_global->input_handle->mx * 0.001f;
	ptr->y = -tds_engine_global->input_handle->my * 0.001f;
}

void tds_obj_cursor_draw(struct tds_object* ptr) {
}

void tds_obj_cursor_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {

}
