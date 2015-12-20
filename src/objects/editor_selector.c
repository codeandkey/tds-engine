#include "editor_selector.h"
#include "../log.h"
#include "../msg.h"

struct tds_object_type obj_editor_selector_type = {
	.type_name = "obj_editor_selector",
	.default_sprite = "spr_editor_selector",
	.data_size = sizeof(struct obj_editor_selector_data),
	.func_init = obj_editor_selector_init,
	.func_destroy = obj_editor_selector_destroy,
	.func_update = obj_editor_selector_update,
	.func_draw = obj_editor_selector_draw,
	.func_msg = obj_editor_selector_msg,
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

	switch(msg) {
	case TDS_MSG_EDIT_TARGET:
		data->target = param;
		ptr->x = data->target->x;
		ptr->y = data->target->y;
		ptr->angle = data->target->angle;
		break;
	}
}
