/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_SEQUENCE_H
#define JIVE_VSDG_SEQUENCE_H

#include <jive/util/hash.h>

#include <jive/vsdg/label.h>

struct jive_graph;
struct jive_node;
struct jive_notifier;
struct jive_region;

typedef struct jive_seq_graph jive_seq_graph;
typedef struct jive_seq_point_class jive_seq_point_class;
typedef struct jive_seq_point jive_seq_point;
typedef struct jive_seq_node jive_seq_node;
typedef struct jive_seq_region jive_seq_region;

struct jive_seq_point_class {
	const jive_seq_point_class * parent;
	void (*fini)(jive_seq_point * self);
};

struct jive_seq_point {
	const jive_seq_point_class * class_;
	jive_seq_region * seq_region;
	
	struct {
		jive_seq_point * prev;
		jive_seq_point * next;
	} seqpoint_list;
	
	struct {
		jive_label_internal ** items;
		size_t nitems, space;
	} attached_labels;
	
	size_t size;
	jive_address address;
};

extern const jive_seq_point_class JIVE_SEQ_POINT;


struct jive_seq_node {
	jive_seq_point base;
	
	struct jive_node * node;
	
	struct {
		jive_seq_node * prev;
		jive_seq_node * next;
	} hash_chain;
	
	uint32_t flags;
};

extern const jive_seq_point_class JIVE_SEQ_NODE;

JIVE_EXPORTED_INLINE jive_seq_node *
jive_seq_node_cast(jive_seq_point * self)
{
	if (self->class_ == &JIVE_SEQ_NODE)
		return (jive_seq_node *) self;
	else
		return 0;
}

struct jive_seq_region {
	struct jive_region * region;
	jive_seq_graph * seq_graph;
	struct {
		jive_seq_region * prev;
		jive_seq_region * next;
	} seqregion_list;
	struct {
		jive_seq_region * prev;
		jive_seq_region * next;
	} hash_chain;
	
	jive_seq_point * first_point;
	jive_seq_point * last_point;
};

typedef struct jive_seq_node_hash jive_seq_node_hash;
typedef struct jive_seq_region_hash jive_seq_region_hash;
JIVE_DECLARE_HASH_TYPE(jive_seq_node_hash, jive_seq_node, struct jive_node *, node, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_seq_region_hash, jive_seq_region, struct jive_region *, region, hash_chain);

struct jive_seq_graph {
	struct jive_context * context;
	struct jive_graph * graph;
	struct {
		jive_seq_point * first;
		jive_seq_point * last;
	} points;
	struct {
		jive_seq_region * first;
		jive_seq_region * last;
	} regions;
	
	jive_seq_node_hash node_map;
	jive_seq_region_hash region_map;
	
	struct jive_notifier * label_notifier;
	
	bool addrs_changed;
};

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_seq_region_map_to_section(const jive_seq_region * seq_region)
{
	return jive_region_map_to_section(seq_region->region);
}

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_seq_point_map_to_section(const jive_seq_point * seq_point)
{
	return jive_seq_region_map_to_section(seq_point->seq_region);
}

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_seq_node_map_to_section(const jive_seq_node * seq_node)
{
	return jive_seq_region_map_to_section(seq_node->base.seq_region);
}

JIVE_EXPORTED_INLINE void
jive_seq_point_init(jive_seq_point * self, jive_seq_region * seq_region)
{
	self->seq_region = seq_region;
	self->size = 0;
	self->attached_labels.items = 0;
	self->attached_labels.nitems = self->attached_labels.space = 0;
	jive_address_init(&self->address, jive_stdsectionid_invalid, 0);
}

JIVE_EXPORTED_INLINE void
jive_seq_point_destroy(jive_seq_point * self)
{
	jive_context * context = self->seq_region->seq_graph->context;
	self->class_->fini(self);
	jive_context_free(context, self);
}

JIVE_EXPORTED_INLINE bool
jive_seq_point_isinstance(const jive_seq_point * self, const jive_seq_point_class * class_)
{
	const jive_seq_point_class * c = self->class_;
	while (c) {
		if (c == class_)
			return true;
		c = c->parent;
	}
	return false;
}

/**
	\brief Sequentialize graph
*/
jive_seq_graph *
jive_graph_sequentialize(struct jive_graph * graph);

void
jive_seq_graph_destroy(jive_seq_graph * seq);

jive_seq_node *
jive_seq_graph_map_node(const jive_seq_graph * seq, struct jive_node * node);

jive_seq_region *
jive_seq_graph_map_region(const jive_seq_graph * seq, struct jive_region * region);

/* inheritable methods */
void
jive_seq_point_fini_(jive_seq_point * self);

#endif
