#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <glib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#include "clips.h"
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

EXPORT void derp_free_data(gpointer data) {
	free(data);
}

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

DerpPlugin* load_plugin(gchar* filename) {
	DerpPlugin*(*derp_init_plugin)(void);
	DerpPlugin* plugin = NULL;
	gchar* error = NULL;
	void* handle;

	handle = dlopen(filename, RTLD_LAZY);
	if (!handle) {
		derp_log(DERP_LOG_ERROR, "%s", dlerror());
		return NULL;
	}

	dlerror();

	derp_init_plugin = (DerpPlugin*(*)(void))dlsym(handle, "derp_init_plugin");
	error = dlerror();
	if (error != NULL) {
		derp_log(DERP_LOG_ERROR, "Error while loading plugin %s: %s", filename, error);
		return NULL;
	}

	plugin = derp_init_plugin();
	if (plugin == NULL) {
		derp_log(DERP_LOG_ERROR, "Error while loading plugin %s: Invalid plugin struct", filename);
	}

	return plugin;
}

GHashTable* load_plugins(GSList_String* plugins) {
	GHashTable* result = g_hash_table_new(
			g_str_hash,        // hash function
			g_str_equal);      // comparator

	gchar* plugin_filename;
	DerpPlugin* plugin = NULL;

	for (GSList_String* node = plugins; node; node = node->next) {
		plugin_filename = (gchar*)node->data;
		plugin = load_plugin(plugin_filename);
		if (plugin != NULL) {
			g_hash_table_insert(result, plugin->name, plugin);
		}
	}

	return result;
}

EXPORT gboolean derp_assert_generic(GString* input) {
	return RouteCommand(theEnv, input->str, FALSE);
}

EXPORT gboolean derp_assert_fact(GString* fact) {
	void* result = AssertString(fact->str);
	return (result != NULL);
}

EXPORT gboolean derp_assert_triple(GString* subject, GString* predicate, GString* object) {
	GString* fact = g_string_sized_new(100);
	g_string_append_printf(fact, "(triple %s %s %s)",
			subject->str, predicate->str, object->str);
	int result = derp_assert_fact(fact);
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

EXPORT GSList_String* derp_get_rule_definition(GString* rulename) {
	// TODO
	return NULL;
}

EXPORT gboolean derp_assert_rule(DerpRule* rule) {
	if (g_slist_length(rule->head) == 0 ||
			g_slist_length(rule->body) == 0 ||
			rule->name == NULL) {
		return FALSE;
	}

	GString* rule_assertion = g_string_sized_new(256);
	g_string_append_printf(rule_assertion, "(defrule %s ", rule->name->str);
	DerpTriple* triple;
	for (GSList_DerpTriple* h = rule->head; h; h = h->next) {
		triple = (DerpTriple*)h->data;
		g_string_append_printf(rule_assertion, "(triple %s %s %s)",
				triple->subject, triple->predicate, triple->object);
	}
	g_string_append(rule_assertion, " => (assert ");
	for (GSList_DerpTriple* h = rule->body; h; h = h->next) {
		triple = (DerpTriple*)h->data;
		g_string_append_printf(rule_assertion, "(triple %s %s %s)",
				triple->subject, triple->predicate, triple->object);
	}
	g_string_append(rule_assertion, "))");

	printf("Asserting rule: %s\n", rule_assertion->str);
	int result = derp_assert_generic(rule_assertion);
	g_string_free(rule_assertion, TRUE);

	derp_delete_rule(rule);
	return result;
}

EXPORT DerpTriple* derp_new_triple(gchar* subject, gchar* predicate, gchar* object) {
	DerpTriple* t = malloc(sizeof(DerpTriple));
	t->subject = strdup(subject);
	t->predicate = strdup(predicate);
	t->object = strdup(object);
	return t;
}

EXPORT void derp_delete_triple(DerpTriple* t) {
	free(t->subject);
	free(t->predicate);
	free(t->object);
	free(t);
}

EXPORT GSList_DerpTriple* derp_new_triple_list(DerpTriple* triple, ...) {
	GSList_DerpTriple* list = NULL;
	GSList_DerpTriple* listptr = NULL;
	list = g_slist_append(list, triple);
	listptr = list;

	va_list ap;
	va_start(ap, triple);
	DerpTriple* t = NULL;
	while(TRUE) {
		t = va_arg(ap, DerpTriple*);
		if (t == NULL) {
			break;
		}
		listptr = g_slist_append(listptr, t);
	};
	va_end(ap);
	return list;
}

EXPORT void derp_delete_triple_list(GSList_DerpTriple* list) {
	g_slist_free_full(list, (GDestroyNotify)derp_delete_triple);
}

EXPORT DerpRule* derp_new_rule(GString* name, GSList_DerpTriple* head, GSList_DerpTriple* body) {
	DerpRule* rule = malloc(sizeof(DerpRule));
	rule->name = name;
	rule->head = head;
	rule->body = body;
	return rule;
}

EXPORT void derp_delete_rule(DerpRule* rule) {
	g_string_free(rule->name, TRUE);
	derp_delete_triple_list(rule->head);
	derp_delete_triple_list(rule->body);
	free(rule);
}

EXPORT gboolean derp_add_callback(DerpPlugin* callee, gchar* name, GSList_DerpTriple* head) {
	GString* assertion = g_string_sized_new(256);
	g_string_append_printf(assertion, "(defrule %s ", name);
	DerpTriple* triple;
	for (GSList_DerpTriple* h = head; h; h = h->next) {
		triple = (DerpTriple*)h->data;
		g_string_append_printf(assertion, "(triple %s %s %s)",
				triple->subject, triple->predicate, triple->object);
	}
	g_string_append_printf(assertion, " => (rule_callback \"%s\" \"%s\"))", callee->name, name);

	int result = derp_assert_generic(assertion);
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
		DerpPlugin* p;
		GList* plugin_list = g_hash_table_get_values(plugins);
		for (GList* node = plugin_list; node; node = node->next) {
			p = (DerpPlugin*)node->data;
			derp_log(DERP_LOG_DEBUG, "Stopping plugin: %s", p->name);
			if (p->shutdown_plugin != NULL) {
				p->shutdown_plugin();
			}
		}

		g_list_free(plugin_list);
		g_hash_table_unref(plugins);
	}

	// Shut down CLIPS environment
	if (theEnv != NULL) {
		DestroyEnvironment(theEnv);
	}
}

