#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oo.h"
#include "visibility.h"
#include "rule.h"

static void* DerpRule_ctor(void* _self, va_list* app) {
	struct DerpRule* self = _self;
	const char* name = va_arg(*app, const char*);
	self->name = malloc(strlen(name) + 1);
	assert(self->name);
	strcpy(self->name, name);
	GSList_DerpTriple* head = va_arg(*app, GSList_DerpTriple*);
	self->head = head;
	GSList_DerpTriple* body = va_arg(*app, GSList_DerpTriple*);
	self->body = body;

	return self;
}

static void* DerpRule_dtor(void* _self) {
	struct DerpRule* self = _self;
	free(self->name);

	struct DerpTriple* t;
	for (GSList* node = self->head; node; node = node->next) {
		t = (struct DerpTriple*)node->data;
		delete(t);
	}
	g_slist_free(self->head);

	for (GSList* node = self->body; node; node = node->next) {
		t = (struct DerpTriple*)node->data;
		delete(t);
	}
	g_slist_free(self->body);

	return self;
}

static const struct Class _DerpRule = {
	.super = NULL,
	.size = sizeof(struct DerpRule),
	.ctor = DerpRule_ctor,
	.dtor = DerpRule_dtor,
	.clone = NULL,
	.equals = NULL
};

EXPORT const void* DerpRule = &_DerpRule;

