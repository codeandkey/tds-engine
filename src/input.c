#include "input.h"
#include "log.h"
#include "memory.h"
#include "console.h"
#include "engine.h"
#include "msg.h"

#include <string.h>

void _tds_input_char_callback(GLFWwindow* window, unsigned int inp) {
	tds_console_char_pressed(tds_engine_global->console_handle, inp);
}

void _tds_input_mouse_callback(GLFWwindow* window, double mx, double my) {
	struct tds_input* ptr = (struct tds_input*) glfwGetWindowUserPointer(window);

	ptr->mx_last = ptr->mx;
	ptr->my_last = ptr->my;

	ptr->mx = mx;
	ptr->my = my;
}

void _tds_input_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	tds_engine_broadcast(tds_engine_global, (action == GLFW_PRESS) ? TDS_MSG_MOUSE_PRESSED : TDS_MSG_MOUSE_RELEASED, &button);
}

void _tds_input_scroll_callback(GLFWwindow* window, double sx, double sy) {
}

void _tds_input_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		tds_console_key_pressed(tds_engine_global->console_handle, key);
	}

	if (tds_engine_global->console_handle->enabled) {
		return;
	}

	tds_engine_broadcast(tds_engine_global, (action == GLFW_PRESS) ? TDS_MSG_KEY_PRESSED : TDS_MSG_KEY_RELEASED, &key);
}

struct tds_input* tds_input_create(struct tds_display* display_handle) {
	/* The input system is actually the only subsystem that requires GLFW callbacks, we can use the window's user pointer here. */

	struct tds_input* output = tds_malloc(sizeof(struct tds_input));
	memset(output, 0, sizeof(struct tds_input));

	output->window_handle = display_handle->win_handle;

	glfwSetWindowUserPointer(display_handle->win_handle, output);
	glfwSetCharCallback(display_handle->win_handle, _tds_input_char_callback);
	glfwSetCursorPosCallback(display_handle->win_handle, _tds_input_mouse_callback);
	glfwSetScrollCallback(display_handle->win_handle, _tds_input_scroll_callback);
	glfwSetKeyCallback(display_handle->win_handle, _tds_input_key_callback);

	return output;
}

void tds_input_free(struct tds_input* ptr) {
	tds_free(ptr);
}

void tds_input_update(struct tds_input* ptr) {
	for (int i = 0; i < 512; ++i) {
		ptr->kb_state[i] = glfwGetKey(ptr->window_handle, i);
	}

	for (int i = 0; i < 16; ++i) {
		ptr->mb_state[i] = glfwGetMouseButton(ptr->window_handle, i);
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

void tds_input_set_mouse(struct tds_input* ptr, double mx, double my) {
	glfwSetCursorPos(ptr->window_handle, mx, my);

	ptr->mx = mx;
	ptr->my = my;
}
