#include "tds.h"

int main(int argc, char** argv) {
	struct tds_display_desc desc = {0};

	desc.width = 640;
	desc.height = 480;
	desc.fs = 0;
	desc.vsync = 0;

	tds_script_create("test.lua");

	struct tds_display* disp = tds_display_create(desc);

	while (!tds_display_get_close(disp)) {
		tds_display_update(disp);
		tds_display_swap(disp);
	}

	tds_display_free(disp);
	tds_memcheck();

	return 0;
}
