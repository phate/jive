/*
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_TYPE_H
#define JIVE_TYPES_BITSTRING_TYPE_H

#include <jive/vsdg/basetype.h>

namespace jive {
namespace bits {

/* bitstring type */

class type final : public jive::value::type {
public:
	virtual ~type() noexcept;

	inline constexpr type(size_t nbits) noexcept
		: nbits_(nbits)
	{
	}

	inline size_t nbits() const noexcept { return nbits_; }

	virtual std::string debug_string() const override;

	virtual bool operator==(const jive::base::type & other) const noexcept override;

	virtual std::unique_ptr<base::type>
	copy() const override;

private:
	size_t nbits_;
};

}
}

#endif
