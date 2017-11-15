/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_H
#define JIVE_TYPES_FLOAT_FLTOPERATION_CLASSES_H

#include <jive/types/float/value-representation.h>
#include <jive/vsdg/binary.h>
#include <jive/vsdg/simple-node.h>
#include <jive/vsdg/unary.h>

namespace jive {
namespace flt {

/* Represents a unary operation on a float. */
class unary_op : public jive::unary_op {
public:
	virtual ~unary_op() noexcept;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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
	virtual ~binary_op() noexcept;

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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
	virtual ~compare_op() noexcept;

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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
		return jive::create_normalized(arg->region(), make_unop(), {arg})[0];
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

	static jive::output *
	normalized_create(jive::output * arg1, jive::output * arg2)
	{
		return jive::create_normalized(arg1->region(), make_binop(), {arg1, arg2})[0];
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

	static jive::output *
	normalized_create(jive::output * arg1, jive::output * arg2)
	{
		make_cmpop op;
		return jive::create_normalized(arg1->region(), make_cmpop(), {arg1, arg2})[0];
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
