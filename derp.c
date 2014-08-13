#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#include "clips.h"
#include "visibility.h"
#include "derp.h"

#define BUFSIZE 256
void* symbols[BUFSIZE];

#define ROUTER_NAME "derp_router"
#define ROUTER_BUFFER_SIZE 1024
static char router_buffer[ROUTER_BUFFER_SIZE];
static int router_buffer_filled = 0;
static void router_buffer_clear();
static void *theEnv = NULL;

// Maps plugin name to plugin struct
GHashTable* plugins = NULL;

EXPORT void derp_log(derp_log_level level, char* fmt, ...) {
	switch(level) {
		case DERP_LOG_WARNING:
			printf("WARN: ");
			break;
		case DERP_LOG_ERROR:
			printf("ERROR: ");
			break;
		case DERP_LOG_INFO:
			printf("INFO: ");
			break;
		case DERP_LOG_DEBUG:
			printf("DEBUG: ");
			break;
	}

	va_list argptr;
	va_start(argptr, fmt);
	vfprintf(stdout, fmt, argptr);
	va_end(argptr);
	putchar('\n');
}

GHashTable* load_plugins(GSList_String* plugins) {
	GHashTable* result = g_hash_table_new(
			g_str_hash,        // hash function
			g_str_equal);      // comparator

	gchar* plugin_filename;
	struct DerpPlugin* plugin = NULL;

	for (GSList_String* node = plugins; node; node = node->next) {
		plugin_filename = (gchar*)node->data;
		plugin = new(DerpPlugin, plugin_filename);
		if (plugin != NULL) {
			g_hash_table_insert(result, plugin->name, plugin);
		}
	}

	return result;
}

EXPORT gboolean derp_assert_generic(gchar* input) {
	return RouteCommand(theEnv, input, FALSE);
}

EXPORT gboolean derp_assert_fact(gchar* fact) {
	return AssertString(fact) != NULL;
}

EXPORT gboolean derp_assert_triple(gchar* subject, gchar* predicate, gchar* object) {
	GString* fact = g_string_sized_new(100);
	g_string_append_printf(fact, "(triple %s %s %s)", subject, predicate, object);
	int result = derp_assert_fact(fact->str);
	g_string_free(fact, TRUE);
	return result;
}

EXPORT int derp_get_facts_size() {
	DATA_OBJECT fact_list;
	GetFactList(&fact_list, NULL);
	return GetpDOLength(&fact_list);
}

EXPORT GSList_String* derp_get_facts() {
	GSList_String* list = NULL;
	GSList_String* pointer = NULL;
	DATA_OBJECT fact_list;

	GetFactList(&fact_list, NULL);

	int start = GetpDOBegin(&fact_list);
	int end = GetpDOEnd(&fact_list);

	void* multi_field_ptr = GetValue(fact_list);
	void* fact_pointer;
	for (int i = start; i <= end; i++) {
		if (GetMFType(multi_field_ptr, i) != FACT_ADDRESS) {
			derp_log(DERP_LOG_WARNING, "Unexpected multi field type");
			continue;
		}
		fact_pointer = GetMFValue(multi_field_ptr, i);
		router_buffer_clear();
		PPFact(fact_pointer, ROUTER_NAME, 0);
		int len = strlen(router_buffer);
		char* fact_string = malloc(len + 1);
		assert(fact_string != NULL);
		strncpy(fact_string, router_buffer, len + 1);
		pointer = g_slist_append(pointer, fact_string);
		if (list == NULL) {
			list = pointer;
		}
	}

	return list;
}

EXPORT GSList_String* derp_get_rules() {
	GSList_String* list = NULL;
	DATA_OBJECT rule_list;

	GetDefruleList(&rule_list, NULL);

	int start = GetpDOBegin(&rule_list);
	int end = GetpDOEnd(&rule_list);

	void* multi_field_ptr = GetValue(rule_list);
	void* rule_pointer;
	for (int i = start; i <= end; i++) {
		// Get rule symbol
		rule_pointer = GetMFValue(multi_field_ptr, i);
		char* rule_name = ValueToString(rule_pointer);
		char* new_rule_pointer = FindDefrule(rule_name);
		char* rule_string = GetDefrulePPForm(new_rule_pointer);
		list = g_slist_append(list, rule_string);
	}

	return list;
}

EXPORT GSList_String* derp_get_rule_definition(gchar* rulename) {
	// TODO
	return NULL;
}

