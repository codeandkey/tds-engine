#pragma once

struct tds_module_template {
	const char* name;
	int data_size;
	void (*init)(void* data);
	void (*destroy)(void* data);
	void (*update)(void* data);
	void (*draw)(void* data);
	void (*msg)(void* data, int msg, void* param);
};

struct tds_module_instance {
	struct tds_module_template type;
	void* data;
	struct tds_module_instance* next;
};

struct tds_module_container {
	struct tds_module_instance* modules;
};

struct tds_module_container* tds_module_container_create(void);
void tds_module_container_free(struct tds_module_container* ptr);

void tds_module_container_add(struct tds_module_container* ptr, struct tds_module_template type);
void tds_module_container_flush(struct tds_module_container* ptr);

void tds_module_container_update(struct tds_module_container* ptr);
void tds_module_container_draw(struct tds_module_container* ptr);
void tds_module_container_broadcast(struct tds_module_container* ptr, int msg, void* param);
