#include "loader.h"
#include "log.h"
#include "memory.h"
#include "engine.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <openssl/bio.h>
#include <openssl/evp.h>

static size_t _decode_len(const char* inp);

struct tds_loader* tds_loader_create(void) {
	struct tds_loader* output = tds_malloc(sizeof *output);
	return output;
}

void tds_loader_free(struct tds_loader* ptr) {
	tds_free(ptr);
}

int tds_loader_parse(struct tds_loader* state, struct tds_engine* eng, const char* filename) {
	xmlDoc* d;
	xmlNode* root, *cur_tld, *inner, *prop;
	FILE* fd;

	int w_w = 0, w_h = 0; /* world width, height */

	if (!filename || !strlen(filename)) return 0;
	if (!(fd = fopen(filename, "r"))) return 0;
	if (!(state && eng)) return 0;

	if (!(d = xmlReadFile(filename, NULL, 0))) return 0;
	root = xmlDocGetRootElement(d);

	if (strcmp(root->name, "map")) {
		tds_logf(TDS_LOG_WARNING, "Map root element name is not <map> (<%s>), invalid file?\n", root->name);
		goto fail;
	}

	for (cur_tld = root->children; cur_tld; cur_tld = cur_tld->next) {
		if (!strcmp(cur_tld->name, "objectgroup")) {
			/* constructing some objects.. walk through children */

			for (inner = cur_tld->children; inner; inner = inner->next) {
				struct tds_object* new_object;
				struct tds_object_type* new_object_type;

				if (strcmp(inner->name, "object")) {
					continue;
				}

				/* first, get the typename, position, and cbox sizes */

				xmlChar* o_typename = xmlGetProp(inner, "type");
				xmlChar* o_pos_x = xmlGetProp(inner, "x");
				xmlChar* o_pos_y = xmlGetProp(inner, "y");
				xmlChar* o_pos_w = xmlGetProp(inner, "width");
				xmlChar* o_pos_h = xmlGetProp(inner, "height");

				struct tds_object_param* o_params = NULL;
				tds_bcp o_pos, o_cbox;

				o_pos.x = strtol(o_pos_x, NULL, 10);
				o_pos.y = strtol(o_pos_y, NULL, 10);

				o_cbox.x = strtol(o_pos_w, NULL, 10);
				o_cbox.y = strtol(o_pos_h, NULL, 10);

				/* the map position is the upper-left origin of the object, also treated with a RH origin */
				if (w_h) {
					o_pos.y = w_h * 16 - (o_pos.y + o_cbox.y);
				} else {
					tds_logf(TDS_LOG_WARNING, "No active current world size! I don't know where to place objects.\n");
				}

				new_object_type = tds_object_type_cache_get(eng->otc_handle, o_typename);

				if (!new_object_type) {
					tds_logf(TDS_LOG_WARNING, "Bad typename [%s], aborting\n", o_typename);
					continue;
				}

				/* now, we walk through inner properties and construct some params */
				prop = inner->children;

				while (prop && strcmp(prop->name, "properties")) {
					prop = prop->next;
				}

				/* confusing line, reuses prop only if the object has properties */
				for (prop = prop ? prop->children : NULL; prop; prop = prop->next) {
					/* iterate through property tags */
					if (strcmp(prop->name, "property")) {
						continue;
					}

					struct tds_object_param* p_next = tds_malloc(sizeof *p_next);

					p_next->next = o_params; /* extend linked list */
					o_params = p_next;

					xmlChar* p_name = xmlGetProp(prop, "name"), *p_index = p_name + 1;
					xmlChar* p_value = xmlGetProp(prop, "value");

					p_next->key = strtoul(p_index, NULL, 10); /* we just kinda hope the input makes sense */

					switch ((char) *p_name) {
					case 'f':
						p_next->type = TDS_PARAM_FLOAT;
						p_next->fpart = strtof(p_value, NULL);
						break;
					case 'i':
						p_next->type = TDS_PARAM_INT;
						p_next->ipart = strtol(p_value, NULL, 10);
						break;
					case 'u':
						p_next->type = TDS_PARAM_UINT;
						p_next->upart = strtoul(p_value, NULL, 10);
						break;
					case 's':
						memset(p_next->spart, 0, TDS_PARAM_VALSIZE); /* zero off */
						strncpy(p_next->spart, TDS_PARAM_VALSIZE, p_value);
						p_next->type = TDS_PARAM_STRING;
						break;
					}

					xmlFree(p_name);
					xmlFree(p_value);
				}

				/* everything's ready for the object */
				new_object = tds_object_create(new_object_type, eng->object_buffer, eng->sc_handle, o_pos, o_params);

				/* should be good, we set the cbox as well. */
				new_object->cbox = o_cbox;

				/* the object throws itself into the hmgr, we don't need to do anything more */
				/* perform xml cleanup */

				xmlFree(o_typename);
				xmlFree(o_pos_x);
				xmlFree(o_pos_y);
				xmlFree(o_pos_w);
				xmlFree(o_pos_h);
			}

		}

		if (!strcmp(cur_tld->name, "layer")) {
			inner = cur_tld->children;

			do {
				inner = inner->next;
			} while (inner && xmlStrcmp(inner->name, "data"));

			if (!inner) {
				tds_logf(TDS_LOG_WARNING, "layer doesn't contain a data element.\n");
				goto fail;
			}

			/* parsing a new world layer -- look for a data element */
			xmlChar* data, *width, *height;

			if (!(width = xmlGetProp(cur_tld, "width"))) {
				tds_logf(TDS_LOG_WARNING, "Layer has no width property\n");
				goto fail;
			}

			if (!(height = xmlGetProp(cur_tld, "height"))) {
				tds_logf(TDS_LOG_WARNING, "Layer has no height property\n");
				goto fail;
			}

			uint32_t w = strtol(width, NULL, 10), h = strtol(height, NULL, 10);

			if (w != w_w || h != w_h) {
				if (w_w || w_h) {
					tds_logf(TDS_LOG_WARNING, "World layer size mismatch! Objects might be out of place. Read size (%d,%d) while previously read size was (%d,%d)\n", w, h, w_w, w_h);
				} else {
					w_w = w;
					w_h = h;
				}
			}

			xmlFree(width);
			xmlFree(height);

			if (!(data = xmlGetProp(inner, "encoding"))) {
				tds_logf(TDS_LOG_WARNING, "World data missing encoding\n");
				xmlFree(data);
				goto fail;
			}

			if (strcmp(data, "base64")) {
				tds_logf(TDS_LOG_WARNING, "World data should be encoded as base64.\n");
				xmlFree(data);
				goto fail;
			}

			tds_logf(TDS_LOG_DEBUG, "Decoding world base64 data.\n");

			xmlFree(data);
			data = xmlNodeGetContent(inner);

			tds_logf(TDS_LOG_DEBUG, "inner string = %d bytes\n", strlen(data));
			tds_logf(TDS_LOG_DEBUG, "stripping leading, trailling whitespace..\n");

			char* c = data + strlen(data) - 1;

			while (isspace(*data)) data++;
			while (isspace(*c)) *(c--) = 0;


			tds_logf(TDS_LOG_DEBUG, "stripped inner string = %d bytes\n", strlen(data));

			/* size_t world_dec_len; // b64 library
			uint8_t* world_dec = b64_decode_ex(data, strlen(data), &world_dec_len); */

			BIO* bio, *b64;
			size_t world_dec_len = _decode_len(data);

			tds_logf(TDS_LOG_DEBUG, "estimated decode length: %d bytes\n", world_dec_len);

			uint32_t* world_dec = tds_malloc(world_dec_len + 1);

			bio = BIO_new_mem_buf(data, -1);
			b64 = BIO_new(BIO_f_base64());
			bio = BIO_push(b64, bio);

			BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
			int len = BIO_read(bio, world_dec, strlen(data));

			if (len != world_dec_len) {
				tds_logf(TDS_LOG_WARNING, "base64 length mismatch during decoding (%d bytes decoded), something went very wrong\n", len);
				goto fail;
			}

			BIO_free_all(bio);

			uint8_t* block_ids = tds_malloc(w * h * sizeof *block_ids);

			tds_logf(TDS_LOG_DEBUG, "Decoded to %d bytes, Copying block ids to u8 buffer\n", world_dec_len);

			for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				int index = x + (h - (y + 1)) * w * 4;

				if (index >= world_dec_len) {
					tds_logf(TDS_LOG_WARNING, "base64 data is shorter than expected.\n");
					goto fail;
				}

				block_ids[x + y * w] = (uint32_t) world_dec[x + (h - (y + 1)) * w]; /* grab LSB of blockids */
			}

			tds_logf(TDS_LOG_DEBUG, "Done. Freeing world b64 data.\n");
			tds_free(world_dec);

			/* fresh block information, we're not going to stream it so we pass it to tds_world */

			tds_logf(TDS_LOG_DEBUG, "Initializing world structure\n");
			struct tds_world* next_world = tds_world_create();

			tds_logf(TDS_LOG_DEBUG, "Initializing world data\n");
			tds_world_load(next_world, block_ids, w, h, 0, 0);

			tds_logf(TDS_LOG_DEBUG, "Done. Appending layer to engine list.\n");

			if (eng->world_buffer_count < sizeof(eng->world_buffer) / sizeof (*eng->world_buffer)) {
				eng->world_buffer[eng->world_buffer_count++] = next_world;
			} else {
				tds_logf(TDS_LOG_WARNING, "World layer overflow, tossing world data\n");
				tds_world_free(next_world);
			}

			tds_free(block_ids);
		}

		/* we don't actually load tilesets! */
	}

	xmlFreeDoc(d);
	xmlCleanupParser();
	return 1;
	fail:
	xmlFreeDoc(d);
	xmlCleanupParser();
	return 0;
}

size_t _decode_len(const char* inp) {
	size_t len = strlen(inp), padding = 0;
	if (inp[len - 1] == '=') {
		if (inp[len - 2] == '=') {
			padding = 2;
		} else {
			padding = 1;
		}
	}

	return (len * 3) / 4 - padding;
}
