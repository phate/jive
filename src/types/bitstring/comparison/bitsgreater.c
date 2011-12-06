#include <jive/types/bitstring/comparison/bitsgreater.h>

#include <jive/vsdg/region.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/types/bitstring/constant.h>

static jive_node *
jive_bitsgreater_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_binop_reduction_path_t
jive_bitsgreater_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2);

static jive_output *
jive_bitsgreater_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1,
	jive_output * op2);

const jive_bitcomparison_operation_class JIVE_BITSGREATER_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BITCOMPARISON_NODE,
			.name = "BITSGREATER",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_bitsgreater_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,
	
		.can_reduce_operand_pair = jive_bitsgreater_node_can_reduce_operand_pair_, /* override */
		.reduce_operand_pair = jive_bitsgreater_node_reduce_operand_pair_ /* override */
	},
	.type = jive_bitcmp_code_sgreater,
	.compare_constants = NULL
};

static void
jive_bitsgreater_node_init_(jive_node * self, jive_region * region,
	jive_output * operand1, jive_output * operand2)
{
	if (!jive_output_isinstance(operand1, &JIVE_BITSTRING_OUTPUT)){
		jive_context_fatal_error(region->graph->context, "Type mismatch: bitsgreater node requires bitstring operands");
	}
	size_t nbits = ((jive_bitstring_output *)operand1)->type.nbits;

	JIVE_DECLARE_CONTROL_TYPE(ctype);
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	jive_node_init_(self, region,
		2, (const jive_type *[]){btype, btype}, (jive_output *[]){operand1, operand2},
		1, &ctype);
}

static jive_node *
jive_bitsgreater_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITSGREATER_NODE;
	jive_bitsgreater_node_init_(node, region, operands[0], operands[1]);

	return node;
}

static jive_binop_reduction_path_t 
jive_bitsgreater_node_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	const jive_bitconstant_node * n1 = jive_bitconstant_node_cast(op1->node);
	const jive_bitconstant_node * n2 = jive_bitconstant_node_cast(op2->node);

	/* constant < constant */
	if (n1 && n2) {
		JIVE_DEBUG_ASSERT(n1->attrs.nbits == n2->attrs.nbits);
		char result = jive_bitstring_sgreater(n1->attrs.bits, n2->attrs.bits,
			n1->attrs.nbits);

		switch(result){
			case '0': return 1;
			case '1': return 2;
			default: return jive_binop_reduction_none;
		}
	}

	/* INT_MIN > constant */
	if (n1){
		size_t nbits = n1->attrs.nbits;
		char int_min[nbits];
		jive_bitstring_init_signed(int_min, nbits, 0);
		int_min[nbits-1] = jive_bit_not(int_min[nbits-1]);
	
		if (jive_bitstring_equal(n1->attrs.bits, int_min, nbits) == '1')
			return 3;
	}
	
	/* constant > INT_MAX */
	if (n2){
		size_t nbits = n2->attrs.nbits;
		char int_max[nbits];
		jive_bitstring_init_signed(int_max, nbits, -1);
		int_max[nbits-1] = jive_bit_not(int_max[nbits-1]);

		if (jive_bitstring_equal(n2->attrs.bits, int_max, nbits) == '1')
			return 4;
	}

	return jive_binop_reduction_none;
}

static jive_output *
jive_bitsgreater_node_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1,
	jive_output * op2)
{
	jive_graph * graph = op1->node->graph;

	switch(path){
		case 1:
		case 3:
		case 4:
			return jive_control_false(graph);
		case 2:
			return jive_control_true(graph);
		default:
			return NULL;
	}
}

jive_node *
jive_bitsgreater_create(jive_region * region,
	jive_output * operand1, jive_output * operand2)
{
	return jive_bitsgreater_create_(region, NULL, 2, (jive_output *[]){operand1, operand2});
}

jive_output *
jive_bitsgreater(jive_output * operand1, jive_output * operand2)
{
	jive_output * operands[] = {operand1, operand2};
	jive_region * region = jive_region_innermost(2, operands);
	const jive_binary_operation_normal_form * nf = (const jive_binary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_BITSGREATER_NODE);

	return jive_binary_operation_normalized_create_new(nf, region, NULL, 2, operands);
}
