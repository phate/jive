/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_PRIVATE_H
#define JIVE_VSDG_NODE_PRIVATE_H

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/tracker.h>
#include <jive/util/list.h>

/* inheritable node member functions */

void
jive_node_init_(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * const operand_types[],
	struct jive_output * const operands[],
	size_t noutputs,
	const struct jive_type * const output_types[]);

void
jive_node_fini_(jive_node * self);

jive_node_normal_form *
jive_node_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent, struct jive_graph * graph);

char *
jive_node_get_label_(const jive_node * self);

const jive_node_attrs *
jive_node_get_attrs_(const jive_node * self);

jive_node *
jive_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

bool
jive_node_match_attrs_(const jive_node * self, const jive_node_attrs * other);

const struct jive_resource_class *
jive_node_get_aux_rescls_(const jive_node * self);

/* private node member functions */

jive_node *
jive_node_create(
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const]);

void
jive_node_add_successor(jive_node * self);

void
jive_node_remove_successor(jive_node * self);

void
jive_node_invalidate_depth_from_root(jive_node * self);

void
jive_node_auto_merge_variables(jive_node * self);

/* normal forms */

JIVE_EXPORTED_INLINE void
jive_node_normal_form_init_(jive_node_normal_form * self,
	const jive_node_class * node_class,
	jive_node_normal_form * parent,
	struct jive_graph * graph)
{
	self->node_class = node_class;
	
	self->parent = parent;
	self->graph = graph;
	
	if (parent) {
		self->enable_mutable = parent->enable_mutable;
		self->enable_cse = parent->enable_cse;
		JIVE_LIST_PUSH_BACK(parent->subclasses, self, normal_form_subclass_list);
	} else {
		self->enable_mutable = true;
		self->enable_cse = true;
	}
	
	self->subclasses.first = NULL;
	self->subclasses.last = NULL;
}

void
jive_node_normal_form_fini_(jive_node_normal_form * self);

bool
jive_node_normal_form_normalize_node_(const jive_node_normal_form * self, jive_node * node);

bool
jive_node_normal_form_operands_are_normalized_(const jive_node_normal_form * self,
	size_t noperands, jive_output * const operands[],
	const jive_node_attrs * attrs);

void
jive_node_normal_form_set_mutable_(jive_node_normal_form * self, bool enable);

void
jive_node_normal_form_set_cse_(jive_node_normal_form * self, bool enable);

#endif
