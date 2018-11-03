#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "derp.h"

static DerpPluginDescriptor plugin;

void start_plugin(struct DerpPlugin* self) {
	// Add rule
	ADD_RULE("foo",
		IF( T("dc:NLM", "dc:modified", "\"2008-01-14\"") ),
		THEN ( ASSERT("FOO", "FOO", "FOO")) );

	ADD_RULE("bar",
		IF( T("dc:NLM", "dc:modified", "\"2008-01-14\"") ),
		THEN ( ASSERT("BAR", "BAR", "BAR") ));

	ADD_RULE("baz",
		IF ( T("FOO", "FOO", "FOO"),
			 T("BAR", "BAR", "BAR") ),
		THEN ( ASSERT("BAZ", "BAZ", "BAZ")) );

	ADD_RULE("filtertest",
		IF ( TF("BAZ", "BAZ", "?o", "B.*Z") ),
		THEN ( ASSERT("BAZ", "did", "fire")) );

	ADD_RULE("callbacktest",
		IF ( T("FOO", "FOO", "?foo") ),
		THEN ( CALLBACK(self, "?foo" )) );

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

void callback(gchar* rule, GHashTable* arguments) {
	printf("Callback received for rule %s\n", rule);
	if (!g_strcmp0(rule, "callbacktest")) {
		printf("Bound argument is %s\n", (char*)g_hash_table_lookup(arguments, "foo"));
	}
}

static DerpPluginDescriptor plugin = {
	"Test",
	start_plugin,
	NULL,
	callback
};

DerpPluginDescriptor* derp_init_plugin(void) {
	return &plugin;
}

