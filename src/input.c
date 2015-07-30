#include "input.h"
#include "log.h"
#include "memory.h"

#include <string.h>

void _tds_input_char_callback(GLFWwindow* window, unsigned int inp) {
	struct tds_input* ptr = (struct tds_input*) glfwGetWindowUserPointer(window);

	ptr->last_char = inp;
}

void _tds_input_mouse_callback(GLFWwindow* window, double mx, double my) {
	struct tds_input* ptr = (struct tds_input*) glfwGetWindowUserPointer(window);

	ptr->mx = mx;
	ptr->my = my;
}

struct tds_input* tds_input_create(struct tds_display* display_handle) {
	/* The input system is actually the only subsystem that requires GLFW callbacks, we can use the window's user pointer here. */

	struct tds_input* output = tds_malloc(sizeof(struct tds_input));
	memset(output, 0, sizeof(struct tds_input));

	output->window_handle = display_handle->win_handle;

	glfwSetWindowUserPointer(display_handle->win_handle, output);
	glfwSetCharCallback(display_handle->win_handle, _tds_input_char_callback);
	glfwSetCursorPosCallback(display_handle->win_handle, _tds_input_mouse_callback);

	return output;
}

void tds_input_free(struct tds_input* ptr) {
	tds_free(ptr);
}

void tds_input_update(struct tds_input* ptr) {
	memcpy(ptr->kb_state_last, ptr->kb_state, sizeof(int) * 512);
	memcpy(ptr->controller_state_last, ptr->controller_state, sizeof(int) * 32);

	for (int i = 0; i < 512; ++i) {
		ptr->kb_state[i] = glfwGetKey(ptr->window_handle, i);
	}

	if (!tds_input_get_controller(ptr)) {
		return;
	}

	int state_count;
	const unsigned char* states = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &state_count);

	for (int i = 0; i < 32; ++i) {
		if (i >= state_count) {
			break;
		}

		ptr->controller_state[i] = (states[i] != 0);
	}

	int axis_count;
	const float* axis_states = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axis_count);

	for (int i = 0; i < 32; ++i) {
		if (i >= axis_count) {
			break;
		}

		ptr->controller_axis_state[i] = axis_states[i];
	}
}

int tds_input_get_controller(struct tds_input* ptr) {
	return glfwJoystickPresent(GLFW_JOYSTICK_1);
}

int tds_input_get_char(struct tds_input* ptr) {
	int out = ptr->last_char;
	ptr->last_char = 0;

	return out;
}
