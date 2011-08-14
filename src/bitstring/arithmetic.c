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

const jive_bitbinary_operation_class JIVE_BITBINARY_NODE_ = {
	.base = { /* jive_binary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_BINARY_OPERATION,
			.name = "BITBINARY",
			.fini = _jive_node_fini, /* inherit */
			.get_label = _jive_node_get_label, /* inherit */
			.get_attrs = _jive_node_get_attrs, /* inherit */
			.match_attrs = _jive_node_match_attrs, /* inherit */
			.create = _jive_node_create, /* inherit */
			.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
		},
		
		.flags = jive_binary_operation_none,
		.single_apply_under = NULL,
		.multi_apply_under = NULL,
		.distributive_over = NULL,
		.distributive_under = NULL,
		
		.can_reduce_operand_pair = jive_binary_operation_can_reduce_operand_pair_ /* inherit */,
		.reduce_operand_pair = jive_binary_operation_reduce_operand_pair /* inherit */
	},
	.type = jive_bitop_code_invalid
};

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

#define MAKE_MULTIOP(name_, NAME, type_) \
\
static void \
_jive_##name_##_node_init( \
	jive_node * self, \
	jive_region * region, \
	size_t noperands, \
	jive_output * const operands[]) \
{ \
	size_t nbits = 0; \
	nbits = ((const jive_bitstring_type *)jive_output_get_type(operands[0]))->nbits; \
	_jive_bitstring_multiop_node_init(self, region, noperands, operands, nbits); \
} \
 \
static char * \
_jive_##name_##_node_get_label(const jive_node * self); \
 \
static jive_node * \
_jive_##name_##_node_create(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]); \
 \
static bool \
jive_##name_##_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2); \
 \
static bool \
jive_##name_##_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2); \
 \
const jive_bitbinary_operation_class JIVE_##NAME##_NODE_ = { \
	.base = { /* jive_bitbinary_operation_class */ \
		.base = { /* jive_node_class */ \
			.parent = &JIVE_BITBINARY_NODE, \
			.name = #NAME, \
			.fini = _jive_node_fini, /* inherit */ \
			.get_label = _jive_##name_##_node_get_label, /* override */ \
			.get_attrs = _jive_node_get_attrs, /* inherit */ \
			.match_attrs = _jive_node_match_attrs, /* inherit */ \
			.create = _jive_##name_##_node_create, /* override */ \
			.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */ \
		}, \
		\
		.flags = jive_binary_operation_associative | jive_binary_operation_commutative, \
		.single_apply_under = NULL, \
		.multi_apply_under = NULL, \
		.distributive_over = NULL, \
		.distributive_under = NULL, \
		\
		.can_reduce_operand_pair = jive_##name_##_can_reduce_operand_pair_, /* override */ \
		.reduce_operand_pair = jive_##name_##_reduce_operand_pair /* override */ \
	}, \
	.type = type_ \
}; \
 \
static char * \
_jive_##name_##_node_get_label(const jive_node * self) \
{ \
	return strdup(#NAME); \
} \
 \
static jive_node * \
_jive_##name_##_node_create(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]) \
{ \
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node)); \
	node->class_ = &JIVE_##NAME##_NODE; \
	_jive_##name_##_node_init(node, region, noperands, operands); \
	return node; \
} \
 \
 \
jive_node * \
jive_##name_##_create( \
	struct jive_region * region, \
	size_t noperands, struct jive_output * operands[const]) \
{ \
	return jive_binary_operation_normalized_create(&JIVE_##NAME##_NODE, region, NULL, noperands, operands)->node; \
} \
 \
jive_output * \
jive_##name_(size_t noperands, jive_output * operands[const]) \
{ \
	jive_region * region = jive_region_innermost(noperands, operands); \
	return jive_binary_operation_normalized_create(&JIVE_##NAME##_NODE, region, NULL, noperands, operands); \
} \

MAKE_MULTIOP(bitand, BITAND, jive_bitop_code_and)
MAKE_MULTIOP(bitor, BITOR, jive_bitop_code_or)
MAKE_MULTIOP(bitxor, BITXOR, jive_bitop_code_xor)
MAKE_MULTIOP(bitsum, BITSUM, jive_bitop_code_sum)
MAKE_MULTIOP(bitproduct, BITPRODUCT, jive_bitop_code_product)

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

const jive_bitunary_operation_class JIVE_BITUNARY_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_UNARY_OPERATION,
			.name = "BITUNARY",
			.fini = _jive_node_fini, /* inherit */
			.get_label = _jive_node_get_label, /* inherit */
			.get_attrs = _jive_node_get_attrs, /* inherit */
			.match_attrs = _jive_node_match_attrs, /* inherit */
			.create = _jive_node_create, /* inherit */
			.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
		},
		
		.single_apply_over = NULL,
		.multi_apply_over = NULL,
		
		.can_reduce_operand = jive_unary_operation_can_reduce_operand_ /* inherit */,
		.reduce_operand = jive_unary_operation_reduce_operand /* inherit */
	},
	.type = jive_bitop_code_invalid
};

static void
jive_bitnegate_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * origin);

static char *
jive_bitnegate_node_get_label_(const jive_node * self);

static jive_node *
jive_bitnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_bitnegate_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * operand);

static bool
jive_bitnegate_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** operand);

