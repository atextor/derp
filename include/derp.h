#ifndef _DERP_H_
#define _DERP_H_

#include <glib.h>

#if defined _WIN32 || defined __CYGWIN__
  #define EXPORT __declspec(dllexport)
#else
  #if __GNUC__ >= 4
    #define EXPORT __attribute__ ((visibility ("default")))
  #else
    #define EXPORT
  #endif
#endif

// Typedefs for code readability
typedef GSList GSList_DerpPlugin;
typedef GSList GSList_String;
typedef GSList GSList_DerpTriple;

// Derp types
typedef enum {
	DERP_LOG_WARNING,
	DERP_LOG_ERROR,
	DERP_LOG_INFO,
	DERP_LOG_DEBUG
} derp_log_level;

struct _DerpPlugin {
	gchar* name;
	void (*start_plugin)(void);
	void (*shutdown_plugin)(void);
	void (*callback)(gchar* rule);
};

typedef struct _DerpPlugin DerpPlugin;

struct _DerpTriple {
	gchar* subject;
	gchar* predicate;
	gchar* object;
};

typedef struct _DerpTriple DerpTriple;

struct _DerpRule {
	GString* name;
	GSList_DerpTriple* head;
	GSList_DerpTriple* body;
};

typedef struct _DerpRule DerpRule;

// Derp functions
void derp_free_data(gpointer data);
gboolean derp_assert_fact(GString* fact);
gboolean derp_assert_generic(GString* input);
gboolean derp_assert_triple(GString* subject, GString* predicate, GString* object);
gboolean derp_assert_rule(DerpRule* rule);
gboolean derp_add_callback(DerpPlugin* callee, gchar* name, GSList_DerpTriple* head);
int derp_get_facts_size();
GSList_String* derp_get_facts();
GSList_String* derp_get_rules();
GSList_String* derp_get_rule_definition(GString* rulename);
DerpTriple* derp_new_triple(gchar* subject, gchar* predicate, gchar* object);
void derp_delete_triple(DerpTriple* t);
GSList_DerpTriple* derp_new_triple_list(DerpTriple* triple, ...);
void derp_delete_triple_list(GSList_DerpTriple* list);
DerpRule* derp_new_rule(GString* name, GSList_DerpTriple* head, GSList_DerpTriple* body);
void derp_delete_rule(DerpRule* rule);
void derp_log(derp_log_level level, char* fmt, ...);

// Rule definition macros
#define IF(...) derp_new_triple_list(__VA_ARGS__, NULL)
#define T(...) derp_new_triple(__VA_ARGS__)
#define THEN(...) derp_new_triple_list(__VA_ARGS__, NULL)
#define ADD_RULE(a,b,c) derp_assert_rule(derp_new_rule(g_string_new(a),b,c))


#endif