int* rule_callback(void* arg) {
	int argc = RtnArgCount();
	if (argc != 2) {
		derp_log(DERP_LOG_ERROR, "Wrong number of callback arguments: %d", argc);
		return NULL;
	}

	char* plugin_name = RtnLexeme(1);
	char* rule_name = RtnLexeme(2);
	DerpPlugin* p = g_hash_table_lookup(plugins, plugin_name);
	if (p == NULL) {
		derp_log(DERP_LOG_ERROR, "Can't callback plugin %s: Unknown plugin", plugin_name);
		return NULL;
	}

	if (p->callback == NULL) {
		derp_log(DERP_LOG_ERROR, "Can't callback plugin %s: No callback function", plugin_name);
		return NULL;
	}

	p->callback(rule_name);
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
	sigaction(SIGSEGV, &sa, NULL);

	// Initialize rule engine
	theEnv = CreateEnvironment();

	// Register callback function.
	// Arguments:
	// 1 - the CLIPS environment
	// 2 - the name of the function within CLIPS rules
	// 3 - return type within rules (v = void, see page 20 of CLIPS Advanced Programming Guide)
	// 4 - function pointer to implementation, PTIEF = (int (*)(void *))
	// 5 - name of the implementation function as string
	// 6 - argument restrictions (22s = exactly two strings, see page 22 of CLIPS Advanced Programming Guide)
	EnvDefineFunction2(theEnv, "rule_callback", 'v', PTIEF rule_callback, "rule_callback", "22s");

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
	list = g_slist_append(list, strdup("./libplugin1.so"));
	list = g_slist_append(list, strdup("./libraptor.so"));
	plugins = load_plugins(list);
	g_slist_free_full(list, derp_free_data);

	DerpPlugin* p;
	GList* plugin_list = g_hash_table_get_values(plugins);
	for (GList* node = plugin_list; node; node = node->next) {
		p = (DerpPlugin*)node->data;
		derp_log(DERP_LOG_DEBUG, "Starting plugin: %s", p->name);
		p->start_plugin();
	}
	g_list_free(plugin_list);

	// Enter main program
	derp_log(DERP_LOG_DEBUG, "Initialized");

	// Run rule engine
	GString* run = g_string_new("(run)");
	derp_assert_generic(run);
	g_string_free(run, TRUE);

	/*
	// List facts
	GSList_String* facts = derp_get_facts();
	for (GSList_String* node = facts; node; node = node->next) {
		printf("fact %s", (char*)node->data);
	}
	g_slist_free_full(facts, derp_free_data);
	*/

	shutdown();
	return 0;
}

