#include "object.h"
#include "log.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

struct tds_object* tds_object_create(struct tds_object_type* type, struct tds_handle_manager* hmgr, struct tds_sprite_cache* smgr, tds_bcp pos, struct tds_object_param* param_list) {
	struct tds_object* output = tds_malloc(sizeof(struct tds_object));

	output->type_name = type->type_name;

	tds_logf(TDS_LOG_MESSAGE, "Creating object of type [%s]\n", type->type_name);

	output->func_init = type->func_init;
	output->func_update = type->func_update;
	output->func_draw = type->func_draw;
	output->func_msg = type->func_msg;
	output->func_destroy = type->func_destroy;

	output->pos = pos;
	output->save = type->save;
	output->speed = tds_vec2_zero;
	output->snd_volume = 1.0f;
	output->snd_loop = 0;
	output->angle = 0.0f;
	output->layer = 0; /* Layers are rendered with lower numbers on bottom, higher numbers on top. */

	output->r = output->g = output->b = output->a = 1.0f;

	output->sprite_handle = type->default_sprite ? tds_sprite_cache_get(smgr, type->default_sprite) : NULL;
	output->visible = (output->sprite_handle != NULL);

	output->object_data = type->data_size ? tds_malloc(type->data_size) : NULL;
	output->object_handle = tds_handle_manager_get_new(hmgr, output);
	output->hmgr = hmgr;
	output->smgr = smgr;
	output->current_frame = 0;

	output->anim_lastframe = tds_clock_get_point();
	output->anim_oneshot = 0;
	output->anim_speed_offset = 0.0f;
	output->anim_running = (output->sprite_handle != NULL);

	output->snd_src = tds_sound_source_create();
	output->param_list = param_list;

	if (output->sprite_handle) {
		output->cbox.x = output->sprite_handle->texture->dim.x;
		output->cbox.y = output->sprite_handle->texture->dim.y;
	}

	tds_logf(TDS_LOG_MESSAGE, "created object with handle %d, sprite %X\n", output->object_handle, (unsigned long) output->sprite_handle);

	if (output->func_init) {
		tds_object_init(output);
	}

	return output;
}

void tds_object_free(struct tds_object* ptr) {
	if (ptr->func_destroy) {
		tds_object_destroy(ptr);
	}

	if (ptr->hmgr) {
		tds_handle_manager_set(ptr->hmgr, ptr->object_handle, NULL);
	}

	if (ptr->object_data) {
		tds_free(ptr->object_data);
	}

	struct tds_object_param* head = ptr->param_list, *tmp = NULL;

	while (head) {
		tmp = head->next;
		tds_free(head);
		head = tmp;
	}

	tds_sound_source_free(ptr->snd_src);
	tds_free(ptr);
}

void tds_object_set_sprite(struct tds_object* ptr, struct tds_sprite* sprite) {
	if (ptr->sprite_handle != sprite) {
		ptr->sprite_handle = sprite;
		ptr->current_frame = 0;
	}
}

void tds_object_send_msg(struct tds_object* ptr, int handle, int msg, void* data) {
	struct tds_object* target = tds_handle_manager_get(ptr->hmgr, handle);

	target->func_msg(target, ptr, msg, data);
}

vec4* tds_object_get_transform(struct tds_object* ptr) {
	mat4x4 id, pos, rot;

	mat4x4_identity(ptr->transform);
	mat4x4_identity(id);

	mat4x4_translate(pos, ptr->pos.x, ptr->pos.y, 0.0);
	mat4x4_rotate_Z(rot, id, ptr->angle);

	mat4x4_mul(ptr->transform, pos, rot);

	return ptr->transform;
};

void tds_object_anim_update(struct tds_object* ptr) {
	if (!ptr->anim_running || !ptr->sprite_handle) {
		return;
	}

	double current_time = tds_clock_get_ms(ptr->anim_lastframe);
	double interval = (double) ptr->sprite_handle->animation_rate + ptr->anim_speed_offset;

	if (!interval) {
		return;
	}

	if (current_time >= interval) {
		ptr->anim_lastframe = tds_clock_get_point();

		if (ptr->current_frame == ptr->sprite_handle->texture->frame_count - 1) {
			if (ptr->anim_oneshot) {
				ptr->anim_running = 0;
			} else {
				ptr->current_frame = 0;
			}
		} else {
			++ptr->current_frame;
		}
	}
}

void tds_object_anim_start(struct tds_object* ptr) {
	ptr->current_frame = 0;
	ptr->anim_running = 1;
	ptr->anim_lastframe = tds_clock_get_point();
}

void tds_object_anim_pause(struct tds_object* ptr) {
	ptr->anim_running = !ptr->anim_running;
}

void tds_object_anim_setframe(struct tds_object* ptr, int frame) {
	ptr->current_frame = frame;
}

int tds_object_anim_oneshot_finished(struct tds_object* ptr) {
	if (!ptr->sprite_handle) {
		return 0;
	}

	return (ptr->anim_oneshot && ptr->current_frame == ptr->sprite_handle->texture->frame_count - 1 && !ptr->anim_running);
}

void tds_object_init(struct tds_object* ptr) {
	if (!ptr->func_init) {
		return;
	}

	ptr->func_init(ptr);
}

void tds_object_update(struct tds_object* ptr) {
	if (ptr->func_update) {
		ptr->func_update(ptr);
	}

	ptr->pos.x += ptr->speed.x;
	ptr->pos.y += ptr->speed.y;

	tds_object_update_sndsrc(ptr);
}

void tds_object_draw(struct tds_object* ptr) {
	tds_object_anim_update(ptr);

	if (ptr->func_draw) {
		ptr->func_draw(ptr);
	}
}

