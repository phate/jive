/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_H

#include <jive/types/bitstring/bitoperation-classes.h>

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

}
}

jive::oport *
jive_bitand(size_t nbits, const std::vector<jive::oport*> & operands);

jive::oport *
jive_bitashr(size_t nbits, jive::oport * operand, jive::oport * shift);

jive::oport *
jive_bitsub(size_t nbits, jive::oport * op1, jive::oport * op2);

jive::oport *
jive_bitneg(size_t nbits, jive::oport * operand);

jive::oport *
jive_bitnot(size_t nbits, jive::oport * operand);

jive::oport *
jive_bitor(size_t nbits, const std::vector<jive::oport*> & operands);

jive::oport *
jive_bitmul(size_t nbits, const std::vector<jive::oport*> & operands);

jive::oport *
jive_bitsmulh(size_t nbits, jive::oport * factor1, jive::oport * factor2);

jive::oport *
jive_bitsmod(size_t nbits, jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitshl(size_t nbits, jive::oport * operand, jive::oport * shift);

jive::oport *
jive_bitshr(size_t nbits, jive::oport * operand, jive::oport * shift);

jive::oport *
jive_bitsdiv(size_t nbits, jive::oport * dividend, jive::oport * divisor);

jive::oport *
jive_bitsum(size_t nbits, const std::vector<jive::oport*> & operands);

jive::oport *
jive_bitumulh(size_t nbits, jive::oport * factor1, jive::oport * factor2);

jive::oport *
jive_bitumod(size_t nbits, jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitudiv(size_t nbits, jive::oport * dividend, jive::oport * divisor);

jive::oport *
jive_bitxor(size_t nbits, const std::vector<jive::oport*> & operands);

#endif
