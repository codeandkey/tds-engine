#include "bg.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>

struct tds_bg* tds_bg_create(void) {
	struct tds_bg* output = tds_malloc(sizeof *output);

	for (int i = 0; i < TDS_BG_LAYERS; ++i) {
		output->layers[i] = NULL;
	}

	return output;
}

void tds_bg_free(struct tds_bg* ptr) {
	tds_bg_flush(ptr);
	tds_free(ptr);
}

void tds_bg_flush(struct tds_bg* ptr) {
	for (int i = 0; i < TDS_BG_LAYERS; ++i) {
		tds_bg_flush_layer(ptr, i);
	}
}

void tds_bg_push(struct tds_bg* ptr, struct tds_texture* tex, int layer) {
	if (layer < 0 || layer >= TDS_BG_LAYERS) {
		tds_logf(TDS_LOG_WARNING, "Layer out of range!\n");
		return;
	}

	struct tds_bg_entry* new = tds_malloc(sizeof *new);

	new->tex = tex;
	new->next = ptr->layers[layer];
	ptr->layers[layer] = new;
}

void tds_bg_flush_layer(struct tds_bg* ptr, int layer) {
	if (layer < 0 || layer >= TDS_BG_LAYERS) {
		tds_logf(TDS_LOG_WARNING, "Layer out of range!\n");
		return;
	}

	struct tds_bg_entry* tmp = NULL;

	while (ptr->layers[layer]) {
		tmp = ptr->layers[layer]->next;
		tds_free(ptr->layers[layer]);
		ptr->layers[layer] = tmp;
	}
}
