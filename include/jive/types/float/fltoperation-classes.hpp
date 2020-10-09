/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_HPP
#define JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_HPP

#include <jive/rvsdg/binary.hpp>
#include <jive/rvsdg/simple-node.hpp>
#include <jive/rvsdg/unary.hpp>
#include <jive/types/bitstring/type.hpp>
#include <jive/types/float/flttype.hpp>
#include <jive/types/float/value-representation.hpp>

namespace jive {
namespace flt {

/* Represents a unary operation on a float. */
class unary_op : public jive::unary_op {
public:
	virtual
	~unary_op() noexcept;

	inline
	unary_op()
	: jive::unary_op(flt::type(), flt::type())
	{}

	virtual jive_unop_reduction_path_t
	can_reduce_operand(const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(jive_unop_reduction_path_t path, jive::output * arg) const override;

	virtual flt::value_repr
	reduce_constant(
		const flt::value_repr & arg) const = 0;
};

/* Represents a binary operation on a float. */
class binary_op : public jive::binary_op {
public:
	virtual
	~binary_op() noexcept;

	inline
	binary_op()
	: jive::binary_op({type(), type()}, type())
	{}

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
class compare_op : public jive::binary_op {
public:
	virtual
	~compare_op() noexcept;

	inline
	compare_op()
	: jive::binary_op({type(), type()}, bit1)
	{}

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

	static jive::output *
	normalized_create(jive::output * arg)
	{
		return simple_node::create_normalized(arg->region(), make_unop(), {arg})[0];
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
	const char * name,
	enum jive::binary_op::flags op_flags>
class make_binop final : public binary_op {
public:
	virtual ~make_binop() noexcept {}

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const make_binop * o = dynamic_cast<const make_binop *>(&other);
		return o != nullptr;
	}

	static jive::output *
	normalized_create(jive::output * arg1, jive::output * arg2)
	{
		return simple_node::create_normalized(arg1->region(), make_binop(), {arg1, arg2})[0];
	}

	virtual enum jive::binary_op::flags
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
	const char * name,
	enum jive::binary_op::flags op_flags>
class make_cmpop final : public compare_op {
public:
	virtual ~make_cmpop() noexcept {}

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const make_cmpop * o = dynamic_cast<const make_cmpop *>(&other);
		return o != nullptr;
	}

	static jive::output *
	normalized_create(jive::output * arg1, jive::output * arg2)
	{
		make_cmpop op;
		return simple_node::create_normalized(arg1->region(), make_cmpop(), {arg1, arg2})[0];
	}

	virtual enum jive::binary_op::flags
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
