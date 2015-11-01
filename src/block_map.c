#include "block_map.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>

struct tds_block_map* tds_block_map_create(void) {
	struct tds_block_map* output = tds_malloc(sizeof(struct tds_block_map));

	output->block_type_list = NULL;

	return output;
}

void tds_block_map_free(struct tds_block_map* ptr) {
	struct tds_block_type* current = ptr->block_type_list, *tmp = NULL;

	while (current) {
		tmp = current->next;
		free(current);
		current = tmp;
	}

	free(ptr);
}

void tds_block_map_add(struct tds_block_map* ptr, struct tds_texture* tex, int solid, int id) {
	struct tds_block_type* new_block = tds_malloc(sizeof(struct tds_block_type));

	new_block->texture = tex;
	new_block->solid = solid;
	new_block->id = id;
}

struct tds_block_type* tds_block_map_get(struct tds_block_map* ptr, int id) {
	struct tds_block_type* current = ptr->block_type_list;

	while (current) {
		if (current->id == id) {
			return current;
		}

		current = current->next;
	}

	tds_logf(TDS_LOG_WARNING, "Could not locate block id %d in block map.\n", id);
	return NULL;
}
