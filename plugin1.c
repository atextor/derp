#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "derp.h"

void start_plugin() {
	// Add rule
	ADD_RULE("foo",
		IF( T("dc:NLM", "dc:modified", "\"2008-01-14\"") ),
		THEN ( T("FOO", "FOO", "FOO"),
			   T("BAR", "BAR", "BAR") ));

	// Check for fact
	GSList_String* facts = derp_get_facts();
	for (GSList_String* node = facts; node; node = node->next) {
		printf("Fact: %s\n", (char*)node->data);
	}
	g_slist_free_full(facts, derp_free_data);

	// List rules
	GSList_String* rules = derp_get_rules();
	for (GSList_String* node = rules; node; node = node->next) {
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

