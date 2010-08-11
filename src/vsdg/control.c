#include <jive/vsdg/control-private.h>

const jive_node_class JIVE_ABSTRACT_GAMMA_SLAVE_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.copy = _jive_node_copy, /* inherit */
	.equiv = _jive_node_equiv /* inherit */
};

void
_jive_abstract_gamma_slave_node_init(
	jive_abstract_gamma_slave_node * self,
	struct jive_region * region,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const])
{
	size_t real_noutputs = noutputs + 1, n;
	const jive_type * real_output_types[real_noutputs];
	JIVE_DECLARE_CONTROL_TYPE(ctrl);
	real_output_types[0] = ctrl;
	for(n=0; n<noutputs; n++) real_outputs_types[n+1] = outputs_types[n];
	
	_jive_node_init(self, region, noperands, operand_types, operands, real_noutputs, real_output_types);
}

const jive_node_class JIVE_ABSTRACT_GAMMA_MASTER_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.copy = _jive_node_copy, /* inherit */
	.equiv = _jive_node_equiv /* inherit */
};

void
_jive_abstract_gamma_master_node_init(
	jive_abstract_gamma_slave_node * self,
	struct jive_region * region,
	jive_control * false_alternative,
	jive_control * true_alternative,
	size_t noperands,
	const struct jive_type * operand_types[const],
	struct jive_output * operands[const],
	size_t noutputs,
	const struct jive_type * output_types[const])
{
	size_t real_ninputs = ninputs + 2, n;
	const jive_type * real_operand_types[real_ninputs];
	const jive_output * real_operands[real_ninputs];
	JIVE_DECLARE_CONTROL_TYPE(ctrl);
	real_operand_types[0] = ctrl; real_operands[0] = false_alternative;
	real_operand_types[1] = ctrl; real_operands[1] = true_alternative;
	for(n=0; n<ninputs; n++) {
		real_operand_types[n+2] = operand_types[n];
		real_operands[n+2] = operands[n];
	}
	
	_jive_node_init(self, region, real_noperand, real_operand_types, real_operands, noutputs, output_types);
}

const jive_node_class JIVE_GAMMA_SLAVE_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_gamma_slave_node_get_label, /* override */
	.copy = _jive_node_copy, /* inherit */
	.equiv = _jive_node_equiv /* inherit */
};

void
_jive_gamma_slave_node_init(
	jive_gamma_slave_node * self,
	struct jive_region * region)
{
	_jive_abstract_gamma_slave_node_init(self, region,
		0, 0, 0,
		0, 0);
}

char *
_jive_gamma_slave_node_get_label(const jive_node * self)
{
	return strdup("ALTERNATIVE");
}

const jive_node_class JIVE__GAMMA_MASTER_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_gamma_slave_node_get_label, /* override */
	.copy = _jive_node_copy, /* inherit */
	.equiv = _jive_node_equiv /* inherit */
};

void
_jive_gamma_master_node_init(
	jive_gamma_slave_node * self,
	struct jive_region * region,
	jive_control * false_alternative,
	jive_control * true_alternative,
	jive_value * predicate)
{
	jive_input * operand = predicate;
	JIVE_DECLARE_VALUE_TYPE(value_type);
	
	_jive_abstract_gamma_master_node_init(self, region,
		false_alternative, true_alternative,
		1, &value_type, &operand,
		0, 0);
}

char *
_jive_gamma_master_node_get_label(const jive_node * self)
{
	return strdup("GAMMA");
}
