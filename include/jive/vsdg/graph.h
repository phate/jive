#ifndef JIVE_VSDG_GRAPH_H
#define JIVE_VSDG_GRAPH_H

#include <stdlib.h>
#include <stdbool.h>

#include <jive/context.h>
#include <jive/vsdg/notifiers.h>

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
	struct {
		struct jive_resource * first;
		struct jive_resource * last;
	} unused_resources;
	
	bool resources_fully_assigned;
	
	size_t ntraverser_slots;
	struct jive_traverser_graphstate * traverser_slots;
	
	jive_node_notifier_slot on_node_create;
	jive_node_notifier_slot on_node_destroy;
	jive_node_notifier_slot on_node_shape;
	
	jive_input_notifier_slot on_input_create;
	jive_input_change_notifier_slot on_input_change;
	jive_input_notifier_slot on_input_destroy;
	
	jive_output_notifier_slot on_output_create;
	jive_output_notifier_slot on_output_destroy;
};

jive_graph *
jive_graph_create(jive_context * context);

void
jive_graph_destroy(jive_graph * self);

void
jive_graph_prune(jive_graph * self);

bool
jive_graph_has_active_traversers(const jive_graph * self);

#endif
