#ifndef JIVE_SERIALIZATION_DRIVER_H
#define JIVE_SERIALIZATION_DRIVER_H

#include <jive/context.h>
#include <jive/serialization/symtab.h>

struct jive_graph;
struct jive_label_internal;
struct jive_serialization_instrcls_registry;
struct jive_serialization_nodecls_registry;
struct jive_serialization_typecls_registry;
struct jive_serialization_rescls_registry;
struct jive_token_istream;
struct jive_token_ostream;

typedef enum jive_unresolved_label_type {
	jive_unresolved_label_node = 0,
	jive_unresolved_label_region_start = 1,
	jive_unresolved_label_region_end = 2
} jive_unresolved_label_type;

typedef struct jive_unresolved_label jive_unresolved_label;
struct jive_unresolved_label {
	struct jive_label_internal * label;
	jive_unresolved_label_type type;
	char * symbol;
	
	struct {
		jive_unresolved_label * prev;
		jive_unresolved_label * next;
	} driver_unresolved_labels_list;
};

typedef struct jive_serialization_driver jive_serialization_driver;
struct jive_serialization_driver {
	jive_context * context;
	
	void (*error)(jive_serialization_driver *, const char []);
	
	jive_serialization_symtab symtab;
	
	const struct jive_serialization_instrcls_registry * instrcls_registry;
	const struct jive_serialization_nodecls_registry * nodecls_registry;
	const struct jive_serialization_typecls_registry * typecls_registry;
	const struct jive_serialization_rescls_registry * rescls_registry;
	
	struct {
		jive_unresolved_label * first;
		jive_unresolved_label * last;
	} unresolved_labels;
};

void
jive_serialization_driver_init(
	jive_serialization_driver * self,
	jive_context * context);

void
jive_serialization_driver_fini(
	jive_serialization_driver * self);

bool
jive_serialization_driver_resolve_labels(
	jive_serialization_driver * self);

void
jive_serialize_graph(
	jive_serialization_driver * self,
	struct jive_graph * graph,
	struct jive_token_ostream * os);

bool
jive_deserialize_graph(
	jive_serialization_driver * self,
	struct jive_token_istream * is,
	struct jive_graph * graph);

#endif
