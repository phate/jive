/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/addresstype.h>

#include <string.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/node.h>

namespace jive {

/* address type */

addrtype::~addrtype() noexcept
{}

std::string
addrtype::debug_string() const
{
	return "addr";
}

bool
addrtype::operator==(const jive::type & other) const noexcept
{
	return dynamic_cast<const jive::addrtype*>(&other) != nullptr;
}

std::unique_ptr<jive::type>
addrtype::copy() const
{
	return std::unique_ptr<jive::type>(new addrtype(*this));
}

const addrtype addrtype::instance_;

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
