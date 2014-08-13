#ifndef _RULE_H_
#define _RULE_H_

#include <glib.h>
#include "oo.h"
#include "triple.h"

typedef GSList DerpRule_HeadList;
typedef GSList DerpRule_BodyList;

struct DerpRule {
	const struct Object _;
	gchar* name;
	DerpRule_HeadList* head;
	DerpRule_BodyList* body;
};

extern const void* DerpRule;

DerpRule_HeadList* derp_new_head_list(void* item, ...);
DerpRule_BodyList* derp_new_body_list(void* item, ...);


#endif
