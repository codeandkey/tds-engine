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
	xmlNode* root, *cur_tld, *inner;
	FILE* fd;

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
			//world_dec[world_dec_len] = 0;

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