EXPORT gboolean derp_assert_rule(struct DerpRule* rule) {
	if (g_slist_length(rule->head) == 0 ||
			g_slist_length(rule->body) == 0 ||
			rule->name == NULL) {
		return FALSE;
	}

	GString* rule_assertion = g_string_sized_new(256);
	g_string_append_printf(rule_assertion, "(defrule %s ", rule->name);
	struct DerpTriple* triple;
	for (GSList_DerpTriple* h = rule->head; h; h = h->next) {
		triple = (struct DerpTriple*)h->data;
		gchar* rule_filter = NULL;

		/*
		if (triple->filter != NULL) {
			gchar* filter_copy = strdup(triple->filter);
			gchar* sep = strchr(filter_copy, ':');
			if (sep == NULL) {
				derp_log(DERP_LOG_ERROR, "Error in triple filter: No separator found: %s", triple->filter);
			} else {
				gchar* expression = sep + 1;
				sep = '\0';
				gchar* variable = filter_copy;
				// TODO append filter expr with expression and variable to rule_filter
			}
			free(filter_copy);
		}
		*/

		g_string_append_printf(rule_assertion, "(triple %s %s %s%s)",
				triple->subject, triple->predicate, triple->object,
				rule_filter == NULL ? "" : rule_filter);
	}
	g_string_append(rule_assertion, " => (assert ");
	for (GSList_DerpTriple* h = rule->body; h; h = h->next) {
		triple = (struct DerpTriple*)h->data;
		g_string_append_printf(rule_assertion, "(triple %s %s %s)",
				triple->subject, triple->predicate, triple->object);
	}
	g_string_append(rule_assertion, "))");

	int result = derp_assert_generic(rule_assertion->str);
	g_string_free(rule_assertion, TRUE);

	delete(rule);
	return result;
}

EXPORT GSList_DerpTriple* derp_new_triple_list(struct DerpTriple* triple, ...) {
	GSList_DerpTriple* list = NULL;
	GSList_DerpTriple* listptr = NULL;
	list = g_slist_append(list, triple);
	listptr = list;

	va_list ap;
	va_start(ap, triple);
	struct DerpTriple* t = NULL;
	while(TRUE) {
		t = va_arg(ap, struct DerpTriple*);
		if (t == NULL) {
			break;
		}
		listptr = g_slist_append(listptr, t);
	};
	va_end(ap);
	return list;
}

EXPORT void derp_delete_triple_list(GSList_DerpTriple* list) {
	for (GSList* node = list; node; node = node->next) {
		delete(node);
	}
	g_slist_free(list);
}

EXPORT gboolean derp_add_callback(struct DerpPlugin* callee, gchar* name, GSList_DerpTriple* head) {
	GHashTable* variables = g_hash_table_new(g_str_hash, g_str_equal);
	GString* assertion = g_string_sized_new(256);
	g_string_append_printf(assertion, "(defrule %s ", name);
	struct DerpTriple* triple;
	for (GSList_DerpTriple* h = head; h; h = h->next) {
		triple = (struct DerpTriple*)h->data;
		g_string_append_printf(assertion, "(triple %s %s %s)",
				triple->subject, triple->predicate, triple->object);
		if (triple->subject[0] == '?') {
			g_hash_table_insert(variables, triple->subject, triple->subject);
		}
		if (triple->predicate[0] == '?') {
			g_hash_table_insert(variables, triple->predicate, triple->predicate);
		}
		if (triple->object[0] == '?') {
			g_hash_table_insert(variables, triple->object, triple->object);
		}
	}
	g_string_append_printf(assertion, " => (rule_callback \"%s\" \"%s\" ", callee->name, name);

	// Append list of variable names to assertion, so that callback function
	// can receive the bound values
	GList* variable_list = g_hash_table_get_keys(variables);
	gchar* var;
	for (GList* node = variable_list; node; node = node->next) {
			var = (gchar*)node->data;
			g_string_append_printf(assertion, "\"%s\" %s ", var + 1, var);
	}
	g_list_free(variable_list);
	g_hash_table_unref(variables);
	g_string_append(assertion, "))");

	int result = derp_assert_generic(assertion->str);
	g_string_free(assertion, TRUE);
	derp_delete_triple_list(head);

	return result;
}

static void router_buffer_clear() {
	memset(router_buffer, 0, ROUTER_BUFFER_SIZE);
	router_buffer_filled = 0;
}

int router_query_function(char* logical_name) {
	return strncmp(ROUTER_NAME, logical_name, strlen(ROUTER_NAME)) == 0;
}

int router_print_function(char* logical_name, char* str) {
	int len = strlen(str);
	snprintf(router_buffer + router_buffer_filled, ROUTER_BUFFER_SIZE - len - 1, "%s", str);
	router_buffer_filled += len;
	return 1;
}

int router_getc_function(char* logical_name) {
	return fgetc(stdin);
}

int router_ungetc_function(int ch, char* logical_name) {
	return ungetc(ch, stdout);
}

int router_exit_function(int exit_code) {
	return 0;
}

void shutdown() {
	// Shut down plugins
	if (plugins != NULL) {
		struct DerpPlugin* p;
		GList* plugin_list = g_hash_table_get_values(plugins);
		for (GList* node = plugin_list; node; node = node->next) {
			p = (struct DerpPlugin*)node->data;
			derp_log(DERP_LOG_DEBUG, "Stopping plugin: %s", p->name);
			if (p->shutdown_plugin != NULL) {
				p->shutdown_plugin();
			}

			delete(p);
		}

		g_list_free(plugin_list);
		g_hash_table_unref(plugins);
	}

	// Shut down CLIPS environment
	if (theEnv != NULL) {
		DestroyEnvironment(theEnv);
	}
}

