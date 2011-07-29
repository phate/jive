#ifndef JIVE_REGALLOC_SHAPED_NODE_H
#define JIVE_REGALLOC_SHAPED_NODE_H

#include <jive/regalloc/xpoint.h>
#include <jive/vsdg/resource.h>

typedef struct jive_shaped_node jive_shaped_node;
typedef struct jive_shaped_node_downward_iterator jive_shaped_node_downward_iterator;

struct jive_shaped_graph;
struct jive_node;

struct jive_cut;

struct jive_shaped_node {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_node * node;
	
	struct {
		jive_shaped_node * prev;
		jive_shaped_node * next;
	} hash_chain;
	
	struct jive_cut * cut;
	struct {
		jive_shaped_node * prev;
		jive_shaped_node * next;
	} cut_location_list;
	
	jive_nodevar_xpoint_hash_byssavar ssavar_xpoints;
	jive_resource_class_count use_count_before;
	jive_resource_class_count use_count_after;
};

jive_shaped_node *
jive_shaped_node_prev_in_region(const jive_shaped_node * self);

jive_shaped_node *
jive_shaped_node_next_in_region(const jive_shaped_node * self);

static inline jive_shaped_node *
jive_shaped_node_prev_in_cut(const jive_shaped_node * self)
{
	return self->cut_location_list.prev;
}

static inline jive_shaped_node *
jive_shaped_node_next_in_cut(const jive_shaped_node * self)
{
	return self->cut_location_list.next;
}

void
jive_shaped_node_get_active_before(const jive_shaped_node * self, jive_mutable_varcut * cut);

void
jive_shaped_node_get_active_after(const jive_shaped_node * self, jive_mutable_varcut * cut);

void
jive_shaped_node_destroy(jive_shaped_node * self);

struct jive_shaped_node_downward_iterator {
	struct jive_shaped_graph * shaped_graph;
	struct jive_shaped_node * current;
	size_t leave_region_depth;
	size_t boundary_region_depth;
};

void
jive_shaped_node_downward_iterator_init(jive_shaped_node_downward_iterator * self, jive_shaped_node * start);

void
jive_shaped_node_downward_iterator_init_outward(jive_shaped_node_downward_iterator * self, jive_shaped_node * start);

jive_shaped_node *
jive_shaped_node_downward_iterator_next(jive_shaped_node_downward_iterator * self);

void
jive_shaped_node_downward_iterator_fini(jive_shaped_node_downward_iterator * self);

#endif