const jive_bitunary_operation_class JIVE_BITNEGATE_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_UNARY_OPERATION,
			.fini = _jive_node_fini, /* inherit */
			.get_label = jive_bitnegate_node_get_label_, /* override */
			.get_attrs = _jive_node_get_attrs, /* inherit */
			.match_attrs = _jive_node_match_attrs, /* inherit */
			.create = jive_bitnegate_node_create_, /* override */
			.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
		},
		
		.single_apply_over = NULL,
		.multi_apply_over = NULL,
		
		.can_reduce_operand = jive_bitnegate_can_reduce_operand_, /* override */
		.reduce_operand = jive_bitnegate_reduce_operand_ /* override */
	},
	.type = jive_bitop_code_negate
};

static void
jive_bitnegate_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * origin)
{
	size_t nbits = ((jive_bitstring_output *) origin)->type.nbits;
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	_jive_node_init(self, region,
		1, &type, &origin,
		1, &type);
}

static char *
jive_bitnegate_node_get_label_(const jive_node * self_)
{
	return strdup("BITNEGATE");
}

static jive_node *
jive_bitnegate_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITNEGATE_NODE;
	jive_bitnegate_node_init_(node, region, operands[0]);
	return node;
}

static bool
jive_bitnegate_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand_)
{
	if (operand_->node->class_ == &JIVE_BITNEGATE_NODE) return true;
	if (operand_->node->class_ == &JIVE_BITCONSTANT_NODE) return true;
	
	return false;
}

static bool
jive_bitnegate_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand_)
{
	jive_bitstring_output * operand = (jive_bitstring_output *) *operand_;
	
	if (operand->base.base.node->class_ == &JIVE_BITNEGATE_NODE) {
		jive_node * node = operand->base.base.node;
		*operand_ = node->inputs[0]->origin;
		return true;
	}
	
	if (operand->base.base.node->class_ == &JIVE_BITCONSTANT_NODE) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->base.base.node;
		char bits[node->attrs.nbits];
		
		jive_multibit_negate(bits, node->attrs.bits, node->attrs.nbits);
		
		*operand_ = jive_bitconstant(node->base.graph, node->attrs.nbits, bits);
		return true;
	}
	
	return false;
}

jive_node *
jive_bitnegate_create(struct jive_region * region, jive_output * operand)
{
	return jive_unary_operation_normalized_create(&JIVE_BITNEGATE_NODE, region, NULL, operand)->node;
}

jive_output *
jive_bitnegate(jive_output * operand)
{
	return jive_unary_operation_normalized_create(&JIVE_BITNEGATE_NODE, operand->node->region, NULL, operand);
}

static void
jive_bitnot_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * origin);

static char *
jive_bitnot_node_get_label_(const jive_node * self);

static jive_node *
jive_bitnot_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static bool
jive_bitnot_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * operand);

static bool
jive_bitnot_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** operand);

const jive_bitunary_operation_class JIVE_BITNOT_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_UNARY_OPERATION,
			.fini = _jive_node_fini, /* inherit */
			.get_label = jive_bitnot_node_get_label_, /* override */
			.get_attrs = _jive_node_get_attrs, /* inherit */
			.match_attrs = _jive_node_match_attrs, /* inherit */
			.create = jive_bitnot_node_create_, /* override */
			.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
		},
		
		.single_apply_over = NULL,
		.multi_apply_over = NULL,
		
		.can_reduce_operand = jive_bitnot_can_reduce_operand_, /* override */
		.reduce_operand = jive_bitnot_reduce_operand_ /* override */
	},
	.type = jive_bitop_code_not
};

static void
jive_bitnot_node_init_(
	jive_node * self,
	jive_region * region,
	jive_output * origin)
{
	size_t nbits = ((jive_bitstring_output *) origin)->type.nbits;
	JIVE_DECLARE_BITSTRING_TYPE(type, nbits);
	_jive_node_init(self, region,
		1, &type, &origin,
		1, &type);
}

static char *
jive_bitnot_node_get_label_(const jive_node * self_)
{
	return strdup("BITNOT");
}

static jive_node *
jive_bitnot_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->class_ = &JIVE_BITNOT_NODE;
	jive_bitnot_node_init_(node, region, operands[0]);
	return node;
}

static bool
jive_bitnot_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand_)
{
	if (operand_->node->class_ == &JIVE_BITNOT_NODE) return true;
	if (operand_->node->class_ == &JIVE_BITCONSTANT_NODE) return true;
	
	return false;
}

static bool
jive_bitnot_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output ** operand_)
{
	jive_bitstring_output * operand = (jive_bitstring_output *) *operand_;
	
	if (operand->base.base.node->class_ == &JIVE_BITNOT_NODE) {
		jive_node * node = operand->base.base.node;
		*operand_ = node->inputs[0]->origin;
		return true;
	}
	
	if (operand->base.base.node->class_ == &JIVE_BITCONSTANT_NODE) {
		const jive_bitconstant_node * node = (const jive_bitconstant_node *) operand->base.base.node;
		char bits[node->attrs.nbits];
		size_t n;
		for(n = 0; n < node->attrs.nbits; n++)
			bits[n] = jive_logic_xor(node->attrs.bits[n], '1');
		
		*operand_ = jive_bitconstant(node->base.graph, node->attrs.nbits, bits);
		return true;
	}
	
	return false;
}

jive_node *
jive_bitnot_create(struct jive_region * region, jive_output * operand)
{
	return jive_unary_operation_normalized_create(&JIVE_BITNOT_NODE, region, NULL, operand)->node;
}

jive_output *
jive_bitnot(jive_output * operand)
{
	return jive_unary_operation_normalized_create(&JIVE_BITNOT_NODE, operand->node->region, NULL, operand);
}

