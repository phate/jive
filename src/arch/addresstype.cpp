/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.hpp>

#include <string.h>

#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/node.hpp>

namespace jive {

/* address type */

addrtype::~addrtype() noexcept
{}

std::string
addrtype::debug_string() const
{
	return "*" + type().debug_string();
}

bool
addrtype::operator==(const jive::type & other) const noexcept
{
	auto at = dynamic_cast<const addrtype*>(&other);
	return at && at->type() == type();
}

std::unique_ptr<jive::type>
addrtype::copy() const
{
	return std::unique_ptr<jive::type>(new addrtype(*this));
}

/* memory type */

memtype::~memtype() noexcept
{}

std::string
memtype::debug_string() const
{
	return "mem";
}

bool
memtype::operator==(const jive::type & other) const noexcept
{
	return dynamic_cast<const jive::memtype*>(&other) != nullptr;
}

std::unique_ptr<jive::type>
memtype::copy() const
{
	return std::unique_ptr<jive::type>(new memtype(*this));
}

const memtype memtype::instance_;

}
