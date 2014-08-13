#ifndef _RETRACTION_H_
#define _RETRACTION_H_

#include "oo.h"
#include "triple.h"

struct DerpRetraction {
	const struct Class* class;
	struct DerpTriple* triple;
};

extern const void* DerpRetraction;

#endif


