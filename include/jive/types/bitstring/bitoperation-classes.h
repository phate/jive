/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H

#include <jive/rvsdg/binary.h>
#include <jive/rvsdg/unary.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/bitstring/value-representation.h>

namespace jive {
namespace bits {

/* Represents a unary operation on a bitstring of a specific width,
 * produces another bitstring of the same width. */
class unary_op : public jive::unary_op {
public:
	virtual ~unary_op() noexcept;

	inline
	unary_op(const jive::bits::type & type) noexcept
	: port_(type)
	{}

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	inline const jive::bits::type &
	type() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&port_.type());
	}

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	virtual value_repr
	reduce_constant(
		const value_repr & arg) const = 0;

private:
	jive::port port_;
};

/* Represents a binary operation (possibly normalized n-ary if associative)
 * on a bitstring of a specific width, produces another bitstring of the
 * same width. */
class binary_op : public jive::binary_op {
public:
	virtual ~binary_op() noexcept;

	inline
	binary_op(const jive::bits::type & type, size_t arity = 2) noexcept
	: arity_(arity)
	, port_(type)
	{}

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

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

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const = 0;


	inline const jive::bits::type &
	type() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&port_.type());
	}

	inline size_t
	arity() const noexcept
	{
		return arity_;
	}

private:
	size_t arity_;
	jive::port port_;
};

enum class compare_result {
	undecidable,
	static_true,
	static_false
};

class compare_op : public jive::binary_op {
public:
	inline
	compare_op(const jive::bits::type & type) noexcept
	: port_(type)
	{}

	virtual ~compare_op() noexcept;

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

	virtual compare_result
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const = 0;

	inline const jive::bits::type &
	type() const noexcept
	{
		return *static_cast<const jive::bits::type*>(&port_.type());
	}

private:
	size_t arity_;
	jive::port port_;
};

}
}

#endif
