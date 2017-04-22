/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_LOAD_H
#define JIVE_ARCH_LOAD_H

#include <algorithm>
#include <memory>

#include <jive/util/ptr-collection.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/statetype.h>

namespace jive {

class load_op final : public simple_op {
public:
	virtual
	~load_op() noexcept;

	template<typename Types>
	inline
	load_op(
		const jive::value::type & address_type,
		const Types & state_types,
		const jive::value::type & data_type)
		: address_type_(address_type.copy())
		, state_types_(state_types.size())
		, data_type_(data_type.copy())
	{
		std::transform(state_types.begin(), state_types.end(), state_types_.begin(),
			[](const auto & t){ return t->copy(); });
	}

	inline
	load_op(const load_op & other)
		: address_type_(other.address_type_->copy())
		, state_types_(other.state_types_.size())
		, data_type_(other.data_type_->copy())
	{
		std::transform(other.state_types_.begin(), other.state_types_.end(), state_types_.begin(),
			[](const auto & t){ return t->copy(); });
	}

	inline
	load_op(load_op && other) noexcept = default;

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
	virtual std::string
	debug_string() const override;

	inline const jive::value::type &
	address_type() const noexcept
	{
		return *static_cast<const value::type*>(address_type_.get());
	}

	inline const jive::value::type &
	data_type() const noexcept
	{
		return *static_cast<const value::type*>(data_type_.get());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::unique_ptr<base::type> address_type_;
	std::vector<std::unique_ptr<base::type>> state_types_;
	std::unique_ptr<base::type> data_type_;
};

}

jive::oport *
jive_load_by_address_create(jive::oport * address,
	const jive::value::type * datatype,
	size_t nstates, jive::oport * const states[]);

jive::oport *
jive_load_by_bitstring_create(jive::oport * address,
	size_t nbits, const jive::value::type * datatype,
	size_t nstates, jive::oport * const states[]);

#endif
