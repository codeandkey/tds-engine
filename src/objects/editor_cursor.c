#include "editor_cursor.h"

struct tds_object_type obj_editor_cursor_type = {
	.type_name = "editor_cursor",
	.default_sprite = "spr_editor_cursor",
	.default_params = 0,
	.default_params_size = 0,
	.data_size = sizeof(struct obj_editor_cursor_data),
	.func_init = obj_editor_cursor_init,
	.func_destroy = obj_editor_cursor_destroy,
	.func_update = obj_editor_cursor_update,
	.func_draw = obj_editor_cursor_draw,
	.func_msg = (void*) 0,
	.func_import = (void*) 0,
	.func_export = (void*) 0
};

void obj_editor_cursor_init(struct tds_object* ptr) {
}

void obj_editor_cursor_destroy(struct tds_object* ptr) {
}

void obj_editor_cursor_update(struct tds_object* ptr) {
}

void obj_editor_cursor_draw(struct tds_object* ptr) {
}
