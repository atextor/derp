#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "derp.h"

#define RULE_NAME "runexternalcallback"
static DerpPluginDescriptor plugin;
static gchar* attribute_run_str = NULL;

void start_plugin(struct DerpPlugin* self) {
	GString* run_str = g_string_new(NULL);
	g_string_append_printf(run_str, "%s_command", self->identifier);
	attribute_run_str = g_string_free(run_str, FALSE);

	// Register configurable attributes
	derp_assert_triple(self->identifier, "derp:reads", attribute_run_str);
	derp_assert_triple(attribute_run_str, "rdfs:range", "rdfs:Literal");
	derp_assert_triple(attribute_run_str, "rdfs:label", "\"run external command\"");
	derp_assert_triple(attribute_run_str, "rdfs:comment", "\"An external command to execute\"");

	// Add rule
	ADD_RULE(RULE_NAME,
		IF ( T(self->identifier, attribute_run_str, "?cmd") ),
		THEN ( CALLBACK(self, "?cmd" )) ); 
}

void shutdown_plugin() {
	g_free(attribute_run_str);
}

void callback(gchar* rule, GHashTable* arguments) {
	printf("CALLBACK\n");
	if (!g_strcmp0(rule, RULE_NAME)) {
		char* arg = (char*)g_hash_table_lookup(arguments, "cmd");
		derp_log(DERP_LOG_DEBUG, "Running command: %s", arg);
		system(arg);
	}
}

static DerpPluginDescriptor plugin = {
	"RunExternal",
	start_plugin,
	shutdown_plugin,
	callback
};

DerpPluginDescriptor* derp_init_plugin(void) {
	return &plugin;
}

