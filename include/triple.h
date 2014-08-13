#ifndef _TRIPLE_H_
#define _TRIPLE_H_

#include <glib.h>

typedef GSList GSList_DerpTriple;

struct DerpTriple {
	const void* class;
	gchar* subject;
	gchar* predicate;
	gchar* object;
};

extern const void* DerpTriple;

struct DerpTripleWithFilter {
	const struct DerpTriple _;
	gchar* filter;
};

extern const void* DerpTripleWithFilter;

#endif
