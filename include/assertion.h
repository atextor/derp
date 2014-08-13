#ifndef _ASSERTION_H_
#define _ASSERTION_H_

#include "oo.h"
#include "triple.h"

struct DerpAssertion {
	const struct Class* class;
	struct DerpTriple* triple;
};

extern const void* DerpAssertion;

#endif

