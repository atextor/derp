#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <glib.h>

#include "clips.h"
#include "derp.h"

GSList* load_plugins(GSList* plugins) {
	GSList* node;
	GSList* result = NULL;
	char* plugin_file_name;
	char* error;
	struct DerpPlugin*(*derp_init_plugin)(void);
	void* handle;
	struct DerpPlugin* plugin = NULL;

	for (int i = 0; (node = g_slist_nth(plugins, i)); i++) {
		plugin_file_name = (char*)node->data;
		handle = dlopen(plugin_file_name, RTLD_LAZY);
		if (!handle) {
			fprintf(stderr, "%s\n", dlerror());
			return NULL;
		}

		dlerror();

		derp_init_plugin = (struct DerpPlugin*(*)(void))dlsym(handle, "derp_init_plugin");
		error = dlerror();
		if (error != NULL) {
			fprintf(stderr, "Error while loading plugin %s: %s\n", plugin_file_name, error);
			continue;
		}

		plugin = derp_init_plugin();
		if (plugin == NULL) {
			fprintf(stderr, "Error while loading plugin %s: Invalid plugin struct\n", plugin_file_name);
			continue;
		}
		result = g_slist_append(result, plugin);
	}

	return result;
}

int main() {
	GSList* list = NULL;
	list = g_slist_append(list, "./libplugin1.so");
	GSList* plugins = load_plugins(list);

	GSList *node;
	struct DerpPlugin* p;
	for (int i = 0; (node = g_slist_nth(plugins, i)); i++) {
		p = (struct DerpPlugin*)node->data;
		printf("Available plugin: %s\n", p->name);
	}

	return 0;
}

