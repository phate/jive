#ifndef JIVE_BITSTRING_ARITHMETIC_H
#define JIVE_BITSTRING_ARITHMETIC_H

#include <jive/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

typedef struct jive_bitbinary_operation_class jive_bitbinary_operation_class;
typedef struct jive_bitunary_operation_class jive_bitunary_operation_class;

typedef enum jive_bitop_code {
	jive_bitop_code_invalid = 0,
	jive_bitop_code_and = 1,
	jive_bitop_code_or = 2,
	jive_bitop_code_xor = 3,
	jive_bitop_code_sum = 4,
	jive_bitop_code_difference = 5,
	jive_bitop_code_product = 6,
	jive_bitop_code_uhiproduct = 7,
	jive_bitop_code_shiproduct = 8,
	jive_bitop_code_uquotient = 9,
	jive_bitop_code_squotient = 10,
	jive_bitop_code_umod = 11,
	jive_bitop_code_smod = 12,
	jive_bitop_code_shl = 13,
	jive_bitop_code_shr = 14,
	jive_bitop_code_ashr = 15,
	jive_bitop_code_negate = 16,
	jive_bitop_code_not = 17
} jive_bitop_code;

struct jive_bitbinary_operation_class {
	jive_binary_operation_class base;
	jive_bitop_code type;
};

struct jive_bitunary_operation_class {
	jive_unary_operation_class base;
	jive_bitop_code type;
};

extern const jive_bitbinary_operation_class JIVE_BITBINARY_NODE_;
#define JIVE_BITBINARY_NODE (JIVE_BITBINARY_NODE_.base.base)

extern const jive_bitunary_operation_class JIVE_BITUNARY_NODE_;
#define JIVE_BITUNARY_NODE (JIVE_BITUNARY_NODE_.base.base)

extern const jive_bitbinary_operation_class JIVE_BITAND_NODE_;
#define JIVE_BITAND_NODE (JIVE_BITAND_NODE_.base.base)

jive_node *
jive_bitand_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitand(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitand_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITAND_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITOR_NODE_;
#define JIVE_BITOR_NODE (JIVE_BITOR_NODE_.base.base)

jive_node *
jive_bitor_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitor(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitor_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITOR_NODE) return node;
	else return 0;
}


extern const jive_bitbinary_operation_class JIVE_BITXOR_NODE_;
#define JIVE_BITXOR_NODE (JIVE_BITXOR_NODE_.base.base)

jive_node *
jive_bitxor_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitxor(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitxor_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITXOR_NODE) return node;
	else return 0;
}


extern const jive_bitbinary_operation_class JIVE_BITSUM_NODE_;
#define JIVE_BITSUM_NODE (JIVE_BITSUM_NODE_.base.base)

jive_node *
jive_bitsum_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitsum(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_output *
jive_bitadd(size_t noperands, jive_output * operands[const])
{
	return jive_bitsum(noperands, operands);
}

JIVE_EXPORTED_INLINE jive_node *
jive_bitsum_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSUM_NODE) return node;
	else return 0;
}


extern const jive_bitbinary_operation_class JIVE_BITPRODUCT_NODE_;
#define JIVE_BITPRODUCT_NODE (JIVE_BITPRODUCT_NODE_.base.base)

jive_node *
jive_bitproduct_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitproduct(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_output *
jive_bitmultiply(size_t noperands, jive_output * operands[const])
{
	return jive_bitproduct(noperands, operands);
}

JIVE_EXPORTED_INLINE jive_node *
jive_bitproduct_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITPRODUCT_NODE) return node;
	else return 0;
}


extern const jive_bitbinary_operation_class JIVE_BITDIFFERENCE_NODE_;
#define JIVE_BITDIFFERENCE_NODE (JIVE_BITDIFFERENCE_NODE_.base.base)

jive_node *
jive_bitdifference_create(
	struct jive_region * region,
	jive_output * minuend, jive_output * subtrahend);

jive_output *
jive_bitdifference(jive_output * minuend, jive_output * subtrahend);

JIVE_EXPORTED_INLINE jive_node *
jive_bitdifference_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITDIFFERENCE_NODE) return node;
	else return 0;
}


extern const jive_bitbinary_operation_class JIVE_BITSHIPRODUCT_NODE_;
#define JIVE_BITSHIPRODUCT_NODE (JIVE_BITSHIPRODUCT_NODE_.base.base)

jive_node *
jive_bitshiproduct_create(
	struct jive_region * region,
	jive_output * factor1, jive_output * factor2);

