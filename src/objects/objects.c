#include "objects.h"

#include "../engine.h"

void tds_load_editor_objects(struct tds_object_type_cache* otc_handle) {
	tds_object_type_cache_add(otc_handle, obj_editor_cursor_type.type_name, &obj_editor_cursor_type);
}

void tds_create_editor_objects(void) {
	tds_object_create(&obj_editor_cursor_type, tds_engine_global->object_buffer, tds_engine_global->sc_handle, 0.0f, 0.0f, 0.0f, NULL);
}

void tds_destroy_editor_objects(void) {
	tds_engine_destroy_objects(tds_engine_global, obj_editor_cursor_type.type_name);
}
