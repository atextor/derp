#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <assert.h>
#include <regex.h>

#include "derp.h"
#include "knowledgebase.h"
#include "typealiases.h"
#include "clips.h"
#include "visibility.h"

#define ROUTER_NAME "derp_router"
static GHashTable* environments_map = NULL;
static GString* router_buffer = NULL;
static void router_buffer_clear();

void DerpKnowledgeBase_init() {
	environments_map = g_hash_table_new(g_direct_hash,    // hash function
										g_direct_equal);  // comparator
}

int DerpKnowledgeBase_filter(ClipsEnvironment clips_environment) {
	char* s = RtnLexeme(clips_environment, 1);
	char* p = RtnLexeme(clips_environment, 2);
	char* o = RtnLexeme(clips_environment, 3);
	char spo_selector = RtnLexeme(clips_environment, 4)[0];
	char* filter = RtnLexeme(clips_environment, 5);

	regex_t regex;
	int reti;
	char msgbuf[100];

	char* target = spo_selector == 's' ? s :
		(spo_selector == 'p' ? p :
		(spo_selector == 'o' ? o : NULL));
	if (!target) {
		return FALSE;
	}

	reti = regcomp(&regex, filter, 0);
	if (reti) {
		derp_log(DERP_LOG_ERROR, "Invalid regular expression in rule triple filter");
		return FALSE;
	}

	reti = regexec(&regex, target, 0, NULL, 0);
	if (!reti) {
		regfree(&regex);
		return TRUE;
	} else if (reti == REG_NOMATCH) {
		regfree(&regex);
		return FALSE;
	} else {
		regerror(reti, &regex, msgbuf, sizeof(msgbuf));
		derp_log(DERP_LOG_ERROR, "Regex match failed in rule triple filter");
		regfree(&regex);
		return FALSE;
	}
}

int* DerpKnowledgeBase_callback(ClipsEnvironment clips_environment) {
	struct DerpKnowledgeBase* knowledge_base = g_hash_table_lookup(environments_map, clips_environment);
	knowledge_base->callback_function();
	return NULL;
}

static void router_buffer_clear() {
	if (router_buffer) {
		g_string_free(router_buffer, TRUE);
	}
	router_buffer = g_string_new(NULL);
	assert(router_buffer);
}

int router_query_function(ClipsEnvironment clips_environment, char* logical_name) {
	return strncmp(ROUTER_NAME, logical_name, strlen(ROUTER_NAME)) == 0;
}

int router_print_function(ClipsEnvironment clips_environment, char* logical_name, char* str) {
	g_string_append(router_buffer, str);
	return 1;
}

int router_getc_function(ClipsEnvironment clips_environment, char* logical_name) {
	return fgetc(stdin);
}

int router_ungetc_function(ClipsEnvironment clips_environment, int ch, char* logical_name) {
	return ungetc(ch, stdout);
}

int router_exit_function(ClipsEnvironment clips_environment, int exit_code) {
	if (router_buffer) {
		g_string_free(router_buffer, TRUE);
	}
	return 0;
}

static void* DerpKnowledgeBase_ctor(void* _self, va_list* app) {
	struct DerpKnowledgeBase* self = _self;
	void (*callback_function)(void) = va_arg(*app, void(*)(void));
	self->callback_function = callback_function;
	assert(self->callback_function);

	// Initialize rule engine
	self->clips_environment = CreateEnvironment();

	// Save reference to knowledge base
	g_hash_table_insert(environments_map, self->clips_environment, self);

	// Register callback function.
	// Arguments:
	// 1 - the CLIPS environment
	// 2 - the name of the function within CLIPS rules
	// 3 - return type within rules (v = void, see page 20 of CLIPS Advanced Programming Guide)
	// 4 - function pointer to implementation, PTIEF = (int (*)(void *))
	// 5 - name of the implementation function as string
	// 6 - argument restrictions (2*uss = two strings followed by any number of any arguments,
	//     see page 22 of CLIPS Advanced Programming Guide)
	EnvDefineFunction2(self->clips_environment, "rule_callback", 'v',
						PTIEF DerpKnowledgeBase_callback, "DerpKnowledgeBase_callback", "2*uss");

	// Register filter function.
	// Arguments:
	// 1 - the CLIPS environment
	// 2 - the name of the function within CLIPS rules
	// 3 - return type within rules (b = boolean, see page 20 of CLIPS Advanced Programming Guide)
	// 4 - function pointer to implementation, PTIEF = (int (*)(void *))
	// 5 - name of the implementation function as string
	// 6 - argument restrictions (55uuuuss = three arguments of any type, followed by two strings,
	//     followed by one argument of any type, see page 22 of CLIPS Advanced Programming Guide)
	EnvDefineFunction2(self->clips_environment, "filter", 'b',
						PTIEF DerpKnowledgeBase_filter, "DerpKnowledgeBase_filter", "55uuuuss");

	// Add I/O routers
	int result = EnvAddRouter(self->clips_environment,
			ROUTER_NAME, 0,
			router_query_function,
			router_print_function,
			router_getc_function,
			router_ungetc_function,
			router_exit_function);
	if (result == 0) {
		derp_log(DERP_LOG_ERROR, "I/O Router %s could not be created", ROUTER_NAME);
		exit(EXIT_FAILURE);
	}
	return self;
}