jive_output *
jive_bitshiproduct(jive_output * factor1, jive_output * factor2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitshiproduct_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSHIPRODUCT_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITUHIPRODUCT_NODE_;
#define JIVE_BITUHIPRODUCT_NODE (JIVE_BITUHIPRODUCT_NODE_.base.base)

jive_node *
jive_bituhiproduct_create(
	struct jive_region * region,
	jive_output * factor1, jive_output * factor2);

jive_output *
jive_bituhiproduct(jive_output * factor1, jive_output * factor2);

JIVE_EXPORTED_INLINE jive_node *
jive_bituhiproduct_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITUHIPRODUCT_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITUQUOTIENT_NODE_;
#define JIVE_BITUQUOTIENT_NODE (JIVE_BITUQUOTIENT_NODE_.base.base)

jive_node *
jive_bituquotient_create(
	struct jive_region * region,
	jive_output * dividend, jive_output * divisor);

jive_output *
jive_bituquotient(jive_output * dividend, jive_output * divisor);

JIVE_EXPORTED_INLINE jive_node *
jive_bituquotient_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITUQUOTIENT_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITSQUOTIENT_NODE_;
#define JIVE_BITSQUOTIENT_NODE (JIVE_BITSQUOTIENT_NODE_.base.base)

jive_node *
jive_bitsquotient_create(
	struct jive_region * region,
	jive_output * dividend, jive_output * divisor);

jive_output *
jive_bitsquotient(jive_output * dividend, jive_output * divisor);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsquotient_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSQUOTIENT_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITUMOD_NODE_;
#define JIVE_BITUMOD_NODE (JIVE_BITUMOD_NODE_.base.base)

jive_node *
jive_bitumod_create(
	struct jive_region * region,
	jive_output * operand1, jive_output * operand2);

jive_output *
jive_bitumod(jive_output * operand1, jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitumod_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITUMOD_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITSMOD_NODE_;
#define JIVE_BITSMOD_NODE (JIVE_BITSMOD_NODE_.base.base)

jive_node *
jive_bitsmod_create(
	struct jive_region * region,
	jive_output * operand1, jive_output * operand2);

jive_output *
jive_bitsmod(jive_output * operand1, jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsmod_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSMOD_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITSHL_NODE_;
#define JIVE_BITSHL_NODE (JIVE_BITSHL_NODE_.base.base)

jive_node *
jive_bitshl_create(
	struct jive_region * region,
	jive_output * operand, jive_output * shift);

jive_output *
jive_bitshl(jive_output * operand, jive_output * shift);

JIVE_EXPORTED_INLINE jive_node *
jive_bitshl_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSHL_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITSHR_NODE_;
#define JIVE_BITSHR_NODE (JIVE_BITSHR_NODE_.base.base)

jive_node *
jive_bitshr_create(
	struct jive_region * region,
	jive_output * operand, jive_output * shift);

jive_output *
jive_bitshr(jive_output * operand, jive_output * shift);

JIVE_EXPORTED_INLINE jive_node *
jive_bitshr_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSHR_NODE) return node;
	else return 0;
}

extern const jive_bitbinary_operation_class JIVE_BITASHR_NODE_;
#define JIVE_BITASHR_NODE (JIVE_BITASHR_NODE_.base.base)

jive_node *
jive_bitashr_create(
	struct jive_region * region,
	jive_output * operand, jive_output * shift);

jive_output *
jive_bitashr(jive_output * operand, jive_output * shift);

JIVE_EXPORTED_INLINE jive_node *
jive_bitashr_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITASHR_NODE) return node;
	else return 0;
}

extern const jive_bitunary_operation_class JIVE_BITNEGATE_NODE_;
#define JIVE_BITNEGATE_NODE (JIVE_BITNEGATE_NODE_.base.base)

/**
	\brief Create bitnegate
	\param region Region to put node into
	\param origin Input value
	\returns Bitstring value representing negate
	
	Create new bitnegate node. Computes the two's complement
	of the input bitstring.
*/
jive_node *
jive_bitnegate_create(struct jive_region * region, jive_output * origin);

/**
	\brief Create bitnegate
	\param operand Input value
	\returns Bitstring value representing negate
	
	Convenience function to create negation of value.
*/
jive_output *
jive_bitnegate(jive_output * operand);

JIVE_EXPORTED_INLINE jive_node *
jive_bitnegate_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITNEGATE_NODE) return node;
	else return 0;
}

extern const jive_bitunary_operation_class JIVE_BITNOT_NODE_;
#define JIVE_BITNOT_NODE (JIVE_BITNOT_NODE_.base.base)

/**
	\brief Create bitnot
	\param region Region to put node into
	\param origin Input value
	\returns Bitstring value representing not
	
	Create new bitnot node. Computes the two's complement
	of the input bitstring.
*/
jive_node *
jive_bitnot_create(struct jive_region * region, jive_output * origin);

/**
	\brief Create bitnot
	\param operand Input value
	\returns Bitstring value representing not
	
	Convenience function to create negation of value.
*/
jive_output *
jive_bitnot(jive_output * operand);

JIVE_EXPORTED_INLINE jive_node *
jive_bitnot_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITNOT_NODE) return node;
	else return 0;
}


#endif
