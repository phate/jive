/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
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
	const jive::base::type * const operand_types[],
	jive::output * const operands[],
	size_t noutputs,
	const jive::base::type * const output_types[]);

template<
	typename Operation,
	typename ArgumentList
>
jive_node *
jive_opnode_create(
	const Operation & op,
	const jive_node_class * cls,
	jive_region * region,
	ArgumentList arguments)
{
	JIVE_DEBUG_ASSERT(arguments.size() == op.narguments());
	const jive::base::type * argument_types[op.narguments()];
	jive::output * argument_values[op.narguments()];
	typename ArgumentList::const_iterator i = arguments.begin();
	for (size_t n = 0; n < op.narguments(); ++n) {
		argument_types[n] = &op.argument_type(n);
		argument_values[n] = *i;
		++i;
	}
	
	const jive::base::type * result_types[op.nresults()];
	for (size_t n = 0; n < op.nresults(); ++n) {
		result_types[n] = &op.result_type(n);
	}
	jive_node * node = jive::create_operation_node(op);
	node->class_ = cls;
	jive_node_init_(node, region,
		op.narguments(), argument_types, argument_values,
		op.nresults(), result_types);
	return node;
}

template<
	typename Operation,
	typename ArgumentIterator
>
jive_node *
jive_opnode_create(
	const Operation & op,
	const jive_node_class * cls,
	jive_region * region,
	ArgumentIterator begin,
	ArgumentIterator end)
{
	const jive::base::type * argument_types[op.narguments()];
	jive::output * argument_values[op.narguments()];
	for (size_t n = 0; n < op.narguments(); ++n) {
		argument_types[n] = &op.argument_type(n);
		JIVE_DEBUG_ASSERT(begin != end);
		argument_values[n] = *begin;
		++begin;
	}
	JIVE_DEBUG_ASSERT(begin == end);
	
	const jive::base::type * result_types[op.nresults()];
	for (size_t n = 0; n < op.nresults(); ++n) {
		result_types[n] = &op.result_type(n);
	}
	jive_node * node = jive::create_operation_node(op);
	node->class_ = cls;
	jive_node_init_(node, region,
		op.narguments(), argument_types, argument_values,
		op.nresults(), result_types);
	return node;
}

void
jive_node_fini_(jive_node * self);

jive_node_normal_form *
jive_node_get_default_normal_form_(const jive_node_class * cls, jive_node_normal_form * parent, struct jive_graph * graph);

void
jive_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

void
jive_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context);

jive_node *
jive_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[]);

bool
jive_node_match_attrs_(const jive_node * self, const jive_node_attrs * other);

/* private node member functions */

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
	size_t noperands, jive::output * const operands[],
	const jive_node_attrs * attrs);

void
jive_node_normal_form_normalized_create_(const jive_node_normal_form * self,
	struct jive_graph * graph, const jive_node_attrs * attrs, size_t noperands,
	jive::output * const operands[], jive::output * result[]);

void
jive_node_normal_form_set_mutable_(jive_node_normal_form * self, bool enable);

void
jive_node_normal_form_set_cse_(jive_node_normal_form * self, bool enable);

#endif
