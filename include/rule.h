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

// Rule definition macros
#define RULE(name,head,body) new(DerpRule,name,head,body)
#define IF(...) derp_new_head_list(__VA_ARGS__, NULL)
#define THEN(...) derp_new_body_list(__VA_ARGS__, NULL)
#define T(s,p,o) new(DerpTriple,s,p,o)
#define TF(s,p,o,f) new(DerpTripleWithFilter,s,p,o,f)
#define ASSERT(s,p,o) new(DerpAssertion,new(DerpTriple,s,p,o))
#define RETRACT(s,p,o) new(DerpRetraction,new(DerpTriple,s,p,o))
#define CALLBACK(callee,...) new(DerpAction,callee,__VA_ARGS__,NULL)
#define ADD_RULE(name,head,body) derp_assert_rule(RULE(name,head,body))

#endif
