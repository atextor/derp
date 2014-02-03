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

DerpPlugin* load_plugin(char* filename) {
	DerpPlugin*(*derp_init_plugin)(void);
	DerpPlugin* plugin = NULL;
	char* error = NULL;
	void* handle;

	handle = dlopen(filename, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		return NULL;
	}

	dlerror();

	derp_init_plugin = (DerpPlugin*(*)(void))dlsym(handle, "derp_init_plugin");
	error = dlerror();
	if (error != NULL) {
		fprintf(stderr, "Error while loading plugin %s: %s\n", filename, error);
		return NULL;
	}

	plugin = derp_init_plugin();
	if (plugin == NULL) {
		fprintf(stderr, "Error while loading plugin %s: Invalid plugin struct\n", filename);
	}

	return plugin;
}

GSList* load_plugins(GSList* plugins) {
	GSList* node;
	GSList* result = NULL;
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

bool derp_assert_fact(char* fact) {
	void* result = AssertString(fact);
	return (result != NULL);
}

int derp_get_facts_size() {
	DATA_OBJECT fact_list;
	GetFactList(&fact_list, NULL);
	return GetpDOLength(&fact_list);
}

GSList* derp_get_facts() {
	GSList* list = NULL;
	DATA_OBJECT fact_list;

	GetFactList(&fact_list, NULL);

	int start = GetpDOBegin(&fact_list);
	int end = GetpDOEnd(&fact_list);

	void* multi_field_ptr = GetValue(fact_list);
	void* fact_pointer;
	for (int i = start; i <= end; i++) {
		if (GetMFType(multi_field_ptr, i) != FACT_ADDRESS) {
			fprintf(stderr, "Unexpected multi field type\n");
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

GSList* derp_get_rules() {
	GSList* list = NULL;
	DATA_OBJECT rule_list;

	GetDefruleList(&rule_list, NULL);
	printf("get rules: %ld\n", GetpDOLength(&rule_list));

	int start = GetpDOBegin(&rule_list);
	int end = GetpDOEnd(&rule_list);

	void* multi_field_ptr = GetValue(rule_list);
	void* rule_pointer;
	for (int i = start; i <= end; i++) {
		rule_pointer = GetMFValue(multi_field_ptr, i);
		printf("rule pointer: %p\n", rule_pointer);
		//char* rule_string = GetDefrulePPForm(rule_pointer);
		char* rule_string = GetDefruleName(rule_pointer);
		list = g_slist_append(list, rule_string);
	}

	return list;
}

GSList* derp_get_rule_definition(char* rulename) {
	// TODO
	return NULL;
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
	InitializeEnvironment();
	Load("init.clp");

	// Add I/O routers
	int result = AddRouter(ROUTER_NAME, 0,
			router_query_function,
			router_print_function,
			router_getc_function,
			router_ungetc_function,
			router_exit_function);
	if (result == 0) {
		fprintf(stderr, "I/O Router %s could not be created\n", ROUTER_NAME);
		exit(EXIT_FAILURE);
	}

	// Load Plugins
	GSList* list = NULL;
	list = g_slist_append(list, "./libplugin1.so");
	GSList* plugins = load_plugins(list);

	GSList *node;
	DerpPlugin* p;
	for (int i = 0; (node = g_slist_nth(plugins, i)); i++) {
		p = (DerpPlugin*)node->data;
		printf("Creating plugin: %s\n", p->name);
		p->create_plugin();
	}

	// Enter main program
	printf("Initialized\n");

	/*
	// Test functions
	derp_assert_fact("(example (x 3) (y red) (z 1.5 b))");
	GSList* facts = derp_get_facts();
	for (int i = 0; (node = g_slist_nth(facts, i)); i++) {
		printf("Fact: %s\n", (char*)node->data);
	}
	*/

	return 0;
}

