#pragma once

/* TDS will not track memory. That is slow (and takes up memory. */
/* It will however keep count of allocated and deallocated memory (in terms of counts) */

void* tds_malloc(int size);
void tds_free(void* ptr);
void* tds_realloc(void* ptr, int size);

int tds_get_blocks(void);
void tds_memcheck(void);
