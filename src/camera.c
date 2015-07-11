#include "camera.h"
#include "memory.h"

struct tds_camera* tds_camera_create(void) {
	struct tds_camera* output = tds_malloc(sizeof(struct tds_camera));

	return output;
}

void tds_camera_free(struct tds_camera* ptr) {
	tds_free(ptr);
}
