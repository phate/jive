#ifndef JIVE_VSDG_NODE_PRIVATE_H
#define JIVE_VSDG_NODE_PRIVATE_H

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/tracker.h>

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

const jive_node_normal_form *
jive_node_get_default_normal_form_(const jive_node * self);

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

#endif
