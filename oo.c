#include <stdlib.h>
#include <assert.h>
#include "oo.h"
#include "visibility.h"

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

EXPORT bool equals(const void* self, const void* other) {
	const struct Class* const* cp = self;
	assert(self && *cp && (*cp)->equals);
	return (*cp)->equals(self, other);
}
