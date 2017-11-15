/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/controltype.h>

#include <inttypes.h>
#include <string.h>

#include <jive/util/list.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace ctl {

/* type */

type::~type() noexcept {}

type::type(size_t nalternatives)
	: jive::statetype()
	, nalternatives_(nalternatives)
{
	if (nalternatives == 0)
		throw compiler_error("Alternatives of a control type must be non-zero.");
}

std::string
type::debug_string() const
{
	return detail::strfmt("ctl(", nalternatives_, ")");
}

bool
type::operator==(const jive::type & other) const noexcept
{
	const jive::ctl::type * type = dynamic_cast<const jive::ctl::type*>(&other);
	return type && type->nalternatives_ == nalternatives_;
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

/* value_repr */

value_repr::value_repr(size_t alternative, size_t nalternatives)
	: alternative_(alternative)
	, nalternatives_(nalternatives)
{
	if (alternative >= nalternatives)
		throw compiler_error("Alternative is bigger than the number of possible alternatives.");
}

}
}
