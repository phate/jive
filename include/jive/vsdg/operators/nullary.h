/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_NULLARY_H
#define JIVE_VSDG_OPERATORS_NULLARY_H

#include <jive/common.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/base.h>

namespace jive {

class output;

namespace base {

/**
	\brief Nullary operator (operator taking no formal arguments)
*/
class nullary_op : public operation {
public:
	virtual
	~nullary_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual const type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;
};

template<typename Type, typename ValueRepr>
struct default_type_of_value {
	Type operator()(const ValueRepr &) const noexcept
	{
		return Type();
	}
};

/* Template to represent a domain-specific constant. Instances are fully
 * characterised by the value they contain.
 *
 * Template argument requirements:
 * - Type: type class of the constants represented
 * - ValueRepr: representation of values
 * - FormatValue: functional that takes a ValueRepr instance and returns
 *   as std::string a human-readable representation of the value
 * - TypeOfValue: functional that takes a ValueRepr instance and returns
 *   the Type instances corresponding to this value (in case the type
 *   class is polymorphic) */
template<
	typename Type,
	typename ValueRepr,
	typename FormatValue,
	typename TypeOfValue
>
class domain_const_op final : public nullary_op {
public:
	typedef ValueRepr value_repr;

	virtual
	~domain_const_op() noexcept
	{
	}

	inline
	domain_const_op(const value_repr & value)
		: value_(value)
		, type_(TypeOfValue()(value_))
	{
	}

	inline
	domain_const_op(value_repr && value) noexcept
		: value_(std::move(value))
		, type_(TypeOfValue()(value_))
	{
	}

	inline
	domain_const_op(const domain_const_op & other) = default;

	inline
	domain_const_op(domain_const_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const domain_const_op * op =
			dynamic_cast<const domain_const_op *>(&other);
		return op && op->type_ == type_ && op->value_ == value_;
	}

	virtual std::string
	debug_string() const override
	{
		return FormatValue()(value_);
	}

	virtual const type &
	result_type(size_t index) const noexcept override
	{
		return type_;
	}

	inline const value_repr &
	value() const noexcept
	{
		return value_;
	}

	virtual std::unique_ptr<jive::operation> copy() const override
	{
		return std::unique_ptr<jive::operation>(new domain_const_op(*this));
	}

private:
	value_repr value_;
	Type type_;
};

}
}

#endif
