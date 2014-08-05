#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <raptor2.h>
#include "derp.h"

#define max(a, b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; })

GHashTable* prefix_map;

// For an URI such as http://foo.bar/baz returns the qname, if the prefix is
// known, for example bar:baz. If the prefix is unknown, the result is the RDF
// form of the URI, e.g. <http://foo.bar/baz>.
static void uri_to_qname(char* uri, char* qname, int qname_size) {
	char* base_uri;
	char* prefix;
	char* suffix;
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
		snprintf(qname, max(qname_size, resultlen + 1), "%s:%s", prefix, suffix);
	} else {
		resultlen = strlen(uri) + 2;
		snprintf(qname, max(qname_size, resultlen + 1), "<%s>", uri);
	}

	free(base_uri);
}

static void term_to_readable(raptor_term* term, char* readable, int readable_size) {
	unsigned char* uri;
	//char* result;
	char* str;
	int resultsize;
	switch (term->type) {
		case RAPTOR_TERM_TYPE_URI:
			uri = raptor_uri_as_string(term->value.uri);
			uri_to_qname((char*)uri, readable, readable_size);
			return;
		case RAPTOR_TERM_TYPE_LITERAL:
			str = (char*)(term->value.literal.string);
			resultsize = strlen(str) + 2;
			snprintf(readable, max(readable_size, resultsize + 1), "\"%s\"", str);
			return;// result;
		case RAPTOR_TERM_TYPE_BLANK:
			snprintf(readable, readable_size, "%s", (char*)raptor_term_to_string(term));
			return;
		default:
			derp_log(DERP_LOG_WARNING, "Unknown thing in RDF term: %s", (char*)raptor_term_to_string(term));
			return;
	}
}

static void handle_triple(void* user_data, raptor_statement* triple) {
	int bufsize = 1024;
	char* s = malloc(bufsize);
	char* p = malloc(bufsize);
	char* o = malloc(bufsize);
	term_to_readable(triple->subject, s, bufsize);
	term_to_readable(triple->predicate, p, bufsize);
	term_to_readable(triple->object, o, bufsize);
	derp_assert_triple(s, p, o);
	free(s);
	free(p);
	free(o);
}

void start_plugin() {
	prefix_map = g_hash_table_new_full(
			g_str_hash,        // hash function
			g_str_equal,       // comparator
			derp_free_data,    // key destructor
			derp_free_data);   // val destructor

	// Fill prefix map
	g_hash_table_insert(prefix_map, strdup("http://purl.org/dc/terms/"), strdup("dc"));
	g_hash_table_insert(prefix_map, strdup("http://www.w3.org/2000/01/rdf-schema#"), strdup("rdfs"));

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

	raptor_parser_parse_file(rdf_parser, uri, base_uri);

	raptor_free_parser(rdf_parser);
	raptor_free_uri(base_uri);
	raptor_free_uri(uri);
	raptor_free_memory(uri_string);
	raptor_free_world(world);

	derp_log(DERP_LOG_INFO, "%d facts loaded", derp_get_facts_size());
}

void shutdown_plugin() {
	printf("Shutting down raptor\n");
	//free(prefix_map);
	g_hash_table_destroy(prefix_map);
}

static DerpPlugin plugin = {
	"Raptor",
	start_plugin,
	shutdown_plugin
};

DerpPlugin* derp_init_plugin(void) {
	return &plugin;
}

