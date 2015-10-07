#pragma once
#include "../object_type_cache.h"

/* The built in objects are designed exclusively for the editor. */

#include "editor_cursor.h"
#include "editor_selector.h"

void tds_load_editor_objects(struct tds_object_type_cache* otc_handle); /* Special function, called to load editor object types into cache. */
/* As of right now, the engine will call it shortly after subsystem init. */

void tds_create_editor_objects(void);
void tds_destroy_editor_objects(void);
