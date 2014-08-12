#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#include "derp.h"
#include "plugin.h"

static void* DerpPlugin_ctor(void* _self, va_list* app) {
	struct DerpPlugin* self = _self;
	const char* file_name = va_arg(*app, const char*);

	// Load shared object
	DerpPluginDescriptor*(*derp_init_plugin)(void);
	DerpPluginDescriptor* plugin_descriptor = NULL;
	gchar* error = NULL;
	void* handle;

	handle = dlopen(file_name, RTLD_LAZY);
	if (!handle) {
		derp_log(DERP_LOG_ERROR, "%s", dlerror());
		return NULL;
	}

	dlerror();

	derp_init_plugin = (DerpPluginDescriptor*(*)(void))dlsym(handle, "derp_init_plugin");
	error = dlerror();
	if (error != NULL) {
		derp_log(DERP_LOG_ERROR, "Error while loading plugin %s: %s", file_name, error);
		return NULL;
	}

	plugin_descriptor = derp_init_plugin();
	if (plugin_descriptor == NULL) {
		derp_log(DERP_LOG_ERROR, "Error while loading plugin %s: Invalid plugin struct", file_name);
	}

	// Copy plugin descriptor info into plugin instance
	self->file_name = malloc(strlen(file_name) + 1);
	assert(self->file_name);
	strcpy(self->file_name, file_name);
	const char* name = plugin_descriptor->name;
	self->name = malloc(strlen(name) + 1);
	assert(self->name);
	strcpy(self->name, name);
	self->start_plugin = plugin_descriptor->start_plugin;
	self->shutdown_plugin = plugin_descriptor->shutdown_plugin;
	self->callback = plugin_descriptor->callback;

	return self;
}

static void* DerpPlugin_dtor(void* _self) {
	struct DerpPlugin* self = _self;
	free(self->name);
	free(self->file_name);
	return self;
}

static const struct Class _DerpPlugin = {
	.super = NULL,
	.size = sizeof(struct DerpPlugin),
	.ctor = DerpPlugin_ctor,
	.dtor = DerpPlugin_dtor,
	.clone = NULL,
	.equals = NULL
};

const void* DerpPlugin = &_DerpPlugin;

