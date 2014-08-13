#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>
#include "oo.h"
#include "visibility.h"
#include "triple.h"

static void* DerpTriple_ctor(void* _self, va_list* app) {
	struct DerpTriple* self = _self;
	const char* s = va_arg(*app, const char*);
	const char* p = va_arg(*app, const char*);
	const char* o = va_arg(*app, const char*);
	self->subject = malloc(strlen(s) + 1);
	assert(self->subject);
	self->predicate = malloc(strlen(p) + 1);
	assert(self->predicate);
	self->object = malloc(strlen(o) + 1);
	assert(self->object);
	strcpy(self->subject, s);
	strcpy(self->predicate, p);
	strcpy(self->object, o);

	return self;
}

static void* DerpTriple_dtor(void* _self) {
	struct DerpTriple* self = _self;
	free(self->subject);
	free(self->predicate);
	free(self->object);
	return self;
}

static char* DerpTriple_tostring(void* _self) {
	struct DerpTriple* self = _self;
	GString* string = g_string_new(NULL);
	g_string_append_printf(string, "DerpTriple(%s, %s, %s)",
		self->subject, self->predicate, self->object);
	gchar* result = g_string_free(string, FALSE);

	return result;
}

static const struct Class _DerpTriple = {
	.size = sizeof(struct DerpTriple),
	.ctor = DerpTriple_ctor,
	.dtor = DerpTriple_dtor,
	.clone = NULL,
	.equals = NULL,
	.tostring = DerpTriple_tostring
};

EXPORT const void* DerpTriple = &_DerpTriple;

static void* DerpTripleWithFilter_ctor(void* _self, va_list* app) {
	struct DerpTripleWithFilter* self = ((const struct Class*)DerpTriple)->ctor(_self, app);
	const char* filter = va_arg(*app, const char*);
	self ->filter = malloc(strlen(filter) + 1);
	assert(self->filter);
	strcpy(self->filter, filter);

	return self;
}

static void* DerpTripleWithFilter_dtor(void* _self) {
	struct DerpTripleWithFilter* self = _self;
	free(self->filter);
	return ((const struct Class*)DerpTriple)->dtor(_self);
}

static const struct Class _DerpTripleWithFilter = {
	.size = sizeof(struct DerpTripleWithFilter),
	.ctor = DerpTripleWithFilter_ctor,
	.dtor = DerpTripleWithFilter_dtor,
	.clone = NULL,
	.equals = NULL
};

EXPORT const void* DerpTripleWithFilter = &_DerpTripleWithFilter;

