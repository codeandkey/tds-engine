#include "input_map.h"
#include "memory.h"

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

int tds_input_map_get_key_pressed(struct tds_input_map* ptr, int key, int button) {
	if (ptr->use_controller && tds_input_get_controller(ptr->input_handle)) {
		return ptr->input_handle->controller_state[button] && !ptr->input_handle->controller_state_last[button];
	}

	return ptr->input_handle->kb_state[key] && !ptr->input_handle->kb_state_last[key];
}

int tds_input_map_get_key_released(struct tds_input_map* ptr, int key, int button) {
	if (ptr->use_controller && tds_input_get_controller(ptr->input_handle)) {
		return !ptr->input_handle->controller_state[button] && ptr->input_handle->controller_state_last[button];
	}

	return !ptr->input_handle->kb_state[key] && ptr->input_handle->kb_state_last[key];
}

char tds_input_map_get_char(struct tds_input_map* ptr) {
	return (char) ptr->input_handle->last_char;
}
