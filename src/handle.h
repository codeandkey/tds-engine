#pragma once

/* The handle system is a simple indirection for managing pointers of dynamic world objects. */

typedef unsigned long handle;

struct tds_handle_manager_entry {
	handle index;
	void* data;
};

struct tds_handle_manager {
	struct tds_handle_manager_entry* buffer;
	unsigned int max_index, buffer_size;
	handle current_handle;
};

struct tds_handle_manager* tds_handle_manager_create(unsigned int buffer_size);
void tds_handle_manager_free(struct tds_handle_manager* ptr);

void* tds_handle_manager_get(struct tds_handle_manager* ptr, handle id);
void tds_handle_manager_set(struct tds_handle_manager* ptr, handle id, void* data);

handle tds_handle_manager_get_new(struct tds_handle_manager* ptr, void* data);
