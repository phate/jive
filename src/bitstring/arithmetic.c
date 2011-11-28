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
	.type = jive_bitop_code_invalid
};

static void
jive_bitstring_multiop_node_init_(
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
	
	jive_node_init_(self, region,
		noperands, operand_types, operands,
		1, &output_type);
}

/* create boiler-plate code for generic binary operation */
#define MAKE_BASE_BINOP(name_, NAME, type_, flags_) \
\
static void \
jive_##name_##_node_init_( \
	jive_node * self, \
	jive_region * region, \
	size_t noperands, \
	jive_output * const operands[]) \
{ \
	if (!jive_output_isinstance(operands[0], &JIVE_BITSTRING_OUTPUT)) { \
		jive_context_fatal_error(region->graph->context, "Type mismatch: '" #name_ "' node requires bitstring operands"); \
	} \
	size_t nbits = ((jive_bitstring_output *)operands[0])->type.nbits; \
	jive_bitstring_multiop_node_init_(self, region, noperands, operands, nbits); \
} \
 \
static char * \
jive_##name_##_node_get_label_(const jive_node * self); \
 \
static jive_node * \
jive_##name_##_node_create_(struct jive_region * region, const jive_node_attrs * attrs, \
	size_t noperands, struct jive_output * const operands[]); \
 \
static bool \
jive_##name_##_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2); \
 \
static bool \
jive_##name_##_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2); \
 \
const jive_bitbinary_operation_class JIVE_##NAME##_NODE_ = { \
	.base = { /* jive_bitbinary_operation_class */ \
		.base = { /* jive_node_class */ \
			.parent = &JIVE_BITBINARY_NODE, \
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
		.flags = flags_, \
		.single_apply_under = NULL, \
		.multi_apply_under = NULL, \
		.distributive_over = NULL, \
		.distributive_under = NULL, \
		\
		.can_reduce_operand_pair = jive_##name_##_can_reduce_operand_pair_, /* override */ \
		.reduce_operand_pair = jive_##name_##_reduce_operand_pair_ /* override */ \
	}, \
	.type = type_ \
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
	jive_##name_##_node_init_(node, region, noperands, operands); \
	return node; \
} \

/* create boiler-plate code for binary operation that is non-associative and non-commutative
FIXME: no reductions for these types of nodes so far */
#define MAKE_BINOP(name_, NAME, type_) \
 \
MAKE_BASE_BINOP(name_, NAME, type_, jive_binary_operation_none) \
 \
static bool \
jive_##name_##_can_reduce_operand_pair_(const jive_node_class * cls, \
	const jive_node_attrs * attrs, jive_output * op1, jive_output * op2) \
{ \
	return false; \
} \
 \
static bool \
jive_##name_##_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, \
	jive_output ** op1, jive_output ** op2) \
{ \
	return false; \
} \
 \
