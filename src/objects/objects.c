#include "objects.h"
#include "../engine.h"
#include "../msg.h"

static int editor_mode = 0;
struct tds_object* editor_cursor = NULL;

void tds_load_editor_objects(struct tds_object_type_cache* otc_handle) {
	tds_object_type_cache_add(otc_handle, obj_editor_cursor_type.type_name, &obj_editor_cursor_type);
	tds_object_type_cache_add(otc_handle, obj_editor_selector_type.type_name, &obj_editor_selector_type);
}

void tds_create_editor_objects(void) {
	if (editor_mode) {
		return;
	}

	editor_mode = TDS_EDITOR_MODE_OBJECTS;
	editor_cursor = tds_object_create(&obj_editor_cursor_type, tds_engine_global->object_buffer, tds_engine_global->sc_handle, 0.0f, 0.0f, 0.0f, NULL);

	/* We want to create a selector for each object in the buffer with the save flag. */
	struct tds_handle_manager* hmgr = tds_engine_global->object_buffer;

	for (int i = 0; i < hmgr->max_index; ++i) {
		struct tds_object* obj = hmgr->buffer[i].data;

		if (!obj) {
			continue;
		}

		if (!obj->save) {
			continue;
		}

		tds_editor_add_selector(obj);
	}
}

void tds_destroy_editor_objects(void) {
	if (!editor_mode) {
		return;
	}

	editor_mode = 0;
	editor_cursor = NULL;

	tds_engine_destroy_objects(tds_engine_global, obj_editor_cursor_type.type_name);
	tds_engine_destroy_objects(tds_engine_global, obj_editor_selector_type.type_name);
}

int tds_editor_get_mode(void) {
	return editor_mode;
}

void tds_editor_add_selector(struct tds_object* ptr) {
	struct tds_object* new_obj = tds_object_create(&obj_editor_selector_type, tds_engine_global->object_buffer, tds_engine_global->sc_handle, ptr->x, ptr->y, 0.0f, NULL);
	tds_object_msg(new_obj, NULL, TDS_MSG_EDIT_TARGET, ptr);
}