int filter(void* arg) {
	int argc = RtnArgCount();

	char* arg1 = RtnLexeme(1);
	printf("Filter called. Argc: %d Arg[0]: %s\n", argc, arg1);

	return TRUE;
}

int* rule_callback(void* arg) {
	int argc = RtnArgCount();

	char* plugin_name = RtnLexeme(1);
	char* rule_name = RtnLexeme(2);

	struct DerpPlugin* p = g_hash_table_lookup(plugins, plugin_name);
	if (p == NULL) {
		derp_log(DERP_LOG_ERROR, "Can't callback plugin %s: Unknown plugin", plugin_name);
		return NULL;
	}

	if (p->callback == NULL) {
		derp_log(DERP_LOG_ERROR, "Can't callback plugin %s: No callback function", plugin_name);
		return NULL;
	}

	// Construct argument list from callback arguments if there are any
	if (argc > 2) {
		GHashTable* arguments = g_hash_table_new(g_str_hash, g_str_equal);
		for (int i = 3; i <= argc; i += 2) {
			g_hash_table_insert(arguments, RtnLexeme(i), RtnLexeme(i + 1));
		}

		p->callback(rule_name, arguments);
		g_hash_table_unref(arguments);
	} else {
		p->callback(rule_name, NULL);
	}
	return NULL;
}

void sighandler(int signum) {
	int numPointers;
	switch (signum) {
		case SIGSEGV:
			fprintf(stderr, "\n*** Derp: Caught SIGSEGV\n");
			fprintf(stderr, "\nBacktrace:\n");
			numPointers = backtrace(symbols, BUFSIZE);
			backtrace_symbols_fd(symbols, numPointers, STDERR_FILENO);
			exit(EXIT_FAILURE);
			break;
		case SIGTERM:
			shutdown();
			break;
		default:
			break;
	}
}

int main() {
	// Install signal handler
	struct sigaction sa;
	sigset_t sigset;
	sigemptyset(&sigset);
	sa.sa_handler = sighandler;
	sa.sa_flags = 0;
	sa.sa_mask = sigset;
	if (sigaction(SIGSEGV, &sa, NULL) == -1 ||
		sigaction(SIGTERM, &sa, NULL)) {
		derp_log(DERP_LOG_ERROR, "Could not set up signal handler");
	}

	// Initialize rule engine
	theEnv = CreateEnvironment();

	// Register callback function.
	// Arguments:
	// 1 - the CLIPS environment
	// 2 - the name of the function within CLIPS rules
	// 3 - return type within rules (v = void, see page 20 of CLIPS Advanced Programming Guide)
	// 4 - function pointer to implementation, PTIEF = (int (*)(void *))
	// 5 - name of the implementation function as string
	// 6 - argument restrictions (2*uss = exactly two strings followed by any number of any arguments,
	//     see page 22 of CLIPS Advanced Programming Guide)
	EnvDefineFunction2(theEnv, "rule_callback", 'v', PTIEF rule_callback, "rule_callback", "2*uss");


	EnvDefineFunction2(theEnv, "filter", 'b', PTIEF filter, "filter", "**u");
	derp_assert_generic("(defrule filtertest (triple FOO FOO ?o&:(filter ?o))  => (assert (triple FOO FOO FILTERED)))");


	// Add I/O routers
	int result = AddRouter(ROUTER_NAME, 0,
			router_query_function,
			router_print_function,
			router_getc_function,
			router_ungetc_function,
			router_exit_function);
	if (result == 0) {
		derp_log(DERP_LOG_ERROR, "I/O Router %s could not be created", ROUTER_NAME);
		exit(EXIT_FAILURE);
	}

	// Load Plugins
	GSList_String* list = NULL;
	list = g_slist_append(list, "./libplugin1.so");
	list = g_slist_append(list, "./libraptor.so");
	plugins = load_plugins(list);
	g_slist_free(list);

	struct DerpPlugin* p;
	GList* plugin_list = g_hash_table_get_values(plugins);
	for (GList* node = plugin_list; node; node = node->next) {
		p = (struct DerpPlugin*)node->data;
		derp_log(DERP_LOG_DEBUG, "Starting plugin: %s", p->name);
		p->start_plugin();
	}
	g_list_free(plugin_list);

	// Enter main program
	derp_log(DERP_LOG_DEBUG, "Initialized");

	// Run rule engine
	derp_assert_generic("(run)");

	// List facts
	/*
	GSList_String* facts = derp_get_facts();
	for (GSList_String* node = facts; node; node = node->next) {
		printf("fact %s", (char*)node->data);
	}
	g_slist_free_full(facts, derp_free_data);
	*/

	shutdown();
	return 0;
}

