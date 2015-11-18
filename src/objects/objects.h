#pragma once
#include "../object_type_cache.h"
#include "../object.h"

#define TDS_EDITOR_MODE_DISABLED 0
#define TDS_EDITOR_MODE_OBJECTS 1
#define TDS_EDITOR_MODE_WORLD 2

/* The built in objects are designed exclusively for the editor. */

#include "editor_cursor.h"
#include "editor_selector.h"
#include "editor_world_cursor.h"

void tds_load_editor_objects(struct tds_object_type_cache* otc_handle); /* Special function, called to load editor object types into cache. */
/* As of right now, the engine will call it shortly after subsystem init. */

void tds_create_editor_objects(void);
void tds_create_world_editor_objects(void);
void tds_destroy_editor_objects(void);

int tds_editor_get_mode(void);
void tds_editor_add_selector(struct tds_object* ptr);
