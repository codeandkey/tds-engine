#include "bullet.h"
#include "../engine.h"
#include "../log.h"
#include "../util.h"
#include "../collision.h"

struct tds_object_type tds_obj_bullet_type = {
	"bullet",
	"bullet",
	sizeof(struct tds_obj_bullet_data),
	tds_obj_bullet_init,
	tds_obj_bullet_destroy,
	tds_obj_bullet_update,
	tds_obj_bullet_draw,
	tds_obj_bullet_msg,
};

void tds_obj_bullet_init(struct tds_object* ptr) {
	ptr->current_frame = 0;
	ptr->anim_running = 0;
	ptr->layer = 9;
	ptr->cbox_height /= 16.0f;
}

void tds_obj_bullet_destroy(struct tds_object* ptr) {
}

void tds_obj_bullet_update(struct tds_object* ptr) {
	struct tds_obj_bullet_data* data = (struct tds_obj_bullet_data*) ptr->object_data;

	data->xspeed = cosf(ptr->angle) * TDS_OBJ_BULLET_SPEED;
	data->yspeed = sinf(ptr->angle) * TDS_OBJ_BULLET_SPEED;

	ptr->x += data->xspeed;
	ptr->y += data->yspeed;

	struct tds_engine_object_list wall_list = tds_engine_get_object_list_by_type(tds_engine_global, "wall");

	for (int i = 0; i < wall_list.size; ++i) {
		if (tds_collision_get_overlap(ptr, wall_list.buffer[i])) {
			tds_object_free(ptr);
			return;
		}
	}
}

void tds_obj_bullet_draw(struct tds_object* ptr) {
}

void tds_obj_bullet_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {

}
