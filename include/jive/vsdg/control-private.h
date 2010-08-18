#ifndef JIVE_VSDG_CONTROL_PRIVATE_H
#define JIVE_VSDG_CONTROL_PRIVATE_H

#include <jive/vsdg/control.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/controltype.h>

void
_jive_abstract_gamma_slave_node_init(
	jive_abstract_gamma_slave_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const]);

void
_jive_abstract_gamma_master_node_init(
	jive_abstract_gamma_slave_node * self,
	struct jive_region * region,
	jive_control_output * false_alternative,
	jive_control_output * true_alternative,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const]);

void
_jive_gamma_slave_node_init(
	jive_gamma_slave_node * self,
	struct jive_region * region);

char *
_jive_gamma_slave_node_get_label(const jive_node * self);

void
_jive_gamma_master_node_init(
	jive_gamma_slave_node * self,
	struct jive_region * region,
	jive_control_output * false_alternative,
	jive_control_output * true_alternative,
	jive_value * predicate);

char *
_jive_gamma_master_node_get_label(const jive_node * self);

#endif
