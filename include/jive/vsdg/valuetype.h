/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_VALUETYPE_H
#define JIVE_VSDG_VALUETYPE_H

#include <jive/vsdg/basetype.h>

namespace jive {
namespace value {

class type : public jive::base::type {
public:
	virtual ~type() noexcept;

	virtual jive::value::type * copy() const override = 0;

protected:
	inline constexpr type() noexcept : jive::base::type() {};
};

}
}

#endif
