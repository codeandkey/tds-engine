#include "block_map.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_block_map* tds_block_map_create(void) {
	struct tds_block_map* output = tds_malloc(sizeof(struct tds_block_map));

	return output;
}

void tds_block_map_free(struct tds_block_map* ptr) {
	free(ptr);
}

void tds_block_map_add(struct tds_block_map* ptr, struct tds_texture* tex, int solid, uint8_t id) {
	ptr->buffer[id].texture = tex;
	ptr->buffer[id].solid = solid;
}

struct tds_block_type tds_block_map_get(struct tds_block_map* ptr, uint8_t id) {
	return ptr->buffer[id];
}
