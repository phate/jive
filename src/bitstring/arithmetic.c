#include <jive/bitstring/arithmetic.h>

#include <string.h>

#include <jive/common.h>

#include <jive/bitstring/bitops-private.h>
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
	JIVE_DECLARE_BITSTRING_TYPE(output_type, nbits);
	size_t n;
	
	for(n=0; n<noperands; n++)
		operand_types[n] = output_type;
	
	_jive_node_init(self, region,
		noperands, operand_types, operands,
		1, &output_type);
}

#define MAKE_MULTIOP(name, NAME) \
\
static void \
_jive_##name##_node_init( \
	jive_node * self, \
	jive_region * region, \
	size_t noperands, \
	jive_output * const operands[]) \
{ \
	size_t nbits = 0, n; \
	for(n=0; n<noperands; n++) \
		nbits += ((const jive_bitstring_type *)jive_output_get_type(operands[n]))->nbits; \
	_jive_bitstring_multiop_node_init(self, region, noperands, operands, nbits); \
} \
 \
static char * \
_jive_##name##_node_get_label(const jive_node * self); \
 \
static jive_node * \
_jive_##name##_node_create(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]); \
 \
static bool \
jive_##name##_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2); \
 \
static bool \
jive_##name##_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2); \
 \
const jive_binary_operation_class JIVE_##NAME##_NODE_ = { \
	.base = { /* jive_node_class */ \
		.parent = &JIVE_BINARY_OPERATION, \
		.fini = _jive_node_fini, /* inherit */ \
		.get_label = _jive_##name##_node_get_label, /* override */ \
		.get_attrs = _jive_node_get_attrs, /* inherit */ \
		.match_attrs = _jive_node_match_attrs, /* inherit */ \
		.create = _jive_##name##_node_create, /* override */ \
		.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */ \
	}, \
	 \
	.flags = jive_binary_operation_associative | jive_binary_operation_commutative, \
	.single_apply_under = NULL, \
	.multi_apply_under = NULL, \
	.distributive_over = NULL, \
	.distributive_under = NULL, \
	 \
	.can_reduce_operand_pair = jive_##name##_can_reduce_operand_pair_, /* override */ \
	.reduce_operand_pair = jive_##name##_reduce_operand_pair /* override */ \
}; \
 \
static char * \
_jive_##name##_node_get_label(const jive_node * self) \
{ \
	return strdup(#NAME); \
} \
 \
static jive_node * \
_jive_##name##_node_create(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]) \
{ \
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node)); \
	node->class_ = &JIVE_##NAME##_NODE; \
	_jive_##name##_node_init(node, region, noperands, operands); \
	return node; \
} \
 \
 \
jive_node * \
jive_##name##_create( \
	struct jive_region * region, \
	size_t noperands, struct jive_output * operands[const]) \
{ \
	return	jive_binary_operation_normalized_create(&JIVE_##NAME##_NODE, region, NULL, noperands, operands); \
} \
 \
jive_output * \
jive_##name(size_t noperands, jive_output * operands[const]) \
{ \
	return jive_##name##_create(jive_region_innermost(noperands, operands), noperands, operands)->outputs[0]; \
} \

MAKE_MULTIOP(bitand, BITAND)
MAKE_MULTIOP(bitor, BITOR)
MAKE_MULTIOP(bitxor, BITXOR)
MAKE_MULTIOP(bitsum, BITSUM)
MAKE_MULTIOP(bitproduct, BITPRODUCT)

static bool
jive_bitand_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;

	return false;
}

static bool
jive_bitand_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;
		char bits[nbits];
		size_t n;
		for(n = 0; n<nbits; n++)
			bits[n] = jive_logic_and(n1->attrs.bits[n], n2->attrs.bits[n]);
		
		*op1 = jive_bitconstant(graph, nbits, bits);
		return true;
	}
	
	return false;
}

static bool
jive_bitor_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;

	return false;
}

static bool
jive_bitor_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;
		char bits[nbits];
		size_t n;
		for(n = 0; n<nbits; n++)
			bits[n] = jive_logic_or(n1->attrs.bits[n], n2->attrs.bits[n]);
		
		*op1 = jive_bitconstant(graph, nbits, bits);
		return true;
	}
	
	return false;
}

static bool
jive_bitxor_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;

	return false;
}

static bool
jive_bitxor_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;
		char bits[nbits];
		size_t n;
		for(n = 0; n<nbits; n++)
			bits[n] = jive_logic_xor(n1->attrs.bits[n], n2->attrs.bits[n]);
		
		*op1 = jive_bitconstant(graph, nbits, bits);
		return true;
	}
	
	return false;
}

static bool
jive_bitsum_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;

	return false;
}

static bool
jive_bitsum_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;
		char bits[nbits];
		jive_multibit_sum(bits, n1->attrs.bits, n2->attrs.bits, nbits);
		
		*op1 = jive_bitconstant(graph, nbits, bits);
		return true;
	}
	
	return false;
}

static bool
jive_bitproduct_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;

	return false;
}

static bool
jive_bitproduct_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;
		char bits[nbits];
		jive_multibit_multiply(bits, nbits, n1->attrs.bits, nbits, n2->attrs.bits, nbits);
		
		*op1 = jive_bitconstant(graph, nbits, bits);
		return true;
	}
	
	return false;
}
