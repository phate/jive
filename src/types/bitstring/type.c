/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/type.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/node.h>

namespace jive {

/* bistring type */

bittype::~bittype() noexcept
{}

std::string
bittype::debug_string() const
{
	return detail::strfmt("bit", nbits());
}

bool
bittype::operator==(const jive::type & other) const noexcept
{
	auto type = dynamic_cast<const bittype*>(&other);
	return type != nullptr && this->nbits() == type->nbits();
}

std::unique_ptr<jive::type>
bittype::copy() const
{
	return std::unique_ptr<jive::type>(new bittype(*this));
}

}
