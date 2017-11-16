/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/node.h>
#include <jive/types/union/unntype.h>

#include <string.h>

namespace jive {
namespace unn {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "unn";
}

bool
type::operator==(const jive::type & _other) const noexcept
{
	const jive::unn::type * other = dynamic_cast<const jive::unn::type*>(&_other);
	return other != nullptr && this->declaration() == other->declaration();
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

}
}
