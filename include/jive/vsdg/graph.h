#ifndef JIVE_VSDG_GRAPH_H
#define JIVE_VSDG_GRAPH_H

#include <stdlib.h>
#include <stdbool.h>

#include <jive/context.h>

typedef struct jive_graph jive_graph;

struct jive_resource;
struct jive_node;
struct jive_region;
struct jive_gate;
struct jive_traverser_graphstate;

struct jive_graph {
	jive_context * context;
	
	struct {
		struct jive_node * first;
		struct jive_node * last;
	} top;
	struct {
		struct jive_node * first;
		struct jive_node * last;
	} bottom;
	
	struct jive_region * root_region;
	
	struct {
		struct jive_gate * first;
		struct jive_gate * last;
	} gates;
	
	struct {
		struct jive_resource * first;
		struct jive_resource * last;
	} resources;
	
	bool resources_fully_assigned;
	
	size_t ntraverser_slots;
	struct jive_traverser_graphstate * traverser_slots;
};

jive_graph *
jive_graph_create(jive_context * context);

void
jive_graph_destroy(jive_graph * self);

bool
jive_graph_has_active_traversers(const jive_graph * self);

#endif
