#ifndef JIVE_VSDG_CUT_H
#define JIVE_VSDG_CUT_H

#include <stdbool.h>

typedef struct jive_cut jive_cut;
typedef struct jive_node_location jive_node_location;

struct jive_cut;
struct jive_node;

struct jive_node_location {
	struct jive_node * node;
	jive_cut * cut;
	struct {
		jive_node_location * prev;
		jive_node_location * next;
	} cut_nodes_list;
};

struct jive_cut {
	struct jive_region * region;
	
	struct {
		jive_cut * prev;
		jive_cut * next;
	} region_cuts_list;
	
	struct {
		jive_node_location * first;
		jive_node_location * last;
	} nodes;
};

void
jive_cut_destroy(jive_cut * self);

jive_node_location *
jive_cut_append(jive_cut * self, struct jive_node * node);

jive_node_location *
jive_cut_insert(jive_cut * self, jive_node_location * at, struct jive_node * node);

static inline bool
jive_cut_empty(const jive_cut * self)
{
	return self->nodes.first == 0;
}

jive_node_location *
jive_node_location_next_in_region_slow(const jive_node_location * self);

static inline jive_node_location *
jive_node_location_next_in_region(const jive_node_location * self)
{
	jive_node_location * loc = self->cut_nodes_list.next;
	if (!loc) return jive_node_location_next_in_region_slow(self);
	return loc;
}

#endif
