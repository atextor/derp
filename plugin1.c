#include <stdio.h>
#include "derp.h"

void create_plugin() {
	printf("create_plugin called\n");
}

static DerpPlugin plugin = {
	"Test",
	create_plugin
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