jive_output * \
jive_##name_(jive_output * operand1, jive_output * operand2) \
{ \
	jive_output * operands[] = {operand1, operand2}; \
	jive_region * region = jive_region_innermost(2, operands); \
	const jive_binary_operation_normal_form * nf = \
		(const jive_binary_operation_normal_form *) \
		jive_graph_get_nodeclass_form(region->graph, &JIVE_##NAME##_NODE); \
	return jive_binary_operation_normalized_create_new(nf, region, NULL, 2, operands); \
} \
 \
jive_node * \
jive_##name_##_create( \
	struct jive_region * region, \
	jive_output * operand1, jive_output * operand2) \
{ \
	jive_output * operands[] = {operand1, operand2}; \
	const jive_binary_operation_normal_form * nf = \
		(const jive_binary_operation_normal_form *) \
		jive_graph_get_nodeclass_form(region->graph, &JIVE_##NAME##_NODE); \
	return jive_binary_operation_normalized_create_new(nf, region, NULL, 2, operands)->node; \
} \

#define MAKE_ASSOCIATIVE_COMMUTATIVE_BINOP(name_, NAME, type_) \
 \
MAKE_BASE_BINOP(name_, NAME, type_, jive_binary_operation_associative | jive_binary_operation_commutative) \
 \
jive_output * \
jive_##name_(size_t noperands, jive_output * operands[const]) \
{ \
	jive_region * region = jive_region_innermost(noperands, operands); \
	const jive_binary_operation_normal_form * nf = \
		(const jive_binary_operation_normal_form *) \
		jive_graph_get_nodeclass_form(region->graph, &JIVE_##NAME##_NODE); \
	return jive_binary_operation_normalized_create_new(nf, region, NULL, noperands, operands); \
} \
 \
jive_node * \
jive_##name_##_create( \
	struct jive_region * region, \
	size_t noperands, struct jive_output * operands[const]) \
{ \
	const jive_binary_operation_normal_form * nf = \
		(const jive_binary_operation_normal_form *) \
		jive_graph_get_nodeclass_form(region->graph, &JIVE_##NAME##_NODE); \
	return jive_binary_operation_normalized_create_new(nf, region, NULL, noperands, operands)->node; \
} \

static bool
jive_bitand_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	if (op1->node->class_ == &JIVE_BITCONSTANT_NODE && op2->node->class_ == &JIVE_BITCONSTANT_NODE)
		return true;

	return false;
}

static bool
jive_bitand_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
	
		size_t nbits = n1->attrs.nbits;	
		char bits[nbits];
		jive_multibit_and(bits, n1->attrs.bits, n2->attrs.bits, nbits);	

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
jive_bitor_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;	
		char bits[nbits];
		jive_multibit_or(bits, n1->attrs.bits, n2->attrs.bits, nbits);	
		
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
jive_bitxor_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	jive_graph * graph = (*op1)->node->graph;
	
	if ((*op1)->node->class_ == &JIVE_BITCONSTANT_NODE && (*op2)->node->class_ == &JIVE_BITCONSTANT_NODE) {
		jive_bitconstant_node * n1 = (jive_bitconstant_node *) (*op1)->node;
		jive_bitconstant_node * n2 = (jive_bitconstant_node *) (*op2)->node;
		
		size_t nbits = n1->attrs.nbits;
		char bits[nbits];
		jive_multibit_xor(bits, n1->attrs.bits, n2->attrs.bits, nbits);
		
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
jive_bitsum_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
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
jive_bitproduct_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
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

MAKE_ASSOCIATIVE_COMMUTATIVE_BINOP(bitand, BITAND, jive_bitop_code_and)
MAKE_ASSOCIATIVE_COMMUTATIVE_BINOP(bitor, BITOR, jive_bitop_code_or)
MAKE_ASSOCIATIVE_COMMUTATIVE_BINOP(bitxor, BITXOR, jive_bitop_code_xor)
MAKE_ASSOCIATIVE_COMMUTATIVE_BINOP(bitsum, BITSUM, jive_bitop_code_sum)
MAKE_ASSOCIATIVE_COMMUTATIVE_BINOP(bitproduct, BITPRODUCT, jive_bitop_code_product)
MAKE_BINOP(bitdifference, BITDIFFERENCE, jive_bitop_code_difference)
MAKE_BINOP(bitshiproduct, BITSHIPRODUCT, jive_bitop_code_shiproduct)
MAKE_BINOP(bituhiproduct, BITUHIPRODUCT, jive_bitop_code_uhiproduct)
MAKE_BINOP(bituquotient, BITUQUOTIENT, jive_bitop_code_uquotient)
MAKE_BINOP(bitsquotient, BITSQUOTIENT, jive_bitop_code_squotient)
MAKE_BINOP(bitumod, BITUMOD, jive_bitop_code_umod)
MAKE_BINOP(bitsmod, BITSMOD, jive_bitop_code_smod)
MAKE_BINOP(bitshl, BITSHL, jive_bitop_code_shl)
MAKE_BINOP(bitshr, BITSHR, jive_bitop_code_shr)
MAKE_BINOP(bitashr, BITASHR, jive_bitop_code_ashr)

/* unary operations */

const jive_bitunary_operation_class JIVE_BITUNARY_NODE_ = {
	.base = { /* jive_unary_operation_class */
		.base = { /* jive_node_class */
			.parent = &JIVE_UNARY_OPERATION,
			.name = "BITUNARY",
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_node_get_label_, /* inherit */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_node_create_, /* inherit */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
		},
		
		.single_apply_over = NULL,
		.multi_apply_over = NULL,
		
		.can_reduce_operand = jive_unary_operation_can_reduce_operand_ /* inherit */,
		.reduce_operand = jive_unary_operation_reduce_operand_ /* inherit */
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
			.parent = &JIVE_BITUNARY_NODE,
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_bitnegate_node_get_label_, /* override */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_bitnegate_node_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
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
	jive_node_init_(self, region,
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
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(region->graph, &JIVE_BITNEGATE_NODE);
	
	return jive_unary_operation_normalized_create(nf, region, NULL, operand)->node;
}

jive_output *
jive_bitnegate(jive_output * operand)
{
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_BITNEGATE_NODE);
	
	return jive_unary_operation_normalized_create(nf, operand->node->region, NULL, operand);
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
			.parent = &JIVE_BITUNARY_NODE,
			.fini = jive_node_fini_, /* inherit */
			.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
			.get_label = jive_bitnot_node_get_label_, /* override */
			.get_attrs = jive_node_get_attrs_, /* inherit */
			.match_attrs = jive_node_match_attrs_, /* inherit */
			.create = jive_bitnot_node_create_, /* override */
			.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
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
	jive_node_init_(self, region,
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
		jive_multibit_not(bits, node->attrs.bits, node->attrs.nbits);
		
		*operand_ = jive_bitconstant(node->base.graph, node->attrs.nbits, bits);
		return true;
	}
	
	return false;
}

jive_node *
jive_bitnot_create(struct jive_region * region, jive_output * operand)
{
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_BITNOT_NODE);
	
	return jive_unary_operation_normalized_create(nf, region, NULL, operand)->node;
}

jive_output *
jive_bitnot(jive_output * operand)
{
	const jive_unary_operation_normal_form * nf =
		(const jive_unary_operation_normal_form *)
		jive_graph_get_nodeclass_form(operand->node->graph, &JIVE_BITNOT_NODE);
	
	return jive_unary_operation_normalized_create(nf, operand->node->region, NULL, operand);
}

