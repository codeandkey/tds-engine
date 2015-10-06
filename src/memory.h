#pragma once

/* TDS will not track memory. That is slow (and takes up memory. */
/* It will however keep count of allocated and deallocated memory (in terms of counts) */

/* A debug flag can be enabled, memcheck will report functions which allocated memory which was never freed. */

void* tds_malloc_dbg(const char* func, int size);
void tds_free_dbg(const char* func, void* ptr);
void* tds_realloc_dbg(void* ptr, int size);
int tds_get_blocks_dbg(void);
void tds_memcheck_dbg(void);

void* tds_malloc_rel(int size);
void tds_free_rel(void* ptr);
void* tds_realloc_rel(void* ptr, int size);
int tds_get_blocks_rel(void);
void tds_memcheck_rel(void);

#ifdef TDS_MEMORY_DEBUG

#define tds_malloc(x) tds_malloc_dbg(__func__, x)
#define tds_free(x) tds_free_dbg(__func__, x)
#define tds_realloc(x, y) tds_realloc_dbg(x, y)
#define tds_get_blocks tds_get_blocks_dbg
#define tds_memcheck tds_memcheck_dbg

#else

#define tds_malloc(x) tds_malloc_rel(x);
#define tds_free(x) tds_free_rel(x);
#define tds_realloc(x, y) tds_realloc_rel(x, y);
#define tds_get_blocks tds_get_blocks_rel
#define tds_memcheck tds_memcheck_rel

#endif
