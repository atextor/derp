#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "derp.h"

static DerpPlugin plugin;

void start_plugin() {
	// Add rule
	ADD_RULE("foo",
		IF( T("dc:NLM", "dc:modified", "\"2008-01-14\"") ),
		THEN ( T("FOO", "FOO", "FOO")) );

	ADD_RULE("bar",
		IF( T("dc:NLM", "dc:modified", "\"2008-01-14\"") ),
		THEN ( T("BAR", "BAR", "BAR") ));

	ADD_RULE("baz",
		IF ( T("FOO", "FOO", "FOO"),
			 T("BAR", "BAR", "BAR") ),
		THEN ( T("BAZ", "BAZ", "BAZ")) );

	derp_add_callback(&plugin, "test", IF(T("BAZ", "BAZ", "BAZ")));

	// Check for fact
	/*
	GSList_String* facts = derp_get_facts();
	for (GSList_String* node = facts; node; node = node->next) {
		printf("Fact: %s\n", (char*)node->data);
	}
	g_slist_free_full(facts, derp_free_data);
	*/

	// List rules
	GSList_String* rules = derp_get_rules();
	for (GSList_String* node = rules; node; node = node->next) {
		printf("Rule: %s\n", (char*)node->data);
	}
	g_slist_free(rules);
}

void callback(gchar* rule) {
	printf("callback called for rule %s\n", rule);
}

static DerpPlugin plugin = {
	"Test",
	start_plugin,
	NULL,
	callback
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

