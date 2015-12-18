#include "input_map.h"
#include "memory.h"
#include "log.h"

#include <math.h>

struct tds_input_map* tds_input_map_create(struct tds_input* ptr) {
	struct tds_input_map* output = tds_malloc(sizeof(struct tds_input_map));

	output->input_handle = ptr;
	output->use_controller = tds_input_get_controller(ptr);

	return output;
}

void tds_input_map_free(struct tds_input_map* ptr) {
	tds_free(ptr);
}

float tds_input_map_get_axis(struct tds_input_map* ptr, int key_low, int key_high, int axis) {
	if (ptr->use_controller && tds_input_get_controller(ptr->input_handle)) {
		return ptr->input_handle->controller_axis_state[axis];
	}

	return (float) (ptr->input_handle->kb_state[key_high] - ptr->input_handle->kb_state[key_low]);
}

int tds_input_map_get_key(struct tds_input_map* ptr, int key, int button) {
	if (ptr->use_controller && tds_input_get_controller(ptr->input_handle)) {
		return ptr->input_handle->controller_state[button];
	}

	return ptr->input_handle->kb_state[key];
}

int tds_input_map_get_mouse_button(struct tds_input_map* ptr, int mb, int button) {
	if (ptr->use_controller && tds_input_get_controller(ptr->input_handle)) {
		return ptr->input_handle->controller_state[button];
	}

	return ptr->input_handle->mb_state[mb];
}
