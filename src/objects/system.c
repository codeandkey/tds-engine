#include "system.h"

#include "../input_map.h"
#include "../key_map.h"
#include "../engine.h"

#include "../game/game_input.h"

#include <string.h>
#include <stdio.h>

struct tds_object_type tds_obj_system_type = {
	"system",
	0,
	(void*) 0,
	0,
	sizeof(struct tds_obj_system_data),
	tds_obj_system_init,
	tds_obj_system_destroy,
	tds_obj_system_update,
	tds_obj_system_draw,
	tds_obj_system_msg
};

void tds_obj_system_init(struct tds_object* ptr) {
	struct tds_obj_system_data* data = (struct tds_obj_system_data*) ptr->object_data;

	data->font = tds_sprite_cache_get(ptr->smgr, "font_debug");

	tds_sound_source_load_buffer(ptr->snd_src, tds_sound_cache_get(tds_engine_global->sndc_handle, "music_bg"));
	tds_sound_source_play(ptr->snd_src);

	ptr->snd_loop = 1;
}

void tds_obj_system_update(struct tds_object* ptr) {
	if (tds_input_map_get_key_pressed(tds_engine_global->input_map_handle, tds_key_map_get(tds_engine_global->key_map_handle, TDS_GAME_INPUT_QUIT), 0)) {
		tds_engine_terminate(tds_engine_global);
	}
}

void tds_obj_system_draw(struct tds_object* ptr) {
	struct tds_obj_system_data* data = (struct tds_obj_system_data*) ptr->object_data;
	struct tds_text_batch batch = {0};
	struct tds_camera* camera = tds_engine_global->camera_handle;

	batch.x = camera->x - camera->width / 2.0f + data->font->width / 2.0f;
	batch.y = camera->y + camera->height / 2.0f - data->font->height / 2.0f;
	batch.font = data->font;

	batch.r = 1.0f;
	batch.a = 1.0f;

	if (tds_engine_global->state.fps > 30.0f) {
		batch.g = 1.0f;
		batch.r = 0.0f;
	}

	if (tds_engine_global->state.fps > 58.0f) {
		batch.r = batch.g = 0.0f;
		batch.b = 1.0f;
	}

	snprintf(data->text_fps, sizeof(data->text_fps), "FPS : %.2f", tds_engine_global->state.fps);

	batch.str = data->text_fps;
	batch.str_len = strlen(data->text_fps);

	tds_text_submit(tds_engine_global->text_handle, &batch);
}

void tds_obj_system_destroy(struct tds_object* ptr) {
}

void tds_obj_system_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param) {
}
