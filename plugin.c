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
	if (error) {
		derp_log(DERP_LOG_ERROR, "Error while loading plugin %s: %s", file_name, error);
		return NULL;
	}

	plugin_descriptor = derp_init_plugin();
	if (!plugin_descriptor) {
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

static char* DerpPlugin_tostring(void* _self) {
	struct DerpPlugin* self = _self;
	GString* str = g_string_new(NULL);
	g_string_append_printf(str, "DerpPlugin(%p, name=%s, file_name=%s)", self, self->name, self->file_name);
	char* string = g_string_free(str, FALSE);
	assert(string);
	return string;
}

static const struct Class _DerpPlugin = {
	.size = sizeof(struct DerpPlugin),
	.name = "DerpPlugin",
	.ctor = DerpPlugin_ctor,
	.dtor = DerpPlugin_dtor,
	.clone = NULL,
	.equals = NULL,
	.tostring = DerpPlugin_tostring
};

const void* DerpPlugin = &_DerpPlugin;

