/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_NODE_PRIVATE_H
#define JIVE_VSDG_NODE_PRIVATE_H

#include <jive/util/list.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/tracker.h>

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

jive_node *
jive_opnode_create(
	const jive::operation & op,
	jive_region * region,
	jive::output * const * args_begin,
	jive::output * const * args_end);

void
jive_node_fini_(jive_node * self);

void
jive_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

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

#endif
