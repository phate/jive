/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-type.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace imm {

/* gate */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[])
	: jive::value::gate(graph, name, jive::imm::type())
{}

/* type */

type::~type() noexcept {}

std::string
type::debug_string() const
{
	return "imm";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::imm::type*>(&other) != nullptr;
}

jive::imm::type *
type::copy() const
{
	return new jive::imm::type();
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::imm::gate(graph, name);
}

}
}
