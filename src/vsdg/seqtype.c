/*
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/seqtype.h>

namespace jive {
namespace seq {

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "seq";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::seq::type*>(&other) != nullptr;
}

jive::seq::type *
type::copy() const
{
	return new jive::seq::type();
}

}
}
