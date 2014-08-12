#ifndef _RULE_H_
#define _RULE_H_

#include <glib.h>
#include "triple.h"

struct DerpRule {
	const void* class;
	gchar* name;
	GSList_DerpTriple* head;
	GSList_DerpTriple* body;
};

extern const void* DerpRule;

#endif
