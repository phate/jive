/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H
#define JIVE_TYPES_BITSTRING_BITOPERATION_CLASSES_H

#include <jive/types/bitstring/type.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/vsdg/operators.h>

namespace jive {

/* Represents a unary operation on a bitstring of a specific width,
 * produces another bitstring of the same width. */
class bits_unary_operation : public base::unary_op {
public:
	virtual ~bits_unary_operation() noexcept;

	inline bits_unary_operation(const jive::bits::type & type) noexcept : type_(type) {}

	/* type signature methods */
	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	inline const jive::bits::type & type() const noexcept { return type_; }

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	virtual bitstring::value_repr
	reduce_constant(
		const bitstring::value_repr & arg) const = 0;

private:
	jive::bits::type type_;
};

/* Represents a binary operation (possibly normalized n-ary if associative)
 * on a bitstring of a specific width, produces another bitstring of the
 * same width. */
class bits_binary_operation : public binary_operation {
public:
	virtual ~bits_binary_operation() noexcept;

	inline bits_binary_operation(const jive::bits::type & type, size_t arity = 2) noexcept
		: type_(type)
		, arity_(arity)
	{}

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

	virtual bitstring::value_repr
	reduce_constants(
		const bitstring::value_repr & arg1,
		const bitstring::value_repr & arg2) const = 0;


	inline const jive::bits::type & type() const noexcept { return type_; }

	inline size_t
	arity() const noexcept { return arity_; }

private:
	jive::bits::type type_;
	size_t arity_;
};

enum class compare_result {
	undecidable,
	static_true,
	static_false
};

class bits_compare_operation : public binary_operation {
public:
	inline
	bits_compare_operation(
		const jive::bits::type & type) noexcept
		: type_(type)
	{
	}

	virtual ~bits_compare_operation() noexcept;

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

	virtual compare_result
	reduce_constants(
		const bitstring::value_repr & arg1,
		const bitstring::value_repr & arg2) const = 0;

	inline const jive::bits::type &
	type() const noexcept { return type_; }

private:
	jive::bits::type type_;
	size_t arity_;
};

}

extern const jive_node_class JIVE_BITBINARY_NODE;

extern const jive_node_class JIVE_BITUNARY_NODE;

extern const jive_node_class JIVE_BITCOMPARISON_NODE;

#endif
