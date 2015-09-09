#include "object.h"
#include "log.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

struct tds_object* tds_object_create(struct tds_object_type* type, struct tds_handle_manager* hmgr, struct tds_sprite_cache* smgr, float x, float y, float z, struct tds_object_param* param_list) {
	struct tds_object* output = tds_malloc(sizeof(struct tds_object));

	output->type_name = type->type_name;

	output->func_init = type->func_init;
	output->func_update = type->func_update;
	output->func_draw = type->func_draw;
	output->func_msg = type->func_msg;
	output->func_destroy = type->func_destroy;
	output->func_import = type->func_import;
	output->func_export = type->func_export;

	output->x = x;
	output->y = y;
	output->z = z;
	output->xspeed = output->yspeed = 0.0;
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

	if (output->sprite_handle) {
		output->cbox_width = output->sprite_handle->width;
		output->cbox_height = output->sprite_handle->height;
	}

	if (output->func_init) {
		tds_object_init(output);
	}

	if (output->func_import && param_list) {
		tds_object_import(output, param_list);
	}

	tds_logf(TDS_LOG_MESSAGE, "created object with handle %d, sprite %X\n", output->object_handle, (unsigned long) output->sprite_handle);

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

	tds_free(ptr->snd_src);
	tds_free(ptr);
}

void tds_object_set_sprite(struct tds_object* ptr, struct tds_sprite* sprite) {
	ptr->sprite_handle = sprite;
	ptr->current_frame = 0;
}

void tds_object_send_msg(struct tds_object* ptr, int handle, int msg, void* data) {
	struct tds_object* target = tds_handle_manager_get(ptr->hmgr, handle);

	target->func_msg(target, ptr, msg, data);
}

vec4* tds_object_get_transform(struct tds_object* ptr) {
	mat4x4 id, pos, rot;

	mat4x4_identity(ptr->transform);
	mat4x4_identity(id);

	mat4x4_translate(pos, ptr->x, ptr->y, ptr->z);
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
	ptr->func_init(ptr);
}

void tds_object_update(struct tds_object* ptr) {
	ptr->func_update(ptr);

	ptr->x += ptr->xspeed;
	ptr->y += ptr->yspeed;

	tds_object_update_sndsrc(ptr);
}

void tds_object_draw(struct tds_object* ptr) {
	tds_object_anim_update(ptr);
	ptr->func_draw(ptr);
}

void tds_object_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
	ptr->func_msg(ptr, sender, msg, param);
}

void tds_object_destroy(struct tds_object* ptr) {
	ptr->func_destroy(ptr);
}

void tds_object_import(struct tds_object* ptr, struct tds_object_param* param_list) {
	ptr->func_import(ptr, param_list);
}

struct tds_object_param* tds_object_export(struct tds_object* ptr) {
	return ptr->func_export(ptr);
}

void tds_object_update_sndsrc(struct tds_object* ptr) {
	tds_sound_source_set_pos(ptr->snd_src, ptr->x, ptr->y);
	tds_sound_source_set_vel(ptr->snd_src, ptr->xspeed, ptr->yspeed);
	tds_sound_source_set_vol(ptr->snd_src, ptr->snd_volume);
	tds_sound_source_set_loop(ptr->snd_src, ptr->snd_loop);
}
