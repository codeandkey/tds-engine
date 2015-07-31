#include "wall.h"
#include "../engine.h"

struct tds_object_type tds_obj_wall_type = {
	"wall",
	"wall_concrete_medium",
	0,
	tds_obj_wall_init,
	tds_obj_wall_destroy,
	tds_obj_wall_update,
	tds_obj_wall_draw,
	tds_obj_wall_msg,
};

void tds_obj_wall_init(struct tds_object* ptr) {
}

void tds_obj_wall_destroy(struct tds_object* ptr) {
}

void tds_obj_wall_update(struct tds_object* ptr) {
}

void tds_obj_wall_draw(struct tds_object* ptr) {
}

void tds_obj_wall_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {

}
