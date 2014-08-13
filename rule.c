#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "oo.h"
#include "visibility.h"
#include "rule.h"
#include "assertion.h"
#include "retraction.h"

static void* DerpRule_ctor(void* _self, va_list* app) {
	struct DerpRule* self = _self;
	const char* name = va_arg(*app, const char*);
	self->name = malloc(strlen(name) + 1);
	assert(self->name);
	strcpy(self->name, name);
	// Skip validation of lists
	DerpRule_HeadList* head = va_arg(*app, GSList*);
	self->head = head;
	DerpRule_BodyList* body = va_arg(*app, GSList*);
	self->body = body;

	return self;
}

static void* DerpRule_dtor(void* _self) {
	struct DerpRule* self = _self;
	free(self->name);

	struct Object* o;
	for (GSList* node = self->head; node; node = node->next) {
		o = (struct Object*)node->data;
		delete(o);
	}
	g_slist_free(self->head);

	for (GSList* node = self->body; node; node = node->next) {
		o = (struct Object*)node->data;
		delete(o);
	}
	g_slist_free(self->body);

	return self;
}

static const struct Class _DerpRule = {
	.size = sizeof(struct DerpRule),
	.ctor = DerpRule_ctor,
	.dtor = DerpRule_dtor,
	.clone = NULL,
	.equals = NULL
};

EXPORT const void* DerpRule = &_DerpRule;

static bool is_valid_head_item(void* item) {
	const struct Class* class = ((struct Object*)item)->class;
	return class == DerpTriple || class == DerpTripleWithFilter;
}

static bool is_valid_body_item(void* item) {
	const struct Class* class = ((struct Object*)item)->class;
	return class == DerpAssertion || class == DerpRetraction;
}

static GSList* build_item_list(void* item, va_list ap, bool (*validator)(void*)) {
	GSList* list = NULL;
	GSList* listptr = NULL;
	if (!validator(item)) {
		return NULL;
	}
	list = g_slist_append(list, item);
	listptr = list;

	while (TRUE) {
		void* o = va_arg(ap, void*);
		if (o == NULL) {
			break;
		}

		if (!validator(o)) {
			g_slist_free(list);
			return NULL;
		}

		listptr = g_slist_append(listptr, o);
	};
	return list;
}

EXPORT DerpRule_HeadList* derp_new_head_list(void* item, ...) {
	va_list ap;
	va_start(ap, item);
	GSList* result = build_item_list(item, ap, is_valid_head_item);
	va_end(ap);
	return result;
}

EXPORT DerpRule_BodyList* derp_new_body_list(void* item, ...) {
	va_list ap;
	va_start(ap, item);
	GSList* result = build_item_list(item, ap, is_valid_body_item);
	va_end(ap);
	return result;
}

