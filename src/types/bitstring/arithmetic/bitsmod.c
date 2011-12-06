#include <jive/types/bitstring/arithmetic/bitsmod.h>

#include <jive/vsdg/region.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/bitstring-operations.h>

static jive_node *
jive_bitsmod_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_binop_reduction_path_t
jive_bitsmod_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2);

static jive_output *
jive_bitsmod_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	jive_output * op1, jive_output * op2);

const jive_bitbinary_operation_class JIVE_BITSMOD_NODE_ = {
	.base = { /* jive_bitbinary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BITBINARY_NODE,
			.name = "BITSMOD",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_bitsmod_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},

		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,

		.can_reduce_operand_pair = jive_bitsmod_node_can_reduce_operand_pair_, /* override */
		.reduce_operand_pair = jive_bitsmod_node_reduce_operand_pair_ /* override */	
	},
	.type = jive_bitop_code_smod
};

static void
jive_bitsmod_node_init_(jive_node * self, jive_region * region,
	size_t noperands, jive_output * const operands[])
{
	if (!jive_output_isinstance(operands[0], &JIVE_BITSTRING_OUTPUT)){
		jive_context_fatal_error(region->graph->context, "Type mismatch: bitsmod node requires bitstring operands");
	}
	size_t nbits = ((jive_bitstring_output *)operands[0])->type.nbits;
	
	size_t n;
	const jive_type * operand_types[noperands];
	JIVE_DECLARE_BITSTRING_TYPE(output_type, nbits);
	for(n = 0; n < noperands; n++)
		operand_types[n] = output_type;

	jive_node_init_(self, region,
		noperands, operand_types, operands,
		1, &output_type);
}

static jive_node *
jive_bitsmod_create_(jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[])
{
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITSMOD_NODE;
	jive_bitsmod_node_init_(node, region, noperands, operands);

	return node;
}

static jive_binop_reduction_path_t
jive_bitsmod_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	return jive_binop_reduction_none;
}

static jive_output *
jive_bitsmod_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs,
	jive_output * op1, jive_output * op2)
{
	return NULL;
}

jive_node *
jive_bitsmod_create(jive_region * region, jive_output * dividend, jive_output * divisor)
{
	return jive_binary_operation_normalized_create(&JIVE_BITSMOD_NODE, region, NULL,
		2, (jive_output * []){dividend, divisor})->node;
}

jive_output *
jive_bitsmod(jive_output * dividend, jive_output * divisor)
{
	jive_output * operands[] = {dividend, divisor};
	jive_region * region = jive_region_innermost(2, operands);
	return jive_binary_operation_normalized_create(&JIVE_BITSMOD_NODE, region, NULL,
		2, operands);
}