void tds_object_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
	if (ptr->func_msg) {
		ptr->func_msg(ptr, sender, msg, param);
	}
}

void tds_object_destroy(struct tds_object* ptr) {
	if (ptr->func_destroy) {
		ptr->func_destroy(ptr);
	}
}

void tds_object_update_sndsrc(struct tds_object* ptr) {
	tds_sound_source_set_pos(ptr->snd_src, ptr->pos.x, ptr->pos.y);
	tds_sound_source_set_vel(ptr->snd_src, ptr->speed.x, ptr->speed.y);
	tds_sound_source_set_vol(ptr->snd_src, ptr->snd_volume);
	tds_sound_source_set_loop(ptr->snd_src, ptr->snd_loop);
}

int* tds_object_get_ipart(struct tds_object* ptr, unsigned int index) {
	struct tds_object_param* head = ptr->param_list;

	while (head) {
		if (head->key == index) {
			if (head->type == TDS_PARAM_INT) {
				return &head->ipart;
			} else {
				tds_logf(TDS_LOG_WARNING, "Type mismatch in parameter.\n");
				return NULL;
			}
		}

		head = head->next;
	}

	return NULL;
}

char* tds_object_get_spart(struct tds_object* ptr, unsigned int index) {
	struct tds_object_param* head = ptr->param_list;

	while (head) {
		if (head->key == index) {
			if (head->type == TDS_PARAM_STRING) {
				return head->spart;
			} else {
				tds_logf(TDS_LOG_WARNING, "Type mismatch in parameter.\n");
				return NULL;
			}
		}

		head = head->next;
	}

	return NULL;
}

unsigned int* tds_object_get_upart(struct tds_object* ptr, unsigned int index) {
	struct tds_object_param* head = ptr->param_list;

	while (head) {
		if (head->key == index) {
			if (head->type == TDS_PARAM_UINT) {
				return &head->upart;
			} else {
				tds_logf(TDS_LOG_WARNING, "Type mismatch in parameter.\n");
				return NULL;
			}
		}

		head = head->next;
	}

	return NULL;
}

float* tds_object_get_fpart(struct tds_object* ptr, unsigned int index) {
	struct tds_object_param* head = ptr->param_list;

	while (head) {
		if (head->key == index) {
			if (head->type == TDS_PARAM_FLOAT) {
				return &head->fpart;
			} else {
				tds_logf(TDS_LOG_WARNING, "Type mismatch in parameter.\n");
				return NULL;
			}
		}

		head = head->next;
	}

	return NULL;
}

void tds_object_set_ipart(struct tds_object* ptr, unsigned int index, int value) {
	struct tds_object_param* cur = ptr->param_list, *head = ptr->param_list, *new = NULL;

	while (cur) {
		if (cur->key == index) {
			cur->type = TDS_PARAM_INT;
			cur->ipart = value;

			return;
		}

		cur = cur->next;
	}

	new = tds_malloc(sizeof *new);

	new->key = index;
	new->type = TDS_PARAM_INT;
	new->ipart = value;

	// Growing the list from the head decreases code size. A lot.
	new->next = head;
	ptr->param_list = new;
}

void tds_object_set_spart(struct tds_object* ptr, unsigned int index, char* value, int value_len) {
	struct tds_object_param* cur = ptr->param_list, *head = ptr->param_list, *new = NULL;

	int copy_len = value_len > TDS_PARAM_VALSIZE ? TDS_PARAM_VALSIZE : value_len;

	if (copy_len < 0) {
		tds_logf(TDS_LOG_WARNING, "Invalid parameter string length.\n");
		return;
	}

	while (cur) {
		if (cur->key == index) {
			cur->type = TDS_PARAM_INT;
			memset(cur->spart, 0, TDS_PARAM_VALSIZE);
			memcpy(cur->spart, value, copy_len);

			return;
		}

		cur = cur->next;
	}

	new = tds_malloc(sizeof *new);

	new->key = index;
	new->type = TDS_PARAM_INT;
	memset(new->spart, 0, TDS_PARAM_VALSIZE);
	memcpy(new->spart, value, copy_len);

	new->next = head;
	ptr->param_list = new;
}

void tds_object_set_upart(struct tds_object* ptr, unsigned int index, unsigned int value) {
	struct tds_object_param* cur = ptr->param_list, *head = ptr->param_list, *new = NULL;

	while (cur) {
		if (cur->key == index) {
			cur->type = TDS_PARAM_UINT;
			cur->upart = value;

			return;
		}

		cur = cur->next;
	}

	new = tds_malloc(sizeof *new);

	new->key = index;
	new->type = TDS_PARAM_UINT;
	new->upart = value;

	new->next = head;
	ptr->param_list = new;
}

void tds_object_set_fpart(struct tds_object* ptr, unsigned int index, float value) {
	struct tds_object_param* cur = ptr->param_list, *head = ptr->param_list, *new = NULL;

	while (cur) {
		if (cur->key == index) {
			cur->type = TDS_PARAM_FLOAT;
			cur->fpart = value;

			return;
		}

		cur = cur->next;
	}

	new = tds_malloc(sizeof *new);

	new->key = index;
	new->type = TDS_PARAM_FLOAT;
	new->fpart = value;

	new->next = head;
	ptr->param_list = new;
}

void tds_object_unset(struct tds_object* ptr, unsigned int index) {
	struct tds_object_param* cur = ptr->param_list, *prev = NULL;

	while (cur) {
		if (cur->key == index) {
			if (prev) {
				prev->next = cur->next;
			} else {
				ptr->param_list = cur->next;
			}

			tds_free(cur);
			return;
		}

		cur = cur->next;
		prev = cur;
	}
}
