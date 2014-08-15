#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <glib.h>
#include "oo.h"
#include "visibility.h"

static void* Object_ctor(void* _self, va_list* app) {
	struct Object* self = _self;
	return self;
}

static void* Object_dtor(void* _self) {
	struct Object* self = _self;
	return self;
}

static void* Object_clone(void* _self) {
	struct Object* self = _self;
	struct Object* copy = new(Object);
	memcpy(copy, self, ((const struct Class*)self->class)->size);
	return copy;
}

static bool Object_equals(void* _self, void* _other) {
	return _self == _other;
}

static char* Object_tostring(void* _self) {
	struct Object* self = _self;
	struct Class* class = (struct Class*)self->class;
	GString* str = g_string_new(NULL);
	g_string_append_printf(str, "%s(%p)", class->name, self);
	char* string = g_string_free(str, FALSE);
	assert(string);
	return string;
}

static const struct Class _Object = {
	.size = sizeof(struct Object),
	.name = "Object",
	.ctor = Object_ctor,
	.dtor = Object_dtor,
	.clone = Object_clone,
	.equals = Object_equals,
	.tostring = Object_tostring
};

EXPORT const void* Object = &_Object;

EXPORT void* new(const void* _class, ...) {
	const struct Class* class = _class;
	void* p = calloc(1, class->size);
	assert(p);
	*(const struct Class**) p = class;
	if (class->ctor) {
		va_list ap;
		va_start(ap, _class);
		p = class->ctor(p, &ap);
		va_end(ap);
	}

	return p;
}

EXPORT void delete(void* self) {
	const struct Class** cp = self;
	if (self && *cp && (*cp)->dtor) {
		self = (*cp)->dtor(self);
	}
	free(self);
}

