#include "handle.h"
#include "memory.h"
#include "log.h"

#include <string.h>

struct handle_manager* handle_manager_create(unsigned int buffer_size) {
	struct handle_manager* output = tds_malloc(sizeof(struct handle_manager));
	memset(output, 0, sizeof(struct handle_manager));

	output->buffer_size = buffer_size;
	output->buffer = tds_malloc(sizeof(void*) * buffer_size);

	return output;
}

void handle_manager_free(struct handle_manager* ptr) {
	tds_free(ptr->buffer);
	tds_free(ptr);
}

void* handle_manager_get(struct handle_manager* ptr, handle id) {
	return ptr->buffer[id];
}

void handle_manager_set(struct handle_manager* ptr, handle id, void* data) {
	if (!ptr->buffer[id]) {
		tds_logf(TDS_LOG_WARNING, "Detected random access to handle buffers.. be careful. This can mess with the indexing process.\n");
	}

	ptr->buffer[id] = data;
}

handle handle_manager_get_new(struct handle_manager* ptr, void* data) {
	if (ptr->max_index >= ptr->buffer_size) {
		/* Resort the buffers here. */
		/* The resort operation will be a linear slide iteration over the buffer, quickly filling the lower slots. We will NOT null out the last bits of the list. */

		int new_index = 0;
		for (int i = 0; i < ptr->buffer_size; ++i) {
			if (ptr->buffer[i]) {
				ptr->buffer[new_index++] = ptr->buffer[i];
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

	ptr->buffer[ptr->max_index] = data;
	return ptr->max_index++;
}
