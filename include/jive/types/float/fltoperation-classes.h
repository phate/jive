/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_H
#define JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_H

#include <jive/types/float/value-representation.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>

typedef struct jive_fltbinary_operation_class jive_fltbinary_operation_class;
typedef struct jive_fltcomparison_operation_class jive_fltcomparison_operation_class;
typedef struct jive_fltunary_operation_class jive_fltunary_operation_class;

typedef enum jive_fltop_code {
	jive_fltop_code_invalid = 0,
	jive_fltop_code_sum = 1,
	jive_fltop_code_difference = 2,
	jive_fltop_code_product = 3,
	jive_fltop_code_quotient = 4,
	jive_fltop_code_negate = 5
} jive_fltop_code;

typedef enum jive_fltcmp_code {
	jive_fltcmp_code_invalid = 0,
	jive_fltcmp_code_less = 1,
	jive_fltcmp_code_lesseq = 2,
	jive_fltcmp_code_equal = 3,
	jive_fltcmp_code_notequal = 4,
	jive_fltcmp_code_greatereq = 5,
	jive_fltcmp_code_greater = 6
} jive_fltcmp_code;

struct jive_fltbinary_operation_class {
	jive_binary_operation_class base;
	jive_fltop_code type;
};

struct jive_fltunary_operation_class {
	jive_unary_operation_class base;
	jive_fltop_code type;
};

struct jive_fltcomparison_operation_class {
	jive_binary_operation_class base;
	jive_fltcmp_code type;
};

extern const jive_fltbinary_operation_class JIVE_FLTBINARY_NODE_;
#define JIVE_FLTBINARY_NODE (JIVE_FLTBINARY_NODE_.base.base)

extern const jive_fltunary_operation_class JIVE_FLTUNARY_NODE_;
#define JIVE_FLTUNARY_NODE (JIVE_FLTUNARY_NODE_.base.base)

extern const jive_fltcomparison_operation_class JIVE_FLTCOMPARISON_NODE_;
#define JIVE_FLTCOMPARISON_NODE (JIVE_FLTCOMPARISON_NODE_.base.base)

namespace jive {

/* Represents a unary operation on a float. */
class flt_unary_operation : public unary_operation {
public:
	virtual ~flt_unary_operation() noexcept;
};

/* Represents a binary operation on a float. */
class flt_binary_operation : public binary_operation {
public:
	virtual ~flt_binary_operation() noexcept;

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * arg1,
		const jive::output * arg2) const noexcept override;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * arg1,
		jive::output * arg2) const override;

	virtual flt::value_repr
	reduce_constants(
		const flt::value_repr & arg1,
		const flt::value_repr & arg2) const = 0;
};

/* Represents a comparison operation on a float. */
class flt_compare_operation : public binary_operation {
public:
	virtual ~flt_compare_operation() noexcept;

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_binop_reduction_path_t
	can_reduce_operand_pair(
		const jive::output * arg1,
		const jive::output * arg2) const noexcept override;

	virtual jive::output *
	reduce_operand_pair(
		jive_binop_reduction_path_t path,
		jive::output * arg1,
		jive::output * arg2) const override;

	virtual bool
	reduce_constants(
		const flt::value_repr & arg1,
		const flt::value_repr & arg2) const = 0;
};

namespace flt {
namespace detail {
template<
	value_repr eval_function(value_repr, value_repr),
	const jive_fltbinary_operation_class * cls,
	const char * name,
	jive_binary_operation_flags op_flags>
class make_binop final : public flt_binary_operation {
public:
	virtual ~make_binop() noexcept {}

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const make_binop * o = dynamic_cast<const make_binop *>(&other);
		return o != nullptr;
	}

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override
	{
		jive_node * node = jive::create_operation_node(*this);
		node->class_ = &cls->base.base;

		const jive::base::type * argument_types[2] = {
			&argument_type(0),
			&argument_type(1)
		};
		const jive::base::type * result_types[1] = {
			&result_type(0)
		};

		jive_node_init_(
			node, region,
			2, argument_types, arguments,
			1, result_types);

		return node;
	}

	static jive::output *
	normalized_create(jive::output * arg1, jive::output * arg2)
	{
		jive_graph * graph = arg1->node()->graph;
		make_binop op;
		jive::output * arguments[] = {arg1, arg2};
		return jive_binary_operation_create_normalized(
			&cls->base, graph, &op,
			2, arguments);
	}

	virtual jive_binary_operation_flags
	flags() const noexcept override
	{
		return op_flags;
	}

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override
	{
		return eval_function(arg1, arg2);
	}

	virtual std::string
	debug_string() const override
	{
		return name;
	}
};

template<
	bool eval_function(value_repr, value_repr),
	const jive_fltcomparison_operation_class * cls,
	const char * name,
	jive_binary_operation_flags op_flags>
class make_cmpop final : public flt_compare_operation {
public:
	virtual ~make_cmpop() noexcept {}

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const make_cmpop * o = dynamic_cast<const make_cmpop *>(&other);
		return o != nullptr;
	}

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override
	{
		jive_node * node = jive::create_operation_node(*this);
		node->class_ = &cls->base.base;

		const jive::base::type * argument_types[2] = {
			&argument_type(0),
			&argument_type(1)
		};
		const jive::base::type * result_types[1] = {
			&result_type(0)
		};

		jive_node_init_(
			node, region,
			2, argument_types, arguments,
			1, result_types);

		return node;
	}

	static jive::output *
	normalized_create(jive::output * arg1, jive::output * arg2)
	{
		jive_graph * graph = arg1->node()->graph;
		make_cmpop op;
		jive::output * arguments[] = {arg1, arg2};
		return jive_binary_operation_create_normalized(
			&cls->base, graph, &op,
			2, arguments);
	}

	virtual jive_binary_operation_flags
	flags() const noexcept override
	{
		return op_flags;
	}

	virtual bool
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override
	{
		return eval_function(arg1, arg2);
	}

	virtual std::string
	debug_string() const override
	{
		return name;
	}
};

}
}

}

#endif
