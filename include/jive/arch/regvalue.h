/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGVALUE_H
#define JIVE_ARCH_REGVALUE_H

#include <stdint.h>

#include <jive/arch/registers.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/simple.h>

namespace jive {

class regvalue_op final : public simple_op {
public:
	virtual
	~regvalue_op() noexcept;

	inline explicit constexpr
	regvalue_op(const jive_register_class * regcls) noexcept
		: regcls_(regcls)
	{
	}

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

	inline const jive_register_class * regcls() const { return regcls_; }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive_register_class * regcls_;
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
jive_regvalue(jive::output * value, const jive_register_class * regcls);

#endif
