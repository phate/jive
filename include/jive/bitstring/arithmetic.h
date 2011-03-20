#ifndef JIVE_BITSTRING_ARITHMETIC_H
#define JIVE_BITSTRING_ARITHMETIC_H

#include <jive/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_binary_operation_class JIVE_BITAND_NODE_;
#define JIVE_BITAND_NODE (JIVE_BITAND_NODE_.base)

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

extern const jive_binary_operation_class JIVE_BITOR_NODE_;
#define JIVE_BITOR_NODE (JIVE_BITOR_NODE_.base)

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


extern const jive_binary_operation_class JIVE_BITXOR_NODE_;
#define JIVE_BITXOR_NODE (JIVE_BITXOR_NODE_.base)

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


extern const jive_binary_operation_class JIVE_BITSUM_NODE_;
#define JIVE_BITSUM_NODE (JIVE_BITSUM_NODE_.base)

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


extern const jive_binary_operation_class JIVE_BITPRODUCT_NODE_;
#define JIVE_BITPRODUCT_NODE (JIVE_BITPRODUCT_NODE_.base)

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

#endif
