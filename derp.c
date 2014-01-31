#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <glib.h>
#include <string.h>

#include "clips.h"
#include "derp.h"

#define ROUTER_NAME "derp_router"

DerpPlugin* load_plugin(char* filename) {
	DerpPlugin*(*derp_init_plugin)(void);
	DerpPlugin* plugin = NULL;
	char* error = NULL;
	void* handle;

	handle = dlopen(filename, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		return NULL;
	}

	dlerror();

	derp_init_plugin = (DerpPlugin*(*)(void))dlsym(handle, "derp_init_plugin");
	error = dlerror();
	if (error != NULL) {
		fprintf(stderr, "Error while loading plugin %s: %s\n", filename, error);
		return NULL;
	}

	plugin = derp_init_plugin();
	if (plugin == NULL) {
		fprintf(stderr, "Error while loading plugin %s: Invalid plugin struct\n", filename);
	}

	return plugin;
}

GSList* load_plugins(GSList* plugins) {
	GSList* node;
	GSList* result = NULL;
	char* plugin_filename;
	DerpPlugin* plugin = NULL;

	for (int i = 0; (node = g_slist_nth(plugins, i)); i++) {
		plugin_filename = (char*)node->data;
		plugin = load_plugin(plugin_filename);
		if (plugin != NULL) {
			result = g_slist_append(result, plugin);
		}
	}

	return result;
}

bool derp_assert_fact(char* fact) {
	void* result = AssertString(fact);
	return (result != NULL);
}

GSList* derp_get_facts() {
	GSList* list = NULL;
	Facts(ROUTER_NAME, NULL, -1, -1, -1);
	// TODO
	return list;
}

GSList* derp_get_rules() {
	// TODO
	return NULL;
}

GSList* derp_get_rule_definition(char* rulename) {
	// TODO
	return NULL;
}

int router_query_function(char* logical_name) {
	return strncmp(ROUTER_NAME, logical_name, strlen(ROUTER_NAME)) == 0;
}

int router_print_function(char* logical_name, char* str) {
	printf("%s", str);
	return 1;
}

int router_getc_function(char* logical_name) {
	return fgetc(stdin);
}

int router_ungetc_function(int ch, char* logical_name) {
	return ungetc(ch, stdout);
}

int router_exit_function(int exit_code) {
	return 0;
}

int main() {
	// Load Plugins
	GSList* list = NULL;
	list = g_slist_append(list, "./libplugin1.so");
	GSList* plugins = load_plugins(list);

	GSList *node;
	DerpPlugin* p;
	for (int i = 0; (node = g_slist_nth(plugins, i)); i++) {
		p = (DerpPlugin*)node->data;
		printf("Available plugin: %s\n", p->name);
	}

	// Initialize rule engine
	InitializeEnvironment();
	Load("init.clp");

	// Add I/O routers
	int result = AddRouter(ROUTER_NAME, 0,
			router_query_function,
			router_print_function,
			router_getc_function,
			router_ungetc_function,
			router_exit_function);
	if (result == 0) {
		fprintf(stderr, "I/O Router %s could not be created\n", ROUTER_NAME);
		exit(EXIT_FAILURE);
	}

	// Enter main program
	printf("Initialized\n");
	derp_assert_fact("(example (x 3) (y red) (z 1.5 b))");
	derp_get_facts();

	return 0;
}

