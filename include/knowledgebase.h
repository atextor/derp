#ifndef _KNOWLEDGEBASE_H_
#define _KNOWLEDGEBASE_H_

#include <glib.h>
#include "oo.h"
#include "typealiases.h"

typedef void* ClipsEnvironment;

struct DerpKnowledgeBase {
	const struct Object _;
	ClipsEnvironment clips_environment;
	void (*callback_function)(void);
};

extern const void* DerpKnowledgeBase;

void DerpKnowledgeBase_init();

gboolean derp_assert_fact(struct DerpKnowledgeBase* self, gchar* fact);
gboolean derp_assert_generic(struct DerpKnowledgeBase* self, gchar* input);
gboolean derp_assert_triple(struct DerpKnowledgeBase* self, gchar* subject, gchar* predicate, gchar* object);
gboolean derp_assert_rule(struct DerpKnowledgeBase* self, struct DerpRule* rule);
int derp_get_facts_size(struct DerpKnowledgeBase* self);
GSList_String* derp_get_facts(struct DerpKnowledgeBase* self);
GSList_String* derp_get_rules(struct DerpKnowledgeBase* self);
GSList_String* derp_get_rule_definition(struct DerpKnowledgeBase* self, gchar* rulename);

#endif

