#include "display.h"
#include "memory.h"
#include "log.h"

static void _tds_display_err_callback(int code, const char* msg);

struct tds_display* tds_display_create(struct tds_display_desc desc) {
	struct tds_display* output = tds_malloc(sizeof(struct tds_display));

	glfwTerminate();

	if (!glfwInit()) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to initialize GLFW.\n");
		return NULL;
	}

	glfwWindowHint(GLFW_SAMPLES, desc.msaa);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwSetErrorCallback(_tds_display_err_callback);

	output->win_handle = glfwCreateWindow(desc.width, desc.height, "TDS", desc.fs ? glfwGetPrimaryMonitor() : NULL, NULL);
	output->desc = desc;

	if (!output->win_handle) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to initialize GLFW window.\n");
		return NULL;
	}

	glfwMakeContextCurrent(output->win_handle);

	if (glxwInit()) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to load GLXW.\n");
		return NULL;
	}

	glfwSetInputMode(output->win_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetWindowSize(output->win_handle, &output->desc.width, &output->desc.height);
	glfwSwapInterval(desc.vsync);

	glViewport(0, 0, output->desc.width, output->desc.height);

	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	tds_display_swap(output);

	return output;
}

void tds_display_free(struct tds_display* ptr) {
	glfwDestroyWindow(ptr->win_handle);
	glfwTerminate();

	tds_free(ptr);
}

void tds_display_swap(struct tds_display* ptr) {
	glfwSwapBuffers(ptr->win_handle);
}

void tds_display_update(struct tds_display* ptr) {
	glfwPollEvents();
}

int tds_display_get_close(struct tds_display* ptr) {
	return glfwWindowShouldClose(ptr->win_handle);
}

void _tds_display_err_callback(int code, const char* msg) {
	tds_logf(TDS_LOG_CRITICAL, "GLFW error %d : [%s]\n", code, msg);
}
