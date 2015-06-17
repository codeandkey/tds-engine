#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>

static int tds_mem_blocks = 0;
static unsigned long tds_mem_bytes = 0;

void* tds_malloc(int size) {
	void* output = malloc(size);

	if (!output) {
		tds_logf(TDS_LOG_CRITICAL, "Memory allocation failure for block of size %d.\n");
		return NULL;
	}

	tds_mem_blocks++;
	tds_mem_bytes += size;

	return output;
}

void tds_free(void* ptr) {
	if (!ptr) {
		return;
	}

	tds_mem_blocks--;
	free(ptr);
}

void* tds_realloc(void* ptr, int size) {
	if (!ptr) {
		return tds_malloc(size);
	}

	tds_mem_bytes += size;
	void* output = realloc(ptr, size);

	if (!output) {
		tds_logf(TDS_LOG_CRITICAL, "Memory reallocation failure for block of size %d.\n", size);
		return NULL;
	}

	return output;
}

int tds_get_blocks(void) {
	return tds_mem_blocks;
}

void tds_memcheck(void) {
	if (tds_mem_blocks > 0) {
		tds_logf(TDS_LOG_WARNING, "%d blocks unfreed.\n", tds_mem_blocks);
	} else {
		tds_logf(TDS_LOG_WARNING, "All blocks freed.\n");
	}
}
