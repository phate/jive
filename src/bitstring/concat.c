#include <jive/bitstring/concat.h>

#include <string.h>

#include <jive/common.h>

#include <jive/bitstring/constant.h>
#include <jive/bitstring/slice.h>
#include <jive/bitstring/type.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

static void
_jive_bitstring_multiop_node_init(
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
	
	_jive_node_init(self, region,
		noperands, operand_types, operands,
		1, &output_type);
}

static void
_jive_bitconcat_node_init(
	jive_node * self,
	jive_region * region,
	size_t noperands,
	jive_output * const operands[])
{
	size_t nbits = 0, n;
	for(n=0; n<noperands; n++)
		nbits += ((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits;
	_jive_bitstring_multiop_node_init(self, region, noperands, operands, nbits);
}

static char *
_jive_bitconcat_node_get_label(const jive_node * self);

static jive_node *
_jive_bitconcat_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);

static bool
jive_bitconcat_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2);

const jive_binary_operation_class JIVE_BITCONCAT_NODE_ = {
	.base = { /* jive_node_class */
		.parent = &JIVE_BINARY_OPERATION,
		.fini = _jive_node_fini, /* inherit */
		.get_label = _jive_bitconcat_node_get_label, /* override */
		.get_attrs = _jive_node_get_attrs, /* inherit */
		.match_attrs = _jive_node_match_attrs, /* inherit */
		.create = _jive_bitconcat_node_create, /* override */
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
	},
	
	.flags = jive_binary_operation_associative,
	.single_apply_under = NULL,
	.multi_apply_under = NULL,
	.distributive_over = NULL,
	.distributive_under = NULL,
	
	.can_reduce_operand_pair = jive_bitconcat_can_reduce_operand_pair_, /* override */
	.reduce_operand_pair = jive_bitconcat_reduce_operand_pair /* override */
};

static char *
_jive_bitconcat_node_get_label(const jive_node * self)
{
	return strdup("BITCONCAT");
}

static jive_node *
_jive_bitconcat_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITCONCAT_NODE;
	_jive_bitconcat_node_init(node, region, noperands, operands);
	return node;
}

static bool
jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;
	
	if (op1->node->class_ == &JIVE_BITSLICE_NODE && op2->node->class_ == &JIVE_BITSLICE_NODE) {
		jive_output * origin1 = op1->node->inputs[0]->origin;
		jive_output * origin2 = op2->node->inputs[1]->origin;
		
		if (origin1 != origin2) return false;
		
		const jive_bitslice_node * n1 = (const jive_bitslice_node *) op1->node;
		const jive_bitslice_node * n2 = (const jive_bitslice_node *) op2->node;
		
		return n1->attrs.high == n2->attrs.low;
		
		/* FIXME: support sign bit */
	}
	
	
	return false;
}

static bool
jive_bitconcat_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits + n2->attrs.nbits;
		char bits[nbits];
		memcpy(bits, n1->attrs.bits, n1->attrs.nbits);
		memcpy(bits + n1->attrs.nbits, n2->attrs.bits, n2->attrs.nbits);
		
		*op1 = jive_bitconstant(graph, nbits, bits);
		return true;
	}
	
	if ((*op1)->node->class_ == &JIVE_BITSLICE_NODE && (*op2)->node->class_ == &JIVE_BITSLICE_NODE) {
		const jive_bitslice_node * n1 = (const jive_bitslice_node *) (*op1)->node;
		jive_output * origin1 = (*op1)->node->inputs[0]->origin;
		
		const jive_bitslice_node * n2 = (const jive_bitslice_node *) (*op2)->node;
		jive_output * origin2 = (*op2)->node->inputs[0]->origin;
		
		if (origin1 != origin2) return false;
		
		*op1 = jive_bitslice(origin1, n1->attrs.low, n2->attrs.high);
		return true;
		
		/* FIXME: support sign bit */
	}
	
	return false;
}

jive_node *
jive_bitconcat_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const])
{
	return	jive_binary_operation_normalized_create(&JIVE_BITCONCAT_NODE, region, NULL, noperands, operands);
}

jive_output *
jive_bitconcat(size_t noperands, jive_output * operands[const])
{
	return jive_bitconcat_create(jive_region_innermost(noperands, operands), noperands, operands)->outputs[0];
}
