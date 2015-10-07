#include "objects.h"

#include "../engine.h"

void tds_load_editor_objects(struct tds_object_type_cache* otc_handle) {
	tds_object_type_cache_add(otc_handle, obj_editor_cursor_type.type_name, &obj_editor_cursor_type);
	tds_object_type_cache_add(otc_handle, obj_editor_selector_type.type_name, &obj_editor_selector_type);
}

void tds_create_editor_objects(void) {
	tds_object_create(&obj_editor_cursor_type, tds_engine_global->object_buffer, tds_engine_global->sc_handle, 0.0f, 0.0f, 0.0f, NULL);

	/* We want to create a selector for each object in the buffer with the save flag. */
	struct tds_handle_manager* hmgr = tds_engine_global->object_buffer;

	for (int i = 0; i < hmgr->max_index; ++i) {
		struct tds_object* obj = hmgr->buffer[i].data, *n_obj = NULL;

		if (!obj) {
			continue;
		}

		if (!obj->save) {
			continue;
		}

		n_obj = tds_object_create(&obj_editor_selector_type, hmgr, tds_engine_global->sc_handle, obj->x, obj->y, 0.0f, NULL);
		tds_object_msg(n_obj, NULL, OBJ_EDITOR_SELECTOR_MSG_TARGET, obj);
	}
}

void tds_destroy_editor_objects(void) {
	tds_engine_destroy_objects(tds_engine_global, obj_editor_cursor_type.type_name);
	tds_engine_destroy_objects(tds_engine_global, obj_editor_selector_type.type_name);
}
