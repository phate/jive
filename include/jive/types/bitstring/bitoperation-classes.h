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

/* Represents a unary operation on a bitstring of a specific width,
 * produces another bitstring of the same width. */
class bitunary_op : public jive::unary_op {
public:
	virtual
	~bitunary_op() noexcept;

	inline
	bitunary_op(const bittype & type) noexcept
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

	inline const bittype &
	type() const noexcept
	{
		return *static_cast<const bittype*>(&port_.type());
	}

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	virtual bitvalue_repr
	reduce_constant(
		const bitvalue_repr & arg) const = 0;

	virtual std::unique_ptr<bitunary_op>
	create(size_t nbits) const = 0;

private:
	jive::port port_;
};

/* Represents a binary operation (possibly normalized n-ary if associative)
 * on a bitstring of a specific width, produces another bitstring of the
 * same width. */
class bitbinary_op : public jive::binary_op {
public:
	virtual
	~bitbinary_op() noexcept;

	inline
	bitbinary_op(const bittype & type, size_t arity = 2) noexcept
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

	virtual bitvalue_repr
	reduce_constants(
		const bitvalue_repr & arg1,
		const bitvalue_repr & arg2) const = 0;

	virtual std::unique_ptr<bitbinary_op>
	create(size_t nbits) const = 0;

	inline const bittype &
	type() const noexcept
	{
		return *static_cast<const bittype*>(&port_.type());
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

class bitcompare_op : public jive::binary_op {
public:
	virtual
	~bitcompare_op() noexcept;

	inline
	bitcompare_op(const bittype & type) noexcept
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
		const bitvalue_repr & arg1,
		const bitvalue_repr & arg2) const = 0;

	virtual std::unique_ptr<bitcompare_op>
	create(size_t nbits) const = 0;

	inline const bittype &
	type() const noexcept
	{
		return *static_cast<const bittype*>(&port_.type());
	}

private:
	size_t arity_;
	jive::port port_;
};

}

#endif
