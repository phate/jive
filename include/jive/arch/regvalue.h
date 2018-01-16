/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGVALUE_H
#define JIVE_ARCH_REGVALUE_H

#include <stdint.h>

#include <jive/arch/registers.h>
#include <jive/rvsdg/node.h>

namespace jive {

class regvalue_op final : public simple_op {
public:
	virtual
	~regvalue_op() noexcept;

	inline
	regvalue_op(const register_class * regcls) noexcept
	: simple_op({regcls}, {regcls})
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const register_class *
	regcls() const
	{
		return static_cast<const register_class*>(argument(0).rescls());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}

/**
	\brief Create register constant
	\param value Value to be represented
	\param regcls Register class
	\returns Bitstring value representing constant, constrained to register class
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive::output *
jive_regvalue(jive::output * value, const jive::register_class * regcls);

#endif
