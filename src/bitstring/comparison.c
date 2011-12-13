#include <jive/bitstring/comparison.h>

#include <string.h>

#include <jive/common.h>

#include <jive/bitstring/bitstring-operations.h>
#include <jive/bitstring/constant.h>
#include <jive/bitstring/slice.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

const jive_bitcomparison_operation_class JIVE_BITCOMPARISON_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BINARY_OPERATION,
			.name = "BITCOMPARISON",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_node_create_, /* inherit */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		
		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,
		
		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_ /* inherit */,
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair_ /* inherit */
	},
	.type = jive_bitcmp_code_invalid
};

static void
jive_bitcomparison_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * x, jive_output * y,
	size_t nbits)
{
	JIVE_DECLARE_BITSTRING_TYPE(input_type, nbits);
	const jive_type * operand_types[2] = {input_type, input_type};
	jive_output * operands[2] = {x, y};
	JIVE_DECLARE_CONTROL_TYPE(output_type);
	
	jive_node_init_(self, region,
		2, operand_types, operands,
		1, &output_type);
}

static jive_binop_reduction_path_t 
jive_bitcompare_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return jive_binop_reduction_constants;
	
	return jive_binop_reduction_none;
}

static jive_output * 
jive_bitcompare_reduce_operand_pair_(jive_binop_reduction_path_t path,
	const jive_node_class * cls_, const jive_node_attrs * attrs, jive_output * op1,
	jive_output * op2)
{
	jive_graph * graph = op1->node->graph;
	const jive_bitcomparison_operation_class * cls;
	cls = (const jive_bitcomparison_operation_class *) cls_;
	if (path == jive_binop_reduction_constants) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) op1->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) op2->node;
		
		size_t nbits = n1->attrs.nbits;
		char result = cls->compare_constants(n1->attrs.bits, n2->attrs.bits, nbits);
		
		switch(result) {
			case '0':
				return jive_control_false(graph);
			case '1':
				return jive_control_true(graph);
			default:
				return NULL;
		};
	}
	
	return NULL;
}

#define MAKE_CMPOP(name_, NAME, type_) \
\
static void \
jive_##name_##_node_init_( \
	jive_node * self, \
	jive_region * region, \
	jive_output * x, jive_output * y) \
{ \
	size_t nbits = 0; \
	nbits = ((const jive_bitstring_type *)jive_output_get_type(x))->nbits; \
	jive_bitcomparison_node_init_(self, region, x, y, nbits); \
} \
 \
static char * \
jive_##name_##_node_get_label_(const jive_node * self); \
 \
static jive_node * \
jive_##name_##_node_create_(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]); \
 \
const jive_bitcomparison_operation_class JIVE_##NAME##_NODE_ = { \
	.base = { /* jive_binary_operation_class */ \
		.base = { /* jive_node_class */ \
			.parent = &JIVE_BITCOMPARISON_NODE, \
			.name = #NAME, \
			.fini = jive_node_fini_, /* inherit */ \
			.get_default_normal_form = jive_binary_operation_get_default_normal_form_, /* inherit */ \
			.get_label = jive_##name_##_node_get_label_, /* override */ \
			.get_attrs = jive_node_get_attrs_, /* inherit */ \
			.match_attrs = jive_node_match_attrs_, /* inherit */ \
			.create = jive_##name_##_node_create_, /* override */ \
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */ \
		}, \
		\
		.flags = jive_binary_operation_none, \
		.single_apply_under = NULL, \
		.multi_apply_under = NULL, \
		.distributive_over = NULL, \
		.distributive_under = NULL, \
		\
		.can_reduce_operand_pair = jive_bitcompare_can_reduce_operand_pair_, /* override */ \
		.reduce_operand_pair = jive_bitcompare_reduce_operand_pair_ /* override */ \
	}, \
	.type = type_, \
	.compare_constants = jive_##name_##_compare_constants_ \
}; \
 \
static char * \
jive_##name_##_node_get_label_(const jive_node * self) \
{ \
	return strdup(#NAME); \
} \
 \
static jive_node * \
jive_##name_##_node_create_(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]) \
{ \
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node)); \
	node->class_ = &JIVE_##NAME##_NODE; \
	jive_##name_##_node_init_(node, region, operands[0], operands[1]); \
	return node; \
} \
 \
 \
jive_node * \
jive_##name_##_create( \
	struct jive_region * region, \
	jive_output * x, jive_output * y) \
{ \
	jive_output * operands[2] = {x, y}; \
	const jive_binary_operation_normal_form * nf = \
		(const jive_binary_operation_normal_form *) \
		jive_graph_get_nodeclass_form(region->graph, &JIVE_##NAME##_NODE); \
	return jive_binary_operation_normalized_create_new(nf, region, NULL, 2, operands)->node; \
} \
 \
jive_output * \
jive_##name_(jive_output * x, jive_output * y) \
{ \
	jive_output * operands[2] = {x, y}; \
	jive_region * region = jive_region_innermost(2, operands); \
	const jive_binary_operation_normal_form * nf = \
		(const jive_binary_operation_normal_form *) \
		jive_graph_get_nodeclass_form(region->graph, &JIVE_##NAME##_NODE); \
	return jive_binary_operation_normalized_create_new(nf, region, NULL, 2, operands); \
} \

MAKE_CMPOP(bitequal, BITEQUAL, jive_bitcmp_code_equal)
MAKE_CMPOP(bitnotequal, BITNOTEQUAL, jive_bitcmp_code_notequal)
MAKE_CMPOP(bitsless, BITSLESS, jive_bitcmp_code_sless)
MAKE_CMPOP(bituless, BITULESS, jive_bitcmp_code_uless)
MAKE_CMPOP(bitslesseq, BITSLESSEQ, jive_bitcmp_code_slesseq)
MAKE_CMPOP(bitulesseq, BITULESSEQ, jive_bitcmp_code_ulesseq)
MAKE_CMPOP(bitsgreater, BITSGREATER, jive_bitcmp_code_sgreater)
MAKE_CMPOP(bitugreater, BITUGREATER, jive_bitcmp_code_ugreater)
MAKE_CMPOP(bitsgreatereq, BITSGREATEREQ, jive_bitcmp_code_sgreatereq)
MAKE_CMPOP(bitugreatereq, BITUGREATEREQ, jive_bitcmp_code_ugreatereq)
