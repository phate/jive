/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/type.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <stdio.h>
#include <string.h>

namespace jive {
namespace bits {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "bits%zd", nbits());
	return std::string(tmp);
}

bool
type::operator==(const jive::base::type & _other) const noexcept
{
	const jive::bits::type * other = dynamic_cast<const jive::bits::type*>(&_other);
	return other != nullptr && this->nbits() == other->nbits();
}

std::unique_ptr<base::type>
type::copy() const
{
	return std::unique_ptr<base::type>(new type(*this));
}

}
}
