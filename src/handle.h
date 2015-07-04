#pragma once

/* The handle system is a simple indirection for managing pointers of dynamic world objects. */

typedef unsigned int handle;

struct handle_manager {
	void** buffer;
	unsigned int max_index, buffer_size;
};

struct handle_manager* handle_manager_create(unsigned int buffer_size);
void handle_manager_free(struct handle_manager* ptr);

void* handle_manager_get(struct handle_manager* ptr, handle id);
void handle_manager_set(struct handle_manager* ptr, handle id, void* data);

handle handle_manager_get_new(struct handle_manager* ptr, void* data);
