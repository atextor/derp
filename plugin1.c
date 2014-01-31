#include <stdio.h>
#include "derp.h"

void create_plugin() {
	printf("create_plugin called\n");
}

static struct DerpPlugin plugin = {
	"Test",
	create_plugin
};

struct DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

