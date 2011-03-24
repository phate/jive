#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_H

#include <jive/regalloc/xpoint.h>
#include <jive/util/hash.h>

typedef struct jive_variable_interference jive_variable_interference;
typedef struct jive_variable_interference_part jive_variable_interference_part;
typedef struct jive_variable_interference_hash jive_variable_interference_hash;

JIVE_DECLARE_HASH_TYPE(jive_variable_interference_hash, struct jive_variable_interference_part, struct jive_variable *, variable, chain);

typedef struct jive_shaped_variable jive_shaped_variable;

struct jive_shaped_graph;
struct jive_variable;

struct jive_shaped_variable {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_variable * variable;
	
	struct {
		jive_shaped_variable * prev;
		jive_shaped_variable * next;
	} hash_chain;
	
	jive_variable_interference_hash interference;
};

jive_shaped_variable *
jive_shaped_variable_create(struct jive_shaped_graph * shaped_graph, struct jive_variable * variable);

size_t
jive_shaped_variable_interferes_with(const jive_shaped_variable * self, const jive_shaped_variable * other);

void
jive_shaped_variable_destroy(jive_shaped_variable * self);

typedef struct jive_shaped_ssavar jive_shaped_ssavar;

struct jive_shaped_graph;
struct jive_ssavar;

struct jive_shaped_ssavar {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_ssavar * ssavar;
	
	struct {
		jive_shaped_ssavar * prev;
		jive_shaped_ssavar * next;
	} hash_chain;
	
	jive_node_xpoint_hash node_xpoints;
};

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar);

void
jive_shaped_ssavar_destroy(jive_shaped_ssavar * self);

#endif
