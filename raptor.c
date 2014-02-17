#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <raptor2.h>
#include "derp.h"

GHashTable* prefix_map;

// For an URI such as http://foo.bar/baz returns the qname, if the prefix is
// known, for example bar:baz. If the prefix is unknown, the result is the RDF
// form of the URI, e.g. <http://foo.bar/baz>.
static char* uri_to_qname(char* uri) {
	char* base_uri;
	char* prefix;
	char* suffix;
	char* result;
	int resultlen;

	// Split URI in base URI and suffix
	char* tl;
	base_uri = malloc(strlen(uri) + 2);
	memcpy(base_uri, uri, strlen(uri) + 1);
	for (tl = base_uri + strlen(base_uri); tl != base_uri && *tl != '#' && *tl != '/'; tl--) {
		*(tl + 1) = *tl;
	}
	*(tl + 1) = '\0';
	suffix = tl + 2;
	// Lookup prefix using base URI
	prefix = g_hash_table_lookup(prefix_map, base_uri);

	if (prefix != NULL && strlen(suffix) > 0) {
		resultlen = strlen(prefix) + strlen(suffix) + 1;
		result = malloc(resultlen + 1);
		snprintf(result, resultlen + 1, "%s:%s", prefix, suffix);
	} else {
		resultlen = strlen(uri) + 2;
		result = malloc(resultlen + 1);
		snprintf(result, resultlen + 1, "<%s>", uri);
	}

	return result;
}

static char* term_to_readable(raptor_term* term) {
	unsigned char* uri;
	char* result;
	char* str;
	int resultsize;
	switch (term->type) {
		case RAPTOR_TERM_TYPE_URI:
			uri = raptor_uri_as_string(term->value.uri);
			return uri_to_qname((char*)uri);
		case RAPTOR_TERM_TYPE_LITERAL:
			str = (char*)(term->value.literal.string);
			resultsize = strlen(str) + 2;
			result = malloc(resultsize + 1);
			snprintf(result, resultsize + 1, "\"%s\"", str);
			return result;
		case RAPTOR_TERM_TYPE_BLANK:
			return strdup((char*)raptor_term_to_string(term));
		default:
			derp_log(DERP_LOG_WARNING, "Unknown thing in RDF term: %s", (char*)raptor_term_to_string(term));
			return NULL;
	}
}

static void handle_triple(void* user_data, raptor_statement* triple) {
	static char buf[2048];
	snprintf(buf, 2048, "(triple (subj %s) (pred %s) (obj %s))",
			term_to_readable(triple->subject),
			term_to_readable(triple->predicate),
			term_to_readable(triple->object));

	derp_assert_fact(buf);
}

static void free_data(gpointer data) {
	free(data);
}

void create_plugin() {
	prefix_map = g_hash_table_new_full(
			g_str_hash,   // hash function
			g_str_equal,  // comparator
			free_data,    // key destructor
			free_data);   // val destructor

	// Fill prefix map
	g_hash_table_insert(prefix_map, strdup("http://purl.org/dc/terms/"), strdup("dc"));
	g_hash_table_insert(prefix_map, strdup("http://www.w3.org/2000/01/rdf-schema#"), strdup("rdfs"));

	// Load rdf file
	raptor_world *world = NULL;
	raptor_parser* rdf_parser = NULL;
	unsigned char *uri_string;
	raptor_uri *uri, *base_uri;

	world = raptor_new_world();

	rdf_parser = raptor_new_parser(world, "rdfxml");

	raptor_parser_set_statement_handler(rdf_parser, NULL, handle_triple);

	uri_string = raptor_uri_filename_to_uri_string("dcterms.rdf");
	uri = raptor_new_uri(world, uri_string);
	base_uri = raptor_uri_copy(uri);

	raptor_parser_parse_file(rdf_parser, uri, base_uri);

	raptor_free_parser(rdf_parser);

	raptor_free_uri(base_uri);
	raptor_free_uri(uri);
	raptor_free_memory(uri_string);

	raptor_free_world(world);

	derp_log(DERP_LOG_INFO, "%d facts loaded", derp_get_facts_size());
}

static DerpPlugin plugin = {
	"Raptor",
	create_plugin
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

