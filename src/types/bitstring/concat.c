#include <jive/types/bitstring/concat.h>

#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/slice.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
jive_bitstring_multiop_node_init_(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * const operands[],
	size_t nbits)
{
	const jive_type * operand_types[noperands];
	jive_bitstring_type operand_type_structs[noperands];
	size_t n;
	
	for(n=0; n<noperands; n++) {
		operand_type_structs[n].base.base.class_ = &JIVE_BITSTRING_TYPE;
		operand_type_structs[n].nbits = ((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits;
		operand_types[n] = &operand_type_structs[n].base.base;
	}
	
	JIVE_DECLARE_BITSTRING_TYPE(output_type, nbits);
	
	jive_node_init_(self, region,
		noperands, operand_types, operands,
		1, &output_type);
}

static void
jive_bitconcat_node_init_(
	jive_node * self,
	jive_region * region,
	size_t noperands,
	jive_output * const operands[])
{
	size_t nbits = 0, n;
	for(n=0; n<noperands; n++)
		nbits += ((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits;
	jive_bitstring_multiop_node_init_(self, region, noperands, operands, nbits);
}

static char *
jive_bitconcat_node_get_label_(const jive_node * self);

static jive_node *
jive_bitconcat_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_binop_reduction_path_t
jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2);
//static bool
//jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);

static jive_output *
jive_bitconcat_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);
//static bool
//jive_bitconcat_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2);

const jive_binary_operation_class JIVE_BITCONCAT_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_BINARY_OPERATION,
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_bitconcat_node_get_label_, /* override */
		.get_attrs = jive_node_get_attrs_, /* inherit */
		.match_attrs = jive_node_match_attrs_, /* inherit */
		.create = jive_bitconcat_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},
	
	.flags = jive_binary_operation_associative,
	.single_apply_under = NULL,
	.multi_apply_under = NULL,
	.distributive_over = NULL,
	.distributive_under = NULL,
	
	.can_reduce_operand_pair = jive_bitconcat_can_reduce_operand_pair_, /* override */
	.reduce_operand_pair = jive_bitconcat_reduce_operand_pair_ /* override */
};

static char *
jive_bitconcat_node_get_label_(const jive_node * self)
{
	return strdup("BITCONCAT");
}

static jive_node *
jive_bitconcat_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITCONCAT_NODE;
	jive_bitconcat_node_init_(node, region, noperands, operands);
	return node;
}

static jive_binop_reduction_path_t
jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	if(jive_node_isinstance(op1->node, &JIVE_BITCONSTANT_NODE) &&
		jive_node_isinstance(op2->node, &JIVE_BITCONSTANT_NODE))
		return jive_binop_reduction_constants;

	const jive_bitslice_node * n1 = jive_bitslice_node_cast(op1->node);
	const jive_bitslice_node * n2 = jive_bitslice_node_cast(op2->node);

	if(n1 && n2){
		jive_output * origin1 = op1->node->inputs[0]->origin;
		jive_output * origin2 = op2->node->inputs[0]->origin;

		if (origin1 != origin2)
			return jive_binop_reduction_none;

		if (n1->attrs.high == n2->attrs.low)
			return jive_binop_reduction_merge;

		/* FIXME: support sign bit */
	}

	return jive_binop_reduction_none;
}

static jive_output *
jive_bitconcat_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	jive_graph * graph = op1->node->graph;

	if (path == jive_binop_reduction_constants) {
		const jive_bitconstant_node * n1 = (const jive_bitconstant_node *) op1->node;
		const jive_bitconstant_node * n2 = (const jive_bitconstant_node *) op2->node;

		size_t nbits = n1->attrs.nbits + n2->attrs.nbits;
		char bits[nbits];
		memcpy(bits, n1->attrs.bits, n1->attrs.nbits);
		memcpy(bits + n1->attrs.nbits, n2->attrs.bits, n2->attrs.nbits);

		return jive_bitconstant(graph, nbits, bits);
	}

	if (path == jive_binop_reduction_merge) {
		const jive_bitslice_node * n1 = (const jive_bitslice_node *) op1->node;
		const jive_bitslice_node * n2 = (const jive_bitslice_node *) op2->node;
		jive_output * origin1 = op1->node->inputs[0]->origin;

		return jive_bitslice(origin1, n1->attrs.low, n2->attrs.high);

		/* FIXME: support sign bit */
	}

	return NULL;
}

jive_node *
jive_bitconcat_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const])
{
	const jive_binary_operation_normal_form * nf =
		(const jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_BITCONCAT_NODE);
	return jive_binary_operation_normalized_create_new(nf, region, NULL, noperands, operands)->node;
}

jive_output *
jive_bitconcat(size_t noperands, jive_output * operands[const])
{
	jive_region * region = jive_region_innermost(noperands, operands);
	const jive_binary_operation_normal_form * nf =
		(const jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_BITCONCAT_NODE);
	return jive_binary_operation_normalized_create_new(nf, region, NULL, noperands, operands);
}