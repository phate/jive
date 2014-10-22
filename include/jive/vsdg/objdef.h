/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OBJDEF_H
#define JIVE_VSDG_OBJDEF_H

#include <string>

#include <jive/vsdg/node.h>

struct jive_label;
struct jive_linker_symbol;

namespace jive {

class output;

class objdef_operation final : public operation {
public:
	virtual
	~objdef_operation() noexcept;

	inline
	objdef_operation(
		const std::string & name,
		const jive_linker_symbol * symbol,
		const base::type & type)
		: name_(name)
		, symbol_(symbol)
		, type_(type.copy())
	{
	}

	inline
	objdef_operation(
		const objdef_operation & other)
		: name_(other.name_)
		, symbol_(other.symbol_)
		, type_(other.type_->copy())
	{
	}

	inline
	objdef_operation(objdef_operation && other) noexcept = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	const std::string & name() const noexcept { return name_; }
	const jive_linker_symbol * symbol() const noexcept { return symbol_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::string name_;
	const jive_linker_symbol * symbol_;
	std::unique_ptr<base::type> type_;
};

}

jive::output *
jive_objdef_create(
	jive::output * output,
	const char * name,
	const jive_linker_symbol * symbol);

#endif
