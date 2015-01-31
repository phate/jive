/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/controltype.h>

#include <inttypes.h>
#include <string.h>

#include <jive/util/buffer.h>
#include <jive/util/list.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace ctl {

/* type */

type::~type() noexcept {}

type::type(size_t nalternatives)
	: jive::state::type()
	, nalternatives_(nalternatives)
{
	if (nalternatives == 0)
		throw compiler_error("Alternatives of a control type must be non-zero.");
}

std::string
type::debug_string() const
{
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "ctl(%zu)", nalternatives_);
	return tmp;
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	const jive::ctl::type * type = dynamic_cast<const jive::ctl::type*>(&other);
	return type && type->nalternatives_ == nalternatives_;
}

jive::ctl::type *
type::copy() const
{
	return new jive::ctl::type(nalternatives_);
}

jive::output *
type::create_output(jive_node * node, size_t index) const
{
	return new jive::ctl::output(nalternatives_, node, index);
}

jive::gate *
type::create_gate(jive_graph * graph, const char * name) const
{
	return new jive::ctl::gate(nalternatives_, graph, name);
}

/* output */

output::~output() noexcept {}

output::output(size_t nalternatives, struct jive_node * node, size_t index)
	: jive::state::output(node, index)
	, type_(nalternatives)
{}

/* gate */

gate::~gate() noexcept {}

gate::gate(size_t nalternatives, jive_graph * graph, const char name[])
	: jive::state::gate(graph, name)
	, type_(nalternatives)
{}

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
