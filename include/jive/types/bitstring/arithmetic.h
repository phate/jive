/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_H

#include <jive/types/bitstring/bitoperation-classes.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace bits {

class and_op final : public binary_op {
public:
	virtual ~and_op() noexcept;

	inline and_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: binary_op(type, arity)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class ashr_op final : public binary_op {
public:
	virtual ~ashr_op() noexcept;

	inline ashr_op(const jive::bits::type & type) noexcept : binary_op(type) {}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class sub_op final : public binary_op {
public:
	virtual ~sub_op() noexcept;

	inline sub_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class neg_op final : public unary_op {
public:
	virtual
	~neg_op() noexcept;

	inline
	neg_op(const jive::bits::type & type) noexcept
		: unary_op(type)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual value_repr
	reduce_constant(
		const value_repr & arg) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class not_op final : public unary_op {
public:
	virtual
	~not_op() noexcept;

	inline not_op(const jive::bits::type & type) noexcept : unary_op(type) {}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual value_repr
	reduce_constant(
		const value_repr & arg) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class or_op final : public binary_op {
public:
	virtual ~or_op() noexcept;

	inline or_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: binary_op(type, arity)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class mul_op final : public binary_op {
public:
	virtual ~mul_op() noexcept;

	inline mul_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: binary_op(type, arity)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class smulh_op final : public binary_op {
public:
	virtual ~smulh_op() noexcept;

	inline smulh_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class shl_op final : public binary_op {
public:
	virtual ~shl_op() noexcept;

	inline shl_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class shr_op final : public binary_op {
public:
	virtual ~shr_op() noexcept;

	inline shr_op(const jive::bits::type & type) noexcept : binary_op(type) {}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class smod_op final : public binary_op {
public:
	virtual ~smod_op() noexcept;

	inline smod_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class sdiv_op final : public binary_op {
public:
	virtual ~sdiv_op() noexcept;

	inline sdiv_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class add_op final : public binary_op {
public:
	virtual ~add_op() noexcept;

	inline add_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: binary_op(type, arity)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class umulh_op final : public binary_op {
public:
	virtual ~umulh_op() noexcept;

	inline umulh_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class umod_op final : public binary_op {
public:
	virtual ~umod_op() noexcept;

	inline umod_op(const jive::bits::type & type) noexcept
		: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class udiv_op final : public binary_op {
public:
	virtual ~udiv_op() noexcept;

	inline udiv_op(const jive::bits::type & type) noexcept
	: binary_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class xor_op final : public binary_op {
public:
	virtual ~xor_op() noexcept;

	inline xor_op(const jive::bits::type & type, size_t arity = 2) noexcept
		: binary_op(type, arity)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual value_repr
	reduce_constants(
		const value_repr & arg1,
		const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

static inline jive::output *
create_and(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), and_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_ashr(size_t nbits, jive::output * op, jive::output * shift)
{
	return create_normalized(op->region(), ashr_op(nbits), {op, shift})[0];
}

static inline jive::output *
create_sub(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), sub_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_neg(size_t nbits, jive::output * op)
{
	return create_normalized(op->region(), neg_op(nbits), {op})[0];
}

static inline jive::output *
create_not(size_t nbits, jive::output * op)
{
	return create_normalized(op->region(), not_op(nbits), {op})[0];
}

static inline jive::output *
create_or(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), or_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_mul(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), mul_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_smulh(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), smulh_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_shl(size_t nbits, jive::output * op, jive::output * shift)
{
	return create_normalized(op->region(), shl_op(nbits), {op, shift})[0];
}

static inline jive::output *
create_shr(size_t nbits, jive::output * op, jive::output * shift)
{
	return create_normalized(op->region(), shr_op(nbits), {op, shift})[0];
}

static inline jive::output *
create_smod(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), smod_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_sdiv(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), sdiv_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_add(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), add_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_umulh(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), umulh_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_umod(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), umod_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_udiv(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), udiv_op(nbits), {op1, op2})[0];
}

static inline jive::output *
create_xor(size_t nbits, jive::output * op1, jive::output * op2)
{
	return create_normalized(op1->region(), xor_op(nbits), {op1, op2})[0];
}

}
}

#endif
