#include "camera.h"
#include "memory.h"
#include "log.h"
#include "engine.h"

#include <math.h>

struct tds_camera* tds_camera_create(struct tds_display* disp) {
	struct tds_camera* output = tds_malloc(sizeof(struct tds_camera));

	output->pos = tds_bcp_zero;
	output->hidden_scale = 1.0f;

	tds_camera_set(output, tds_bcp_zero, (tds_bc) 320);

	return output;
}

void tds_camera_free(struct tds_camera* ptr) {
	tds_free(ptr);
}

void tds_camera_set(struct tds_camera* ptr, tds_bcp pos, tds_bc height) {
	/* to make the game pretty, we try snapping the height so the screen evenly divides into in-game pixels */
	/*
	 * if the screen is evenly divided, height | disp->desc.height, so we divide until we get close
	 */

	struct tds_display* disp = tds_engine_global->display_handle;
	unsigned int pixel_height = disp->desc.height / height;
	if (!pixel_height) pixel_height = 1; /* cap camera height at single pixel accuracy */
	height = disp->desc.height / pixel_height; /* round down */

	float disp_width = disp->desc.width, disp_height = disp->desc.height;

	ptr->dim.y = height;
	ptr->dim.x = (ptr->dim.y * disp_width) / disp_height;

	tds_camera_set_raw(ptr, pos, ptr->dim);
}

void tds_camera_set_raw(struct tds_camera* ptr, tds_bcp pos, tds_bcp dim) {
	ptr->pos = pos;
	ptr->dim = dim;

	mat4x4 translate, ortho;

	mat4x4_identity(ptr->mat_transform);
	mat4x4_ortho(ortho, 0.0f, dim.x / 16.0f, 0.0f, dim.y / 16.0f, 1.0f, -1.0f);
	mat4x4_translate(translate, -pos.x / 16.0f, -pos.y / 16.0f, 0.0f);

	// tds_logf(TDS_LOG_DEBUG, "Camera dimensions: ortho (L%f R%f T%f B%f) translate (%f %f)\n", 0.0f, dim.x / 16.0f, dim.y / 16.0f, 0.0f, -pos.x / 16.0f, -pos.y / 16.0f);

	mat4x4_mul(ptr->mat_transform, ortho, translate);
}
