#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <raptor2.h>
#include "derp.h"

static GHashTable* prefix_map;

// For an URI such as http://foo.bar/baz returns the qname, if the prefix is
// known, for example bar:baz. If the prefix is unknown, the result is the RDF
// form of the URI, e.g. <http://foo.bar/baz>.
static GString* uri_to_qname(gchar* uri) {
	GString* qname = g_string_sized_new(128);
	gchar* separator = rindex(uri, '#');
	if (separator == NULL) {
		separator = rindex(uri, '/');
	}
	if (separator == NULL) {
		g_string_append_printf(qname, "<%s>", uri);
		return qname;
	}

	GString* base_uri = g_string_sized_new(separator - uri);
	g_string_append_printf(base_uri, "%.*s", (int)(separator - uri), uri);
	gchar* prefix = g_hash_table_lookup(prefix_map, base_uri->str);
	g_string_free(base_uri, TRUE);

	gchar* suffix = separator + 1;
	if (prefix != NULL && strlen(suffix) > 0) {
		g_string_append_printf(qname, "%s:%s", prefix, suffix);
	} else {
		g_string_append_printf(qname, "<%s>", uri);
	}

	return qname;
}

static GString* sanitize_uri(GString* input) {
	GString* result = g_string_sized_new(input->len);
	gchar* str = input->str;
	char c;
	while (*str) {
		c = *str == '(' || *str == ')' ? '_' : *str;
		g_string_append_c(result, c);
		str++;
	}

	g_string_free(input, TRUE);
	return result;
}

static GString* sanitize_literal(gchar* input) {
	GString* result = g_string_sized_new(strlen(input));
	g_string_append_c(result, '"');
	while(*input) {
		if (*input == '\\') {
			g_string_append_c(result, ' ');
			input++;
		} else {
			g_string_append_c(result, *input);
		}
		input++;
	}
	g_string_append_c(result, '"');

	return result;
}

static GString* term_to_readable(raptor_term* term) {
	GString* readable;
	switch (term->type) {
		case RAPTOR_TERM_TYPE_URI:
			readable = sanitize_uri(uri_to_qname((gchar*)raptor_uri_as_string(term->value.uri)));
			break;
		case RAPTOR_TERM_TYPE_LITERAL:
			readable = sanitize_literal((gchar*)(term->value.literal.string));
			break;
		case RAPTOR_TERM_TYPE_BLANK: {
			unsigned char* t = raptor_term_to_string(term);
			readable = g_string_sized_new(256);
			g_string_append_printf(readable, "%s", t);
			free(t);
			break;
									 }
		default:
			derp_log(DERP_LOG_WARNING, "Unknown thing in RDF term: %s", (char*)raptor_term_to_string(term));
			break;
	}

	return readable;
}

static void handle_triple(void* user_data, raptor_statement* triple) {
	GString* s = term_to_readable(triple->subject);
	GString* p = term_to_readable(triple->predicate);
	GString* o = term_to_readable(triple->object);
	derp_assert_triple(s->str, p->str, o->str);
	g_string_free(s, TRUE);
	g_string_free(p, TRUE);
	g_string_free(o, TRUE);
}

void start_plugin(struct DerpPlugin* self) {
	prefix_map = g_hash_table_new_full(
			g_str_hash,    // hash function
			g_str_equal,   // comparator
			free,          // key destructor
			free);         // val destructor

	// Fill prefix map
	g_hash_table_insert(prefix_map, strdup("http://purl.org/dc/terms"), strdup("dc"));
	g_hash_table_insert(prefix_map, strdup("http://www.w3.org/2000/01/rdf-schema"), strdup("rdfs"));

	// Load rdf file
	raptor_world* world = NULL;
	raptor_parser* rdf_parser = NULL;
	unsigned char* uri_string;
	raptor_uri* uri;
	raptor_uri* base_uri;

	world = raptor_new_world();
	rdf_parser = raptor_new_parser(world, "rdfxml");
	raptor_parser_set_statement_handler(rdf_parser, NULL, handle_triple);
	uri_string = raptor_uri_filename_to_uri_string("dcterms.rdf");
	uri = raptor_new_uri(world, uri_string);
	base_uri = raptor_uri_copy(uri);
	derp_log(DERP_LOG_INFO, "Loading RDF from %s", uri_string);
	raptor_parser_parse_file(rdf_parser, uri, base_uri);

	raptor_free_parser(rdf_parser);
	raptor_free_uri(base_uri);
	raptor_free_uri(uri);
	raptor_free_memory(uri_string);
	raptor_free_world(world);

	derp_log(DERP_LOG_INFO, "%d facts loaded", derp_get_facts_size());
}

void shutdown_plugin() {
	g_hash_table_destroy(prefix_map);
}

static DerpPluginDescriptor plugin = {
	"Raptor",
	start_plugin,
	shutdown_plugin,
	NULL
};

DerpPluginDescriptor* derp_init_plugin(void) {
	return &plugin;
}

