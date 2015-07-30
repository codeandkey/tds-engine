#include "handle.h"
#include "memory.h"
#include "log.h"

#include <string.h>

struct tds_handle_manager* tds_handle_manager_create(unsigned int buffer_size) {
	struct tds_handle_manager* output = tds_malloc(sizeof(struct tds_handle_manager));
	memset(output, 0, sizeof(struct tds_handle_manager));

	output->buffer_size = buffer_size;
	output->buffer = tds_malloc(sizeof(struct tds_handle_manager_entry) * buffer_size);
	output->current_handle = 1;
	output->max_index = 0;

	for (int i = 0; i < output->buffer_size; ++i) {
		output->buffer[i].index = 0;
		output->buffer[i].data = NULL;
	}

	return output;
}

void tds_handle_manager_free(struct tds_handle_manager* ptr) {
	tds_logf(TDS_LOG_DEBUG, "Freeing handle manager buffers.\n");

	tds_free(ptr->buffer);
	tds_free(ptr);
}

void* tds_handle_manager_get(struct tds_handle_manager* ptr, handle id) {
	for (int i = 0; i < ptr->max_index; ++i) {
		if (ptr->buffer[i].index == id) {
			return ptr->buffer[i].data;
		}
	}

	tds_logf(TDS_LOG_CRITICAL, "Handle not found in buffers");
	return NULL;
}

void tds_handle_manager_set(struct tds_handle_manager* ptr, handle id, void* data) {
	for (int i = 0; i < ptr->max_index; ++i) {
		if (ptr->buffer[i].index == id) {
			ptr->buffer[i].data = data;
			return;
		}
	}

	tds_logf(TDS_LOG_CRITICAL, "Handle not found in buffers");
}

handle tds_handle_manager_get_new(struct tds_handle_manager* ptr, void* data) {
	if (ptr->max_index >= ptr->buffer_size) {
		/* Resort the buffers here. */
		/* The resort operation will be a linear slide iteration over the buffer, quickly filling the lower slots. We will NOT null out the last bits of the list. */

		int new_index = 0;
		for (int i = 0; i < ptr->buffer_size; ++i) {
			if (ptr->buffer[i].data) {
				handle tmpi = ptr->buffer[i].index;
				void* tmpd = ptr->buffer[i].data;

				ptr->buffer[i].index = 0;
				ptr->buffer[i].data = NULL;

				ptr->buffer[new_index].index = tmpi;
				ptr->buffer[new_index++].data = tmpd;
			}
		}

		tds_logf(TDS_LOG_DEBUG, "Performed resort operation, reduced fragmentation from %d slots usage to %d (buffer size %d)..\n", ptr->max_index, new_index, ptr->buffer_size);

		ptr->max_index = new_index;
	}

	if (ptr->max_index >= ptr->buffer_size) {
		/* Buffer is full. */

		tds_logf(TDS_LOG_CRITICAL, "Buffer is full! (%d slots, post-resort)\n", ptr->buffer_size);
		return 0;
	}

	ptr->buffer[ptr->max_index].index = ptr->current_handle;
	ptr->buffer[ptr->max_index++].data = data;

	return ptr->current_handle++;
}
