#include <stdio.h>
#include <glib.h>
#include "derp.h"

void create_plugin() {
	// Assert fact
	derp_assert_fact("(example (x 3) (y red) (z 1.5 b))");

	// Check for fact
	GSList* facts = derp_get_facts();
	GSList* node;
	for (int i = 0; (node = g_slist_nth(facts, i)); i++) {
		printf("Fact: %s\n", (char*)node->data);
	}

	// List rules
	GSList* rules = derp_get_rules();
	for (int i = 0; (node = g_slist_nth(rules, i)); i++) {
		printf("Rule: %s\n", (char*)node->data);
	}
}

static DerpPlugin plugin = {
	"Test",
	create_plugin
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

