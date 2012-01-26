#include <jive/types/bitstring/comparison/bitequal.h>

#include <jive/vsdg/region.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>

static jive_node *
jive_bitequal_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_binop_reduction_path_t
jive_bitequal_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2);

static jive_output *
jive_bitequal_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1,
	jive_output * op2);

const jive_bitcomparison_operation_class JIVE_BITEQUAL_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BITCOMPARISON_NODE,
			.name = "BITEQUAL",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_bitequal_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		.flags = jive_binary_operation_commutative,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,
	
		.can_reduce_operand_pair = jive_bitequal_node_can_reduce_operand_pair_, /* override */
		.reduce_operand_pair = jive_bitequal_node_reduce_operand_pair_ /* override */
	},
	.type = jive_bitcmp_code_equal,
	.compare_constants = NULL
};

static void
jive_bitequal_node_init_(jive_node * self, jive_region * region,
	jive_output * operand1, jive_output * operand2)
{
	if (!jive_output_isinstance(operand1, &JIVE_BITSTRING_OUTPUT)){
		jive_context_fatal_error(region->graph->context, "Type mismatch: bitequal node requires bitstring operands");
	}
	size_t nbits = ((jive_bitstring_output *)operand1)->type.nbits;

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	jive_node_init_(self, region,
		2, (const jive_type *[]){btype, btype}, (jive_output *[]){operand1, operand2},
		1, &ctype);
}

static jive_node *
jive_bitequal_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITEQUAL_NODE;
	jive_bitequal_node_init_(node, region, operands[0], operands[1]);

	return node;
}

static jive_binop_reduction_path_t
jive_bitequal_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	if (jive_node_isinstance(op1->node, &JIVE_BITCONSTANT_NODE) && jive_node_isinstance(op2->node, &JIVE_BITCONSTANT_NODE))
		return jive_binop_reduction_constants;

	return jive_binop_reduction_none;
}

static jive_output *
jive_bitequal_node_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (path == jive_binop_reduction_constants) {
		const jive_bitconstant_node * n1 = (const jive_bitconstant_node *) op1->node;
		const jive_bitconstant_node * n2 = (const jive_bitconstant_node *) op2->node;

		JIVE_DEBUG_ASSERT(n1->attrs.nbits == n2->attrs.nbits);
		char result = jive_bitstring_equal(n1->attrs.bits, n2->attrs.bits,
			n1->attrs.nbits);

		switch(result){
			case '0':
				return jive_control_false(op1->node->graph);
			case '1':
				return jive_control_true(op1->node->graph);
			default:
				return NULL;
		};
	}

	return NULL;
}

jive_node *
jive_bitequal_create(jive_region * region,
	jive_output * operand1, jive_output * operand2)
{
	return jive_bitequal_create_(region, NULL, 2, (jive_output *[]){operand1, operand2});
}

jive_output *
jive_bitequal(jive_output * operand1, jive_output * operand2)
{
	jive_output * operands[] = {operand1, operand2};
	jive_region * region = jive_region_innermost(2, operands);
	const jive_binary_operation_normal_form * nf = (const jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_BITEQUAL_NODE);

	return jive_binary_operation_normalized_create_new(nf, region, NULL, 2, operands);
}
