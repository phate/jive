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
	jive_bitop_code_hiproduct = 7,
	jive_bitop_code_uquotient = 8,
	jive_bitop_code_squotient = 9,
	jive_bitop_code_mod = 10,
	jive_bitop_code_shl = 11,
	jive_bitop_code_shr = 12,
	jive_bitop_code_ashr = 13,
	jive_bitop_code_negate = 14,
	jive_bitop_code_not = 15
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

static inline jive_node *
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

static inline jive_node *
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

static inline jive_node *
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

static inline jive_output *
jive_bitadd(size_t noperands, jive_output * operands[const])
{
	return jive_bitsum(noperands, operands);
}

static inline jive_node *
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

static inline jive_output *
jive_bitmultiply(size_t noperands, jive_output * operands[const])
{
	return jive_bitproduct(noperands, operands);
}

static inline jive_node *
jive_bitproduct_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITPRODUCT_NODE) return node;
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

static inline jive_node *
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

static inline jive_node *
jive_bitnot_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITNOT_NODE) return node;
	else return 0;
}


#endif
