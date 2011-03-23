#ifndef JIVE_REGALLOC_SHAPED_VARIABLE_H
#define JIVE_REGALLOC_SHAPED_VARIABLE_H

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
};

jive_shaped_variable *
jive_shaped_variable_create(struct jive_shaped_graph * shaped_graph, struct jive_variable * variable);

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
};

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar);

void
jive_shaped_ssavar_destroy(jive_shaped_ssavar * self);

#endif
