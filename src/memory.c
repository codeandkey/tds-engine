#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct _tds_mem_block {
	const char* func;
	void* ptr;
	int size;
	struct _tds_mem_block* next, *prev;
};

static struct _tds_mem_block *_tds_mem_dbg_head = NULL;

static int tds_mem_blocks = 0, tds_mem_blocks_dbg = 0;
static unsigned long tds_mem_bytes = 0;

void* tds_malloc_rel(int size) {
	void* output = malloc(size);

	if (!output) {
		tds_logf(TDS_LOG_CRITICAL, "Memory allocation failure for block of size %d.\n");
		return NULL;
	}

	tds_mem_blocks++;
	tds_mem_bytes += size;

	memset(output, 0, size);

	return output;
}

void tds_free_rel(void* ptr) {
	if (!ptr) {
		return;
	}

	tds_mem_blocks--;
	free(ptr);
}

void* tds_realloc_rel(void* ptr, int size) {
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

int tds_get_blocks_rel(void) {
	return tds_mem_blocks;
}

void tds_memcheck_rel(void) {
	tds_logf(TDS_LOG_MESSAGE, "Starting release memcheck.\n");

	if (tds_mem_blocks > 0) {
		tds_logf(TDS_LOG_WARNING, "%d blocks unfreed.\n", tds_mem_blocks);
	} else {
		tds_logf(TDS_LOG_MESSAGE, "All blocks freed.\n");
	}
}

void* tds_malloc_dbg(const char* func, int size) {
	/* Debug : we operate on a linked list. */
	struct _tds_mem_block* blk = malloc(sizeof *blk);

	if (!blk) {
		tds_logf(TDS_LOG_CRITICAL, "Memory allocation failure for block of size %d. (Debug list element)\n", sizeof *blk);
		return NULL;
	}

	blk->ptr = malloc(size);

	if (!blk->ptr) {
		tds_logf(TDS_LOG_CRITICAL, "Memory allocation failure for block of size %d.\n", size);
		return NULL;
	}

	memset(blk->ptr, 0, size);

	tds_mem_blocks_dbg++;
	tds_mem_bytes += size;

	blk->size = size;
	blk->next = _tds_mem_dbg_head;
	blk->prev = NULL;
	blk->func = func;

	if (_tds_mem_dbg_head) {
		_tds_mem_dbg_head->prev = blk;
	}

	_tds_mem_dbg_head = blk;

	return blk->ptr;
}

void tds_free_dbg(const char* func, void* ptr) {
	struct _tds_mem_block* blk = _tds_mem_dbg_head;

	while (blk) {
		if (blk->ptr == ptr) {
			free(ptr);

			tds_mem_bytes -= blk->size;
			tds_mem_blocks_dbg--;

			if (blk->next) {
				blk->next->prev = blk->prev;
			}

			if (blk->prev) {
				blk->prev->next = blk->next;
			}

			if (_tds_mem_dbg_head == blk) {
				_tds_mem_dbg_head = blk->next;
			}

			free(blk);
			return;
		}

		blk = blk->next;
	}

	tds_logf(TDS_LOG_WARNING, "Pointer %p not found in list! [called from %s]\n", ptr, func);
}

void* tds_realloc_dbg(const char* func, void* ptr, int size) {
	struct _tds_mem_block* blk = _tds_mem_dbg_head;

	while (blk) {
		if (blk->ptr == ptr) {
			blk->ptr = realloc(ptr, size);

			if (!blk->ptr) {
				tds_logf(TDS_LOG_CRITICAL, "Reallocation failed for %p with new block size %d.\n", ptr, size);
				return NULL;
			}

			blk->size = size;
			tds_mem_bytes += blk->size;

			return blk->ptr;
		}

		blk = blk->next;
	}

	tds_logf(TDS_LOG_WARNING, "Pointer %p not found in list! (called from %s)\n", ptr, func);
	return tds_malloc_dbg(func, size);
}

int tds_get_blocks_dbg(void) {
	return tds_mem_blocks_dbg;
}

void tds_memcheck_dbg(void) {
	tds_logf(TDS_LOG_MESSAGE, "Starting debug memcheck. Blocks = %d [rel blocks = %d]\n", tds_mem_blocks_dbg, tds_mem_blocks);

	struct _tds_mem_block* blk = _tds_mem_dbg_head, *tmp = NULL;

	while (blk) {
		tds_logf(TDS_LOG_WARNING, "Unfreed block : %p:[%d] - allocated in %s\n", blk->ptr, blk->size, blk->func);

		free(blk->ptr);
		tmp = blk->next;
		free(blk);
		blk = tmp;
	}
}
