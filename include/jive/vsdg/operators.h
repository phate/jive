#ifndef JIVE_VSDG_OPERATORS_H
#define JIVE_VSDG_OPERATORS_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/operators/unary.h>

struct jive_region;

typedef struct jive_binary_operation_class jive_binary_operation_class;

typedef struct {
	size_t count;
	const jive_node_class * const * classes;
} jive_node_class_vector;

typedef enum jive_binary_operation_flags {
	jive_binary_operation_none = 0,
	jive_binary_operation_associative = 1,
	jive_binary_operation_commutative = 2
} jive_binary_operation_flags;

struct jive_binary_operation_class {
	jive_node_class base;
	
	/* algebraic properties */
	jive_binary_operation_flags flags;
	const jive_unary_operation_class * const * single_apply_under;
	const jive_unary_operation_class * const * multi_apply_under;
	const jive_binary_operation_class * const * distributive_over;
	const jive_binary_operation_class * const * distributive_under;
	
	bool (*can_reduce_operand_pair)(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);
	
	bool (*reduce_operand_pair)(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2);
};

jive_output *
jive_binary_operation_normalized_create(
	const jive_node_class * cls,
	struct jive_region * region,
	const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

extern const jive_binary_operation_class JIVE_BINARY_OPERATION_;
#define JIVE_BINARY_OPERATION (JIVE_BINARY_OPERATION_.base)

JIVE_EXPORTED_INLINE bool
jive_binary_operation_can_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	return ((const jive_binary_operation_class *)cls)->can_reduce_operand_pair(cls, attrs, op1, op2);
}

JIVE_EXPORTED_INLINE bool
jive_binary_operation_reduce_operand_pair(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2)
{
	return ((const jive_binary_operation_class *)cls)->reduce_operand_pair(cls, attrs, op1, op2);
}

/* inheritable default implementations for methods */

bool
jive_binary_operation_can_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);

bool
jive_binary_operation_reduce_operand_pair_(const jive_node_class * cls, const jive_node_attrs * attrs, jive_output ** op1, jive_output ** op2);

#endif
