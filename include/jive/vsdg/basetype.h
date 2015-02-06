/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_BASETYPE_H
#define JIVE_VSDG_BASETYPE_H

#include <string>

namespace jive {
namespace base {

class type {
public:
	virtual
	~type() noexcept;

protected:
	inline constexpr
	type() noexcept
	{}

public:
	virtual bool
	operator==(const jive::base::type & other) const noexcept = 0;

	inline bool
	operator!=(const jive::base::type & other) const noexcept
	{
		return !(*this == other);
	}

	virtual jive::base::type *
	copy() const = 0;

	virtual std::string
	debug_string() const = 0;
};

}
}

#endif
