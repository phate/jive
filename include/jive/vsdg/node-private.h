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
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const]);

void
_jive_node_fini(jive_node * self);

char *
_jive_node_get_label(const jive_node * self);

const jive_node_attrs *
_jive_node_get_attrs(const jive_node * self);

jive_node *
_jive_node_create(const jive_node_attrs * attrs, struct jive_region * region,
	size_t noperands, struct jive_output * operands[]);

bool
_jive_node_equiv(const jive_node_attrs * first, const jive_node_attrs * second);

bool
_jive_node_can_reduce(const jive_output * first, const jive_output * second);

jive_output *
_jive_node_reduce(jive_output * first, jive_output * second);

const struct jive_regcls *
_jive_node_get_aux_regcls(const jive_node * self);

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
jive_node_add_used_resource(jive_node * self, jive_resource * resource);

void
jive_node_remove_used_resource(jive_node * self, jive_resource * resource);

void
jive_node_add_defined_resource(jive_node * self, jive_resource * resource);

void
jive_node_remove_defined_resource(jive_node * self, jive_resource * resource);

void
jive_node_add_crossed_resource(jive_node * self, jive_resource * resource, unsigned int count);

void
jive_node_remove_crossed_resource(jive_node * self, jive_resource * resource, unsigned int count);

void
jive_node_invalidate_depth_from_root(jive_node * self);

void
jive_node_unregister_resource_crossings(jive_node * self);

void
jive_node_register_resource_crossings(jive_node * self);

void
jive_node_remove_all_crossed(jive_node * self);

#endif
