#include "rt.h"
#include "log.h"
#include "memory.h"
#include "engine.h"

#include <GLXW/glxw.h>

struct tds_rt* tds_rt_create(unsigned int width, unsigned int height) {
	struct tds_rt* output = tds_malloc(sizeof *output);

	glGenFramebuffers(1, &output->gl_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, output->gl_fb);

	glGenTextures(1, &output->gl_tex);
	glBindTexture(GL_TEXTURE_2D, output->gl_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &output->gl_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, output->gl_rb);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, output->gl_rb);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, output->gl_tex, 0);

	GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to create OpenGL framebuffer.\n");
		return NULL;
	}

	output->width = width;
	output->height = height;

	return output;
}

void tds_rt_free(struct tds_rt* ptr) {
	if (!ptr) {
		return;
	}

	if (ptr->gl_tex) {
		glDeleteTextures(1, &ptr->gl_tex);
	}

	if (ptr->gl_fb) {
		glDeleteFramebuffers(1, &ptr->gl_fb);
	}

	if (ptr->gl_rb) {
		glDeleteRenderbuffers(1, &ptr->gl_rb);
	}

	tds_free(ptr);
}

void tds_rt_bind(struct tds_rt* ptr) {
	unsigned int t_fb = ptr ? ptr->gl_fb : 0;
	glBindFramebuffer(GL_FRAMEBUFFER, t_fb);

	if (ptr) {
		glViewport(0, 0, ptr->width, ptr->height);
	} else {
		glViewport(0, 0, tds_engine_global->display_handle->desc.width, tds_engine_global->display_handle->desc.height);
	}
}
