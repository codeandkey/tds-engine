#include "texture.h"
#include "memory.h"

struct tds_texture* tds_texture_create(const char* filename) {
	struct tds_texture* output = tds_malloc(sizeof(struct tds_texture));

	return output;
}

void tds_texture_free(struct tds_texture* ptr) {
	tds_free(ptr);
}
