/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_SHAPED_REGION_H
#define JIVE_REGALLOC_SHAPED_REGION_H

#include <stdbool.h>

#include <jive/common.h>

#include <jive/regalloc/xpoint.h>

typedef struct jive_shaped_region jive_shaped_region;
typedef struct jive_cut jive_cut;

struct jive_shaped_graph;
struct jive_region;
struct jive_node;

struct jive_shaped_region {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_region * region;
	
	struct {
		jive_shaped_region * prev;
		jive_shaped_region * next;
	} hash_chain;
	
	struct {
		jive_cut * first;
		jive_cut * last;
	} cuts;
	
	jive_region_varcut active_top;
};

struct jive_shaped_node;

struct jive_cut {
	jive_shaped_region * shaped_region;
	struct {
		jive_cut * prev;
		jive_cut * next;
	} region_cut_list;
	
	struct {
		struct jive_shaped_node * first;
		struct jive_shaped_node * last;
	} locations;
};

jive_shaped_region *
jive_shaped_region_create(struct jive_shaped_graph * shaped_graph, struct jive_region * region);

/** \brief Create new top-most cut */
jive_cut *
jive_shaped_region_create_cut(jive_shaped_region * self);

void
jive_shaped_region_destroy(jive_shaped_region * self);

void
jive_shaped_region_destroy_cuts(jive_shaped_region * self);

void
jive_cut_destroy(jive_cut * self);

/** \brief Create new cut in same region above this cut */
jive_cut *
jive_cut_create_above(jive_cut * self);

/** \brief Create new cut in same region below this cut */
jive_cut *
jive_cut_create_below(jive_cut * self);

/** \brief Split cut at location, return empty cut before split point */
jive_cut *
jive_cut_split(jive_cut * self, struct jive_shaped_node * at);

/** \brief Insert node into cut */
struct jive_shaped_node *
jive_cut_insert(jive_cut * self, struct jive_shaped_node * before, struct jive_node * node);

JIVE_EXPORTED_INLINE struct jive_shaped_node *
jive_cut_append(jive_cut * self, struct jive_node * node)
{
	return jive_cut_insert(self, 0, node);
}

/** \brief First node in region */
struct jive_shaped_node *
jive_shaped_region_first(const jive_shaped_region * self);

/** \brief Last node in region */
struct jive_shaped_node *
jive_shaped_region_last(const jive_shaped_region * self);

#endif
