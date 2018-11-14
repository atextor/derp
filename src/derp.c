#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <assert.h>
#include <regex.h>

#include <sys/types.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#include "clips.h"
#include "visibility.h"
#include "derp.h"
#include "knowledgebase.h"

#define BUFSIZE 256
void* symbols[BUFSIZE];

struct DerpKnowledgeBase* knowledgebase;

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

struct DerpKnowledgeBase* derp_get_default_knowledgebase() {
	return knowledgebase;
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
		if (plugin) {
			g_hash_table_insert(result, plugin->name, plugin);
		}
	}

	return result;
}

void shutdown() {
	// Shut down plugins
	if (plugins) {
		struct DerpPlugin* p;
		GList* plugin_list = g_hash_table_get_values(plugins);
		for (GList* node = plugin_list; node; node = node->next) {
			p = (struct DerpPlugin*)node->data;
			derp_log(DERP_LOG_DEBUG, "Stopping plugin: %s", p->name);
			if (p->shutdown_plugin) {
				p->shutdown_plugin();
			}

			delete(p);
		}

		g_list_free(plugin_list);
		g_hash_table_unref(plugins);
	}

	// Shut down CLIPS environment
	delete(knowledgebase);
}

void rule_callback(void) {
	ClipsEnvironment clips_environment = knowledgebase->clips_environment;

	int argc = RtnArgCount(clips_environment);

	char* plugin_name = RtnLexeme(clips_environment, 1);
	char* rule_name = RtnLexeme(clips_environment, 2);

	struct DerpPlugin* p = g_hash_table_lookup(plugins, plugin_name);
	if (!p) {
		derp_log(DERP_LOG_ERROR, "Can't callback plugin %s: Unknown plugin", plugin_name);
		return;
	}

	if (!p->callback) {
		derp_log(DERP_LOG_ERROR, "Can't callback plugin %s: No callback function", plugin_name);
		return;
	}

	// Construct argument list from callback arguments if there are any
	if (argc > 2) {
		GHashTable* arguments = g_hash_table_new(g_str_hash, g_str_equal);
		for (int i = 3; i <= argc; i += 2) {
			g_hash_table_insert(arguments, RtnLexeme(clips_environment, i),
								RtnLexeme(clips_environment, i + 1));
		}

		p->callback(rule_name, arguments);
		g_hash_table_unref(arguments);
	} else {
		p->callback(rule_name, NULL);
	}
	return;
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

	DerpKnowledgeBase_init();
	knowledgebase = new(DerpKnowledgeBase, rule_callback);

	// Load Plugins
	GSList_String* list = NULL;
	list = g_slist_append(list, "./libplugin1.so");
	list = g_slist_append(list, "./libraptor.so");
	list = g_slist_append(list, "./librunexternal.so");
	plugins = load_plugins(list);
	g_slist_free(list);

	struct DerpPlugin* p;
	GList* plugin_list = g_hash_table_get_values(plugins);
	for (GList* node = plugin_list; node; node = node->next) {
		p = (struct DerpPlugin*)node->data;
		derp_log(DERP_LOG_DEBUG, "Starting plugin: %s", p->name);
		p->start_plugin(p);
	}
	g_list_free(plugin_list);

	// Advise raptor plugin to load rdf file
	derp_assert_triple(knowledgebase, "derp:raptor", "derp:raptor_load_file", "\"dcterms.rdf\"");

	// Test runexternal plugin
	derp_assert_triple(knowledgebase, "derp:runexternal", "derp:runexternal_command", "\"/bin/ls .\"");

	// Enter main program
	derp_log(DERP_LOG_DEBUG, "Initialized");

	// Run rule engine
	derp_assert_generic(knowledgebase, "(run)");

	// List facts
	/*
	GSList_String* facts = derp_get_facts();
	for (GSList_String* node = facts; node; node = node->next) {
		printf("fact %s", (char*)node->data);
	}
	g_slist_free_full(facts, free);
	*/

	shutdown();
	return 0;
}

