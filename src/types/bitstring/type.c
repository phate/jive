/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/type.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/node.h>

namespace jive {
namespace bits {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return detail::strfmt("bits", nbits());
}

bool
type::operator==(const jive::type & _other) const noexcept
{
	const jive::bits::type * other = dynamic_cast<const jive::bits::type*>(&_other);
	return other != nullptr && this->nbits() == other->nbits();
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

}
}
