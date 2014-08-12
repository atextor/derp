#ifndef _DERP_H_
#define _DERP_H_

#include <glib.h>
#include "oo.h"
#include "plugin.h"
#include "triple.h"
#include "rule.h"

// Typedefs for code readability
typedef GSList GSList_String;

// Derp types
typedef enum {
	DERP_LOG_WARNING,
	DERP_LOG_ERROR,
	DERP_LOG_INFO,
	DERP_LOG_DEBUG
} derp_log_level;

// Plugin Descriptor is not a class
struct _DerpPluginDescriptor {
	gchar* name;
	void (*start_plugin)(void);
	void (*shutdown_plugin)(void);
	void (*callback)(gchar* rule, GHashTable* arguments);
};

typedef struct _DerpPluginDescriptor DerpPluginDescriptor;

// Derp functions
void derp_free_data(gpointer data);
gboolean derp_assert_fact(gchar* fact);
gboolean derp_assert_generic(gchar* input);
gboolean derp_assert_triple(gchar* subject, gchar* predicate, gchar* object);
gboolean derp_assert_rule(struct DerpRule* rule);
gboolean derp_add_callback(struct DerpPlugin* callee, gchar* name, GSList_DerpTriple* head);
int derp_get_facts_size();
GSList_String* derp_get_facts();
GSList_String* derp_get_rules();
GSList_String* derp_get_rule_definition(gchar* rulename);
GSList_DerpTriple* derp_new_triple_list(struct DerpTriple* triple, ...);
void derp_delete_triple_list(GSList_DerpTriple* list);
void derp_log(derp_log_level level, char* fmt, ...);

// Rule definition macros
#define IF(...) derp_new_triple_list(__VA_ARGS__, NULL)
#define T(s,p,o) new(DerpTriple,s,p,o,NULL)
#define TF(s,p,o,f) derp_new_triple(s,p,o,p)
#define THEN(...) derp_new_triple_list(__VA_ARGS__, NULL)
#define ADD_RULE(a,b,c) derp_assert_rule(new(DerpRule,a,b,c))


#endif

