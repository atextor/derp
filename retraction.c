#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oo.h"
#include "visibility.h"
#include "retraction.h"

static void* DerpRetraction_ctor(void* _self, va_list* app) {
	struct DerpRetraction* self = _self;
	struct DerpTriple* triple = va_arg(*app, struct DerpTriple*);
	self->triple = triple;
	assert(self->triple);

	return self;
}

static void* DerpRetraction_dtor(void* _self) {
	struct DerpRetraction* self = _self;
	free(self->triple);

	return self;
}

static char* DerpRetraction_tostring(void* _self) {
	struct DerpRetraction* self = _self;
	GString* string = g_string_new(NULL);
	const struct Object* o = (const struct Object*)self->triple;
	const struct Class* c = (const struct Class*)o->class;
	g_string_append_printf(string, "(retract %s)",
		c->tostring(self->triple));
	gchar* result = g_string_free(string, FALSE);
	assert(result);

	return result;
}

static const struct Class _DerpRetraction = {
	.size = sizeof(struct DerpRetraction),
	.ctor = DerpRetraction_ctor,
	.dtor = DerpRetraction_dtor,
	.clone = NULL,
	.equals = NULL,
	.tostring = DerpRetraction_tostring
};

EXPORT const void* DerpRetraction = &_DerpRetraction;

