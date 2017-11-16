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
namespace addr {

/* address type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "addr";
}

bool
type::operator==(const jive::type & other) const noexcept
{
	return dynamic_cast<const jive::addr::type*>(&other) != nullptr;
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

const type type::instance_;

}

/* memory type */

namespace mem {

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "mem";
}

bool
type::operator==(const jive::type & other) const noexcept
{
	return dynamic_cast<const jive::mem::type*>(&other) != nullptr;
}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

const type type::instance_;

}

}
