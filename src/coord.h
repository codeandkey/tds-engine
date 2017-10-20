#pragma once

#include <stdint.h>

/*
 * new coordinate system
 * the engine can migrate to prefer a newer ooordinate system -- the engine currently has 16x16 block images, so we can use a new coordinate system of two 32-bit integer coordinates
 * then, we can start performing collisions on a pixel-like world instead of discrete geometry, avoiding a lot of floating-point problems we had before
 * using 32-bit integers means we have 2^28 = 268435456^2 block map size.
 * we can keep the block map simple by using the 28 high bits to indicate blocks and the 4 small bits to indicate offsets.
 * low values indicate leftmost and bottommost directions.
 */

typedef uint32_t tds_bc; /* block coordinate */
typedef struct _tds_bcp { tds_bc x, y; } tds_bcp;

typedef struct _tds_vec2 { int32_t x, y; } tds_vec2;

/*
 * fortunately ths representation makes artihmetic super easy. any addition/substraction/etc will automatically
 * roll over between blocks smoothly by nature of the stored value.
 */

extern const tds_bc tds_bc_zero;
extern const tds_bcp tds_bcp_zero;
extern const tds_vec2 tds_vec2_zero;

tds_bcp tds_bcp_midpoint(tds_bcp a, tds_bcp b);
double tds_bcp_distance(tds_bcp a, tds_bcp b);

int tds_vec2_cmpi(tds_vec2 v, int x, int y);
