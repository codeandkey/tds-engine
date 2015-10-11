#include "editor_selector.h"
#include "../log.h"

struct tds_object_type obj_editor_selector_type = {
	.type_name = "obj_editor_selector",
	.default_sprite = "spr_editor_selector",
	.default_params = 0,
	.default_params_size = 0,
	.data_size = sizeof(struct obj_editor_selector_data),
	.func_init = obj_editor_selector_init,
	.func_destroy = obj_editor_selector_destroy,
	.func_update = obj_editor_selector_update,
	.func_draw = obj_editor_selector_draw,
	.func_msg = obj_editor_selector_msg,
	.func_import = (void*) 0,
	.func_export = (void*) 0,
	.save = 0
};

void obj_editor_selector_init(struct tds_object* ptr) {
}

void obj_editor_selector_destroy(struct tds_object* ptr) {
}

void obj_editor_selector_update(struct tds_object* ptr) {
}

void obj_editor_selector_draw(struct tds_object* ptr) {
	struct obj_editor_selector_data* data = ptr->object_data;

	ptr->visible = (data->target != 0);

	data->target->x = ptr->x;
	data->target->y = ptr->y;
	data->target->angle = ptr->angle;

	ptr->cbox_width = 1.0f;
	ptr->cbox_height = 1.0f;
}

void obj_editor_selector_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
	struct obj_editor_selector_data* data = ptr->object_data;

	tds_logf(TDS_LOG_DEBUG, "Selector %p received %d, %p from %p\n", ptr, msg, param, sender);

	switch(msg) {
	case OBJ_EDITOR_SELECTOR_MSG_TARGET:
		data->target = param;
		ptr->x = data->target->x;
		ptr->y = data->target->y;
		ptr->angle = data->target->angle;
		break;
	}
}
