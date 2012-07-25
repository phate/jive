#ifndef JIVE_SERIALIZATION_DRIVER_H
#define JIVE_SERIALIZATION_DRIVER_H

#include <jive/context.h>
#include <jive/serialization/symtab.h>

struct jive_graph;
struct jive_serialization_nodecls_registry;
struct jive_serialization_typecls_registry;
struct jive_serialization_rescls_registry;
struct jive_token_istream;
struct jive_token_ostream;

typedef struct jive_serialization_driver jive_serialization_driver;
struct jive_serialization_driver {
	jive_context * context;
	
	void (*error)(jive_serialization_driver *, const char []);
	
	jive_serialization_symtab symtab;
	
	const struct jive_serialization_nodecls_registry * nodecls_registry;
	const struct jive_serialization_typecls_registry * typecls_registry;
	const struct jive_serialization_rescls_registry * rescls_registry;
};

void
jive_serialization_driver_init(
	jive_serialization_driver * self,
	jive_context * context);

void
jive_serialization_driver_fini(
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
