#include "display.h"
#include "memory.h"
#include "log.h"

struct tds_display* tds_display_create(struct tds_display_desc desc) {
	struct tds_display* output = tds_malloc(sizeof(struct tds_display));

	glfwTerminate();

	if (!glfwInit()) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to initialize GLFW.\n");
		return NULL;
	}

	output->win_handle = glfwCreateWindow(desc.width, desc.height, "TDS", desc.fs ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (!output->win_handle) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to initialize GLFW window.\n");
		return NULL;
	}

	glfwMakeContextCurrent(output->win_handle);
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
