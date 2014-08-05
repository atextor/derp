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

typedef enum {
	DERP_LOG_WARNING,
	DERP_LOG_ERROR,
	DERP_LOG_INFO,
	DERP_LOG_DEBUG
} derp_log_level;

struct _DerpPlugin {
	char* name;
	void (*start_plugin)(void);
	void (*shutdown_plugin)(void);
};

typedef struct _DerpPlugin DerpPlugin;

struct _DerpTriple {
	char* subject;
	char* predicate;
	char* object;
};

typedef struct _DerpTriple DerpTriple;

typedef GSList GSList_DerpPlugin;
typedef GSList GSList_String;
typedef GSList GSList_DerpTriple;

void derp_free_data(gpointer data);
gboolean derp_assert_fact(char* fact);
gboolean derp_assert_generic(char* input);
gboolean derp_assert_triple(char* subject, char* predicate, char* object);
gboolean derp_add_rule(char* name, GSList_DerpTriple* head, GSList_DerpTriple* body);
int derp_get_facts_size();
GSList_String* derp_get_facts();
GSList_String* derp_get_rules();
GSList_String* derp_get_rule_definition(char* rulename);
void derp_log(derp_log_level level, char* fmt, ...);

#endif

