#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "derp.h"

void start_plugin() {
	// Assert fact
	derp_assert_rule(
			derp_new_rule(
				// Name
				g_string_new("foo"),
				// Head
				derp_new_triple_list(derp_new_triple("dc:NLM", "dc:modified", "\"2008-01-14\""), NULL),
				// Body
				derp_new_triple_list(derp_new_triple("FOO", "FOO", "FOO"), NULL)));

	// Check for fact
	GSList_String* facts = derp_get_facts();
	GSList_String* node;
	for (int i = 0; (node = g_slist_nth(facts, i)); i++) {
		printf("Fact: %s\n", (char*)node->data);
	}
	g_slist_free_full(facts, derp_free_data);

	// List rules
	GSList_String* rules = derp_get_rules();
	for (int i = 0; (node = g_slist_nth(rules, i)); i++) {
		printf("Rule: %s\n", (char*)node->data);
	}
	g_slist_free(rules);
}

static DerpPlugin plugin = {
	"Test",
	start_plugin,
	NULL
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

