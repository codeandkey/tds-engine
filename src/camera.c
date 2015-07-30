#include "camera.h"
#include "memory.h"
#include "log.h"

#include <math.h>

struct tds_camera* tds_camera_create(struct tds_display* disp) {
	struct tds_camera* output = tds_malloc(sizeof(struct tds_camera));
	output->disp = disp;

	return output;
}

void tds_camera_free(struct tds_camera* ptr) {
	tds_free(ptr);
}

void tds_camera_set(struct tds_camera* ptr, float size, float x, float y) {
	float disp_width = ptr->disp->desc.width, disp_height = ptr->disp->desc.height;
	float camera_width = 0.0f, camera_height = 0.0f;

	if (disp_width < disp_height) {
		camera_width = size;
		camera_height = size * (disp_height / disp_width);
	} else {
		camera_height = size;
		camera_width = size * (disp_width / disp_height);
	}

	tds_logf(TDS_LOG_MESSAGE, "Setting camera ortho (DW %f, DH %f) : L %f, R %f, T %f, B %f\n", disp_width, disp_height, -camera_width / 2.0f, camera_width / 2.0f, camera_height / 2.0f, -camera_height / 2.0f);

	mat4x4 translate;

	mat4x4_identity(ptr->mat_transform);
	mat4x4_ortho(ptr->mat_transform, -camera_width / 2.0f, camera_width / 2.0f, -camera_height / 2.0f, camera_height / 2.0f, 1.0f, -1.0f);
	mat4x4_translate(translate, -x, -y, 0.0f);

	// mat4x4_mul(ptr->mat_transform, ptr->mat_transform, translate);
}
