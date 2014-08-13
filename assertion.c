#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oo.h"
#include "visibility.h"
#include "assertion.h"

static void* DerpAssertion_ctor(void* _self, va_list* app) {
	struct DerpAssertion* self = _self;
	struct DerpTriple* triple = va_arg(*app, struct DerpTriple*);
	self->triple = triple;
	assert(self->triple);

	return self;
}

static void* DerpAssertion_dtor(void* _self) {
	struct DerpAssertion* self = _self;
	free(self->triple);

	return self;
}

static char* DerpAssertion_tostring(void* _self) {
	struct DerpAssertion* self = _self;
	GString* string = g_string_new(NULL);
	const struct Object* o = (const struct Object*)self->triple;
	const struct Class* c = (const struct Class*)o->class;
	g_string_append_printf(string, "(assert %s)",
		c->tostring(self->triple));
	gchar* result = g_string_free(string, FALSE);
	assert(result);

	return result;
}

static const struct Class _DerpAssertion = {
	.size = sizeof(struct DerpAssertion),
	.ctor = DerpAssertion_ctor,
	.dtor = DerpAssertion_dtor,
	.clone = NULL,
	.equals = NULL,
	.tostring = DerpAssertion_tostring
};

EXPORT const void* DerpAssertion = &_DerpAssertion;

