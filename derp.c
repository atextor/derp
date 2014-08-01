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
static void *theEnv;

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


DerpPlugin* load_plugin(char* filename) {
	DerpPlugin*(*derp_init_plugin)(void);
	DerpPlugin* plugin = NULL;
	char* error = NULL;
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

GSList_DerpPlugin* load_plugins(GSList_String* plugins) {
	GSList_String* node;
	GSList_DerpPlugin* result = NULL;
	char* plugin_filename;
	DerpPlugin* plugin = NULL;

	for (int i = 0; (node = g_slist_nth(plugins, i)); i++) {
		plugin_filename = (char*)node->data;
		plugin = load_plugin(plugin_filename);
		if (plugin != NULL) {
			result = g_slist_append(result, plugin);
		}
	}

	return result;
}

EXPORT gboolean derp_assert_generic(char* input) {
	return RouteCommand(theEnv, input, FALSE);
}


EXPORT gboolean derp_assert_fact(char* fact) {
	void* result = AssertString(fact);
	return (result != NULL);
}

EXPORT gboolean derp_assert_triple(char* subject, char* predicate, char* object) {
	// TODO make reentrant
	static char buf[2048];
	int result = snprintf(buf, 2048, "(triple (subj %s) (pred %s) (obj %s))", subject, predicate, object);
	if (result >= 2048 || result < 0) {
		return FALSE;
	}
	return derp_assert_fact(buf);
}


EXPORT int derp_get_facts_size() {
	DATA_OBJECT fact_list;
	GetFactList(&fact_list, NULL);
	return GetpDOLength(&fact_list);
}

EXPORT GSList_String* derp_get_facts() {
	GSList_String* list = NULL;
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
		list = g_slist_append(list, fact_string);
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

EXPORT GSList_String* derp_get_rule_definition(char* rulename) {
	// TODO
	return NULL;
}

EXPORT gboolean derp_add_rule(char* name, GSList_DerpTriple* head, GSList_DerpTriple* body) {
	// TODO make reentrant
	GSList_String* assertion = NULL;
	GSList_String* assertion_head = NULL;

	DerpTriple* triple;

	assertion = g_slist_append(assertion, "(defrule ");
	assertion_head = assertion;
	assertion = g_slist_append(assertion, name);
	assertion = g_slist_append(assertion, " ");

	for (GSList_DerpTriple* h = head; h; h = h->next) {
		triple = (DerpTriple*)h->data;
		assertion = g_slist_append(assertion, "(triple (subj ");
		assertion = g_slist_append(assertion, triple->subject);
		assertion = g_slist_append(assertion, ") (pred ");
		assertion = g_slist_append(assertion, triple->predicate);
		assertion = g_slist_append(assertion, ") (obj ");
		assertion = g_slist_append(assertion, triple->object);
		assertion = g_slist_append(assertion, "))");
	}

	assertion = g_slist_append(assertion, " => (assert ");

	for (GSList_DerpTriple* h = body; h; h = h->next) {
		triple = (DerpTriple*)h->data;
		assertion = g_slist_append(assertion, "(triple (subj ");
		assertion = g_slist_append(assertion, triple->subject);
		assertion = g_slist_append(assertion, ") (pred ");
		assertion = g_slist_append(assertion, triple->predicate);
		assertion = g_slist_append(assertion, ") (obj ");
		assertion = g_slist_append(assertion, triple->object);
		assertion = g_slist_append(assertion, "))");
	}

	assertion = g_slist_append(assertion, "))");

	// TODO make reentrant
	static char buf[2048];
	memset(buf, '\0', 2048);
	for (GSList_String* node = assertion_head; node; node = node->next) {
		strcat(buf, (char*)node->data);
	}
	derp_log(DERP_LOG_DEBUG, "Asserting: %s", buf);
	return derp_assert_generic(buf);
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
		default:
			break;
	}
}

int main() {
	// Install signal handler
	struct sigaction sa;
	sa.sa_handler = sighandler;
	sa.sa_flags = 0;
	sigaction(SIGSEGV, &sa, NULL);

	// Initialize rule engine
	theEnv = CreateEnvironment();
	Load("init.clp");

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
	GSList_DerpPlugin* plugins = load_plugins(list);

	DerpPlugin* p;
	for (GSList_DerpPlugin* node = plugins; node; node = node->next) {
		p = (DerpPlugin*)node->data;
		derp_log(DERP_LOG_DEBUG, "Creating plugin: %s", p->name);
		p->create_plugin();
	}

	// Enter main program
	derp_log(DERP_LOG_DEBUG, "Initialized");

	// Test functions
	/*
	derp_assert_fact("(example (x 3) (y red) (z 1.5 b))");
	*/

	derp_assert_generic("(run)");

	for (GSList_String* node = derp_get_facts(); node; node = node->next) {
		printf("Fact:\n%s\n", (char*)node->data);
	}

	return 0;
}

