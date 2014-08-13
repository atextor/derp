#ifndef _RULE_H_
#define _RULE_H_

#include <glib.h>
#include "oo.h"
#include "triple.h"

struct DerpRule {
	const struct Class* class;
	gchar* name;
	GSList* head;
	GSList* body;
};

extern const void* DerpRule;

#endif
