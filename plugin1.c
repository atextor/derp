#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "derp.h"

static DerpTriple* derp_triple(char* subject, char* predicate, char* object) {
	DerpTriple* t = malloc(sizeof(DerpTriple));
	t->subject = strdup(subject);
	t->predicate = strdup(predicate);
	t->object = strdup(object);
	return t;
}

static void delete_triple(DerpTriple* t) {
	free(t->subject);
	free(t->predicate);
	free(t->object);
	free(t);
}

void start_plugin() {
	// Assert fact
	GSList_DerpTriple* head = NULL;
	DerpTriple* h1 = derp_triple("dc:NLM", "dc:modified", "\"2008-01-14\"");
	head = g_slist_append(head, h1);

	GSList_DerpTriple* body = NULL;
	DerpTriple* b1 = derp_triple("FOO", "FOO", "FOO");
	body = g_slist_append(body, b1);

	GString* name = g_string_new("foo");
	derp_add_rule(name, head, body);
	g_string_free(name, TRUE);
	delete_triple(h1);
	delete_triple(b1);
	g_slist_free(head);
	g_slist_free(body);

	// Check for fact
	GSList_String* facts = derp_get_facts();
	GSList_String* node;
	for (int i = 0; (node = g_slist_nth(facts, i)); i++) {
		printf("Fact: %s\n", (char*)node->data);
	}
	g_slist_free_full(facts, derp_free_data);

	// List rules
	GSList_String* rules = derp_get_rules();
	for (int i = 0; (node = g_slist_nth(rules, i)); i++) {
		printf("Rule: %s\n", (char*)node->data);
	}
	g_slist_free(rules);
}

static DerpPlugin plugin = {
	"Test",
	start_plugin,
	NULL
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

