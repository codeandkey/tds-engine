#include "loader.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

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
			/* parsing a new world layer -- look for a data element */
			xmlChar* data;

			inner = cur_tld->children;
			if (!inner || strcmp(inner->name, "data")) {
				tds_logf(TDS_LOG_WARNING, "Improper formatting of internal world data.\n");
				goto fail;
			}

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

			uint8_t* world_dec = b64_decode(data, strlen(data));

			xmlFree(data);
		}
	}

	xmlFreeDoc(d);
	xmlCleanupParser();
	return 1;
	fail:
	xmlFreeDoc(d);
	xmlCleanupParser();
	return 0;
}
