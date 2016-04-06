#include "module.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>

struct tds_module_container* tds_module_container_create(void) {
	struct tds_module_container* output = tds_malloc(sizeof *output);

	output->modules = NULL;

	return output;
}

void tds_module_container_free(struct tds_module_container* ptr) {
	tds_module_container_flush(ptr);
	tds_free(ptr);
}

void tds_module_container_add(struct tds_module_container* ptr, struct tds_module_template type) {
	struct tds_module_instance* next_instance = tds_malloc(sizeof *next_instance);

	next_instance->type = type;
	next_instance->data = NULL;

	if (type.data_size) {
		next_instance->data = tds_malloc(type.data_size);
	}

	next_instance->next = ptr->modules;
	ptr->modules = next_instance;

	if (type.init) {
		type.init(next_instance->data);
	}

	tds_logf(TDS_LOG_MESSAGE, "Initialized module [%s]\n", type.name);
}

void tds_module_container_flush(struct tds_module_container* ptr) {
	struct tds_module_instance* cur = ptr->modules, *tmp = NULL;

	while (cur) {
		tmp = cur->next;

		if (cur->type.destroy) {
			cur->type.destroy(cur->data);
		}

		if (cur->data) {
			tds_free(cur->data);
		}

		tds_free(cur);
		cur = tmp;
	}

	ptr->modules = NULL;
}

void tds_module_container_update(struct tds_module_container* ptr) {
	struct tds_module_instance* cur = ptr->modules;

	while (cur) {
		if (cur->type.update) {
			cur->type.update(cur->data);
		}

		cur = cur->next;
	}
}

void tds_module_container_draw(struct tds_module_container* ptr) {
	struct tds_module_instance* cur = ptr->modules;

	while (cur) {
		if (cur->type.draw) {
			cur->type.draw(cur->data);
		}

		cur = cur->next;
	}
}

void tds_module_container_broadcast(struct tds_module_container* ptr, int msg, void* param) {
	struct tds_module_instance* cur = ptr->modules;

	while (cur) {
		if (cur->type.msg) {
			cur->type.msg(cur->data, msg, param);
		}

		cur = cur->next;
	}
}
