#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

static const struct Class _DerpTriple = {
	.super = NULL,
	.size = sizeof(struct DerpTriple),
	.ctor = DerpTriple_ctor,
	.dtor = DerpTriple_dtor,
	.clone = NULL,
	.equals = NULL
};

EXPORT const void* DerpTriple = &_DerpTriple;

