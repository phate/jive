#ifndef JIVE_VSDG_NODE_PRIVATE_H
#define JIVE_VSDG_NODE_PRIVATE_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>

/* inheritable node member functions */

void
_jive_node_init(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * const operand_types[],
	struct jive_output * const operands[],
	size_t noutputs,
	const struct jive_type * const output_types[]);

void
_jive_node_fini(jive_node * self);

const jive_node_normal_form *
_jive_node_get_default_normal_form(const jive_node * self);

char *
_jive_node_get_label(const jive_node * self);

const jive_node_attrs *
_jive_node_get_attrs(const jive_node * self);

jive_node *
_jive_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

bool
_jive_node_match_attrs(const jive_node * self, const jive_node_attrs * other);

bool
_jive_node_can_reduce(const jive_output * first, const jive_output * second);

jive_output *
_jive_node_reduce(jive_output * first, jive_output * second);

const struct jive_resource_class *
_jive_node_get_aux_rescls(const jive_node * self);

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