static void* DerpKnowledgeBase_dtor(void* _self) {
	struct DerpKnowledgeBase* self = _self;

	if (self->clips_environment) {
		DestroyEnvironment(self->clips_environment);
	}

	return self;
}

EXPORT gboolean derp_assert_generic(struct DerpKnowledgeBase* self, gchar* input) {
	return RouteCommand(self->clips_environment, input, FALSE);
}

EXPORT gboolean derp_assert_fact(struct DerpKnowledgeBase* self, gchar* fact) {
	return AssertString(self->clips_environment, fact) != NULL;
}

EXPORT gboolean derp_assert_triple(struct DerpKnowledgeBase* self, gchar* subject, gchar* predicate, gchar* object) {
	GString* fact = g_string_new(NULL);
	g_string_append_printf(fact, "(triple %s %s %s)", subject, predicate, object);
	int result = derp_assert_fact(self, fact->str);
	g_string_free(fact, TRUE);
	return result;
}

EXPORT int derp_get_facts_size(struct DerpKnowledgeBase* self) {
	DATA_OBJECT fact_list;
	GetFactList(self->clips_environment, &fact_list, NULL);
	return GetpDOLength(&fact_list);
}

EXPORT GSList_String* derp_get_facts(struct DerpKnowledgeBase* self) {
	GSList_String* list = NULL;
	GSList_String* pointer = NULL;
	DATA_OBJECT fact_list;

	GetFactList(self->clips_environment, &fact_list, NULL);

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
		PPFact(self->clips_environment, fact_pointer, ROUTER_NAME, 0);
		gchar* fact_string = g_string_free(router_buffer, FALSE);
		router_buffer = NULL;
		pointer = g_slist_append(pointer, fact_string);
		if (!list) {
			list = pointer;
		}
	}

	return list;
}

EXPORT GSList_String* derp_get_rules(struct DerpKnowledgeBase* self) {
	GSList_String* list = NULL;
	DATA_OBJECT rule_list;

	GetDefruleList(self->clips_environment, &rule_list, NULL);

	int start = GetpDOBegin(&rule_list);
	int end = GetpDOEnd(&rule_list);

	void* multi_field_ptr = GetValue(rule_list);
	void* rule_pointer;
	for (int i = start; i <= end; i++) {
		// Get rule symbol
		rule_pointer = GetMFValue(multi_field_ptr, i);
		char* rule_name = ValueToString(rule_pointer);
		char* new_rule_pointer = FindDefrule(self->clips_environment, rule_name);
		char* rule_string = GetDefrulePPForm(self->clips_environment, new_rule_pointer);
		list = g_slist_append(list, rule_string);
	}

	return list;
}

EXPORT GSList_String* derp_get_rule_definition(struct DerpKnowledgeBase* self, gchar* rulename) {
	// TODO
	return NULL;
}

EXPORT gboolean derp_assert_rule(struct DerpKnowledgeBase* self, struct DerpRule* rule) {
	gchar* str = ((struct Class*)DerpRule)->tostring(rule);
	printf("Asserting rule: %s\n", str);
	int result = derp_assert_generic(self, str);
	free(str);
	delete(rule);
	return result;
}

static const struct Class _DerpKnowledgeBase = {
	.size = sizeof(struct DerpKnowledgeBase),
	.name = "DerpKnowledgeBase",
	.ctor = DerpKnowledgeBase_ctor,
	.dtor = DerpKnowledgeBase_dtor,
	.clone = NULL,
	.equals = NULL,
	.tostring = NULL
};

EXPORT const void* DerpKnowledgeBase = &_DerpKnowledgeBase;
