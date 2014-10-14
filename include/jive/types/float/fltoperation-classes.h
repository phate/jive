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

extern const jive_node_class JIVE_FLTBINARY_NODE;

extern const jive_node_class JIVE_FLTUNARY_NODE;

extern const jive_node_class JIVE_FLTCOMPARISON_NODE;

namespace jive {
namespace flt {

/* Represents a unary operation on a float. */
class unary_op : public base::unary_op {
public:
	virtual ~unary_op() noexcept;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	virtual flt::value_repr
	reduce_constant(
		const flt::value_repr & arg) const = 0;
};

/* Represents a binary operation on a float. */
class binary_op : public base::binary_op {
public:
	virtual ~binary_op() noexcept;

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
class compare_op : public base::binary_op {
public:
	virtual ~compare_op() noexcept;

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

namespace detail {

template<
	typename evaluator_functional,
	const jive_node_class * cls,
	const char * name>
class make_unop final : public unary_op {
public:
	virtual ~make_unop() noexcept {}

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const make_unop * o = dynamic_cast<const make_unop *>(&other);
		return o != nullptr;
	}

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override
	{
		jive_node * node = jive::create_operation_node(*this);
		node->class_ = cls;

		const jive::base::type * argument_types[2] = {
			&argument_type(0),
		};
		const jive::base::type * result_types[1] = {
			&result_type(0)
		};

		jive_node_init_(
			node, region,
			1, argument_types, arguments,
			1, result_types);

		return node;
	}

	static jive::output *
	normalized_create(jive::output * arg)
	{
		jive_graph * graph = arg->node()->graph;
		make_unop op;
		return jive_unary_operation_create_normalized(
			cls, graph, &op, arg);
	}

	virtual value_repr
	reduce_constant(const value_repr & arg) const override
	{
		return evaluator_(arg);
	}

	virtual std::string
	debug_string() const override
	{
		return name;
	}

	virtual std::unique_ptr<jive::operation> copy() const override
	{
		return std::unique_ptr<jive::operation>(new make_unop(*this));
	}

private:
	evaluator_functional evaluator_;
};

template<
	typename evaluator_functional,
	const jive_node_class * cls,
	const char * name,
	jive_binary_operation_flags op_flags>
class make_binop final : public binary_op {
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
		node->class_ = cls;

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
			cls, graph, &op,
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
		return evaluator_(arg1, arg2);
	}

	virtual std::string
	debug_string() const override
	{
		return name;
	}

	virtual std::unique_ptr<jive::operation> copy() const override
	{
		return std::unique_ptr<jive::operation>(new make_binop(*this));
	}

private:
	evaluator_functional evaluator_;
};

template<
	typename evaluator_functional,
	const jive_node_class * cls,
	const char * name,
	jive_binary_operation_flags op_flags>
class make_cmpop final : public compare_op {
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
		node->class_ = cls;

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
			cls, graph, &op,
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
		return evaluator_(arg1, arg2);
	}

	virtual std::string
	debug_string() const override
	{
		return name;
	}

	virtual std::unique_ptr<jive::operation> copy() const override
	{
		return std::unique_ptr<jive::operation>(new make_cmpop(*this));
	}

private:
	evaluator_functional evaluator_;
};

}
}
}

#endif
