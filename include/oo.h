#ifndef _OO_H_
#define _OO_H_

#include <stdbool.h>
#include <stdarg.h>

struct Class {
	const struct Class* super;
	size_t size;
	void* (*ctor)(void* self, va_list* app);
	void* (*dtor)(void* self);
	void* (*clone)(const void* self);
	bool (*equals)(const void* self, const void* other);
};

void* new(const void* _class, ...);
void delete(void* item);

#endif

