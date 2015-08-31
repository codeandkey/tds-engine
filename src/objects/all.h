#pragma once

#include "../object.h"

#include "system.h"
#include "player.h"
#include "player_legs.h"
#include "player_arms.h"
#include "cursor.h"
#include "enemy_basic.h"
#include "bullet.h"

#include "wall.h"

struct tds_object_type** tds_object_type_get_list(void);
int tds_object_type_get_count(void);
struct tds_object_type* tds_object_type_get_by_name(const char* type_name);

/* We will encapulate it with functions to make it smaller in memory (avoid multiple definitions + static decl) */
