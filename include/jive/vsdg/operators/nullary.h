/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_NULLARY_H
#define JIVE_VSDG_OPERATORS_NULLARY_H

#include <jive/common.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node-private.h>
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
 * - Class: the node class
 * - Type: type class of the constants represented
 * - ValueRepr: representation of values
 * - FormatValue: functional that takes a ValueRepr instance and returns
 *   as std::string a human-readable representation of the value
 * - TypeOfValue: functional that takes a ValueRepr instance and returns
 *   the Type instances corresponding to this value (in case the type
 *   class is polymorphic) */
template<
	const jive_node_class * Class,
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

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override
	{
		operation_node<domain_const_op> * node =
			jive::create_operation_node(*this);
		const base::type * result_types[1] = { &result_type(0) };
		jive_node_init_(node, region,
			0, nullptr, nullptr,
			1, result_types);
		return node;
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

/* Template to allow definition of domain-specific "symbolic constants":
 * These are operators yielding a specific but unknown value of the
 * respective type. Useful mainly for testing purposes as it represents
 * sub-expressions which cannot be transformed any further. */
template<const jive_node_class * Class, typename Type>
class domain_symbol_op final : public nullary_op {
public:
	virtual
	~domain_symbol_op() noexcept
	{
	}

	inline
	domain_symbol_op(
		std::string && name,
		Type && type) noexcept
		: name_(std::move(name))
		, type_(std::move(type))
	{
	}

	inline
	domain_symbol_op(const domain_symbol_op & other) = default;

	inline
	domain_symbol_op(domain_symbol_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const domain_symbol_op * op =
			dynamic_cast<const domain_symbol_op *>(&other);
		return op && op->type_ == type_ && op->name_ == name_;
	}

	virtual std::string
	debug_string() const override
	{
		return name_;
	}

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override
	{
		operation_node<domain_symbol_op> * node =
			jive::create_operation_node(*this);
		const base::type * result_types[1] = { &result_type(0) };
		jive_node_init_(node, region,
			0, nullptr, nullptr,
			1, result_types);
		return node;
	}

	virtual const type &
	result_type(size_t index) const noexcept override
	{
		return type_;
	}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	virtual std::unique_ptr<jive::operation> copy() const override
	{
		return std::unique_ptr<jive::operation>(new domain_symbol_op(*this));
	}

private:
	std::string name_;
	Type type_;
};

}
}

extern const jive_node_class JIVE_NULLARY_OPERATION;

JIVE_EXPORTED_INLINE jive::output *
jive_nullary_operation_create_normalized(const jive_node_class * cls,
	struct jive_graph * graph, const jive_node_attrs * attrs)
{
	jive::output * result;
	jive_node_create_normalized(cls, graph, attrs, 0, NULL, &result);
	return result;
}

/* node class inheritable methods */

jive::node_normal_form *
jive_nullary_operation_get_default_normal_form_(
	const jive_node_class * cls,
	jive::node_normal_form * parent,
	struct jive_graph * graph);


#endif
