#include "camera.h"
#include "memory.h"
#include "log.h"

#include <math.h>

struct tds_camera* tds_camera_create(struct tds_display* disp) {
	struct tds_camera* output = tds_malloc(sizeof(struct tds_camera));
	output->disp = disp;
	output->x = output->y = output->z = output->angle = 0.0f;
	output->hidden_scale = 1.0f;

	tds_camera_set(output, 10.0f, 0.0f, 0.0f);

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

	tds_camera_set_raw(ptr, camera_width, camera_height, x, y);
}

void tds_camera_set_raw(struct tds_camera* ptr, float width, float height, float x, float y) {
	ptr->width = width;
	ptr->height = height;

	ptr->x = x;
	ptr->y = y;

	mat4x4 translate, ortho;

	mat4x4_identity(ptr->mat_transform);
	mat4x4_ortho(ortho, -ptr->width * ptr->hidden_scale / 2.0f, ptr->width * ptr->hidden_scale / 2.0f, -ptr->height * ptr->hidden_scale / 2.0f, ptr->height * ptr->hidden_scale / 2.0f, 1.0f, -1.0f);
	mat4x4_translate(translate, -x, -y, 0.0f);

	mat4x4_mul(ptr->mat_transform, ortho, translate);
}

void tds_camera_render_outline(struct tds_camera* ptr, struct tds_overlay* overlay) {
	tds_overlay_set_color(overlay, 0.0f, 1.0f, 0.0f, 1.0f);

	tds_overlay_render_line(overlay, ptr->x - ptr->width / 2.0f, ptr->y + ptr->height / 2.0f, ptr->x + ptr->width / 2.0f, ptr->y + ptr->height / 2.0f, 1.0f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
	tds_overlay_render_line(overlay, ptr->x - ptr->width / 2.0f, ptr->y - ptr->height / 2.0f, ptr->x + ptr->width / 2.0f, ptr->y - ptr->height / 2.0f, 1.0f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
	tds_overlay_render_line(overlay, ptr->x - ptr->width / 2.0f, ptr->y + ptr->height / 2.0f, ptr->x - ptr->width / 2.0f, ptr->y - ptr->height / 2.0f, 1.0f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
	tds_overlay_render_line(overlay, ptr->x + ptr->width / 2.0f, ptr->y + ptr->height / 2.0f, ptr->x + ptr->width / 2.0f, ptr->y - ptr->height / 2.0f, 1.0f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
}
