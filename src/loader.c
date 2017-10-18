#include "loader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

int tds_loader_parse(struct tds_loader* state, struct tds_engine* eng, const char* filename) {
	xmlDoc* d;
	xmlNode* root, *cur_tld;

	if (!filename || !strlen(filename)) return 0;
	if (!(fd = fopen(filename, "r"))) return 0;
	if (!(state && eng)) return 0;

	if (!(d = xmlReadFile(filename, NULL, 0))) return 0;
	root = xmlDocGetRootElement(d);

	for (cur_tld = root->children; cur_tld; cur_tld = cur_tld->next) {
	}

	xmlFreeDoc(d);
	xmlCleanupParser();
	return 1;
}
