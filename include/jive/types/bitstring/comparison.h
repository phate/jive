/*
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_H
#define JIVE_TYPES_BITSTRING_COMPARISON_H

#include <jive/types/bitstring/bitoperation-classes.h>

namespace jive {
namespace bits {

class eq_op final : public compare_op {
public:
	virtual
	~eq_op() noexcept;

	inline
	eq_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class ne_op final : public compare_op {
public:
	virtual
	~ne_op() noexcept;

	inline
	ne_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class sgt_op final : public compare_op {
public:
	virtual
	~sgt_op() noexcept;

	inline
	sgt_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class sge_op final : public compare_op {
public:
	virtual
	~sge_op() noexcept;

	inline
	sge_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class slt_op final : public compare_op {
public:
	virtual
	~slt_op() noexcept;

	inline
	slt_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class sle_op final : public compare_op {
public:
	virtual
	~sle_op() noexcept;

	inline
	sle_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class ugt_op final : public compare_op {
public:
	virtual
	~ugt_op() noexcept;

	inline
	ugt_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class uge_op final : public compare_op {
public:
	virtual
	~uge_op() noexcept;

	inline
	uge_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class ult_op final : public compare_op {
public:
	virtual
	~ult_op() noexcept;

	inline
	ult_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class ule_op final : public compare_op {
public:
	virtual
	~ule_op() noexcept;

	inline
	ule_op(const jive::bits::type & type) noexcept
	: compare_op(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_binary_operation_flags
	flags() const noexcept override;

	virtual compare_result
	reduce_constants(const value_repr & arg1, const value_repr & arg2) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}
}

jive::oport *
jive_bitequal(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitnotequal(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitsgreater(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitsgreatereq(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitsless(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitslesseq(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitugreater(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitugreatereq(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bituless(jive::oport * operand1, jive::oport * operand2);

jive::oport *
jive_bitulesseq(jive::oport * operand1, jive::oport * operand2);

#endif
