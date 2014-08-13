#ifndef _OO_H_
#define _OO_H_

#include <stdbool.h>
#include <stdarg.h>

struct Class {
	size_t size;
	void* (*ctor)(void* self, va_list* app);
	void* (*dtor)(void* self);
	void* (*clone)(void* self);
	bool (*equals)(void* self, void* other);
	char* (*tostring)(void* self);
};

struct Object {
	const void* class;
};

extern const void* Object;

void* new(const void* _class, ...);
void delete(void* item);

#endif

