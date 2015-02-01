/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdtype.h>
#include <jive/util/buffer.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

namespace jive {
namespace rcd {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "rcd";
}

bool
type::operator==(const jive::base::type & _other) const noexcept
{
	const jive::rcd::type * other = dynamic_cast<const jive::rcd::type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

jive::rcd::type *
type::copy() const
{
	return new jive::rcd::type(this->declaration());
}

}
}
