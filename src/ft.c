#include "ft.h"
#include "memory.h"
#include "log.h"

struct tds_ft* tds_ft_create(void) {
	struct tds_ft* output = tds_malloc(sizeof *output);

	if (FT_Init_FreeType(&output->ctx)) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to initialize FreeType2 context.\n");
		return NULL;
	}

	return output;
}

void tds_ft_free(struct tds_ft* ptr) {
	FT_Done_FreeType(ptr->ctx);
	tds_free(ptr);
}
