#ifndef JIVE_VSDG_REGION_H
#define JIVE_VSDG_REGION_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct jive_region jive_region;

struct jive_graph;
struct jive_node;

struct jive_region {
	struct jive_graph * graph;
	jive_region * parent;
	struct {
		struct jive_node * first;
		struct jive_node * last;
	} nodes;
	struct {
		jive_region * first;
		jive_region * last;
	} subregions;
	struct {
		jive_region * prev;
		jive_region * next;
	} region_subregions_list;
	
	void * top_cut; /* TODO: data type */
	void * bottom_cut; /* TODO: data type */
	struct jive_node * anchor_node;
};

void
jive_region_destroy(jive_region * self);

static inline bool
jive_region_empty(const jive_region * self)
{
	return self->nodes.first == 0 && self->subregions.first == 0;
}

#endif
