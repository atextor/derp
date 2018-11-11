#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oo.h"
#include "visibility.h"
#include "action.h"

static void* DerpAction_ctor(void* _self, va_list* app) {
	struct DerpAction* self = _self;
	struct DerpPlugin* callee = va_arg(*app, struct DerpPlugin*);
	self->callee = callee;
	assert(self->callee);

	GSList_String* vars = NULL;
	char* var = va_arg(*app, char*);
	while (var) {
		char* copy = strdup(var);
		assert(copy);
		vars = g_slist_append(vars, copy);
		var = va_arg(*app, char*);
	}

	self->vars = vars;
	// This is not known at construction time!
	self->rule = NULL;
	return self;
}

static void* DerpAction_dtor(void* _self) {
	struct DerpAction* self = _self;
	if (self->vars) {
		g_slist_free_full(self->vars, free);
	}
	return self;
}

static char* DerpAction_tostring(void* _self) {
	struct DerpAction* self = _self;
	GString* string = g_string_new(NULL);
	char* callee_name = self->callee->name;
	char* rule_name = self->rule ? self->rule->name : "(unknown rule)";
	g_string_append_printf(string, "(rule_callback \"%s\" \"%s\" ", callee_name, rule_name);

	char* var;
	for (GSList_String* node = self->vars; node; node = node->next) {
		var = (gchar*)node->data;
		g_string_append_printf(string, "\"%s\" %s ", var + 1, var);
	}
	g_string_append(string, ")");
	char* result = g_string_free(string, FALSE);

	return result;
}

static const struct Class _DerpAction = {
	.size = sizeof(struct DerpAction),
	.name = "DerpAction",
	.ctor = DerpAction_ctor,
	.dtor = DerpAction_dtor,
	.clone = NULL,
	.equals = NULL,
	.tostring = DerpAction_tostring
};

EXPORT const void* DerpAction = &_DerpAction;

