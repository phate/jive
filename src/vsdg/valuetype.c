/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/util/list.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/valuetype.h>

namespace jive {
namespace value {

type::~type() noexcept {}

/* value gates */

gate::~gate() noexcept {}

gate::gate(jive_graph * graph, const char name[], const jive::base::type & type)
	: jive::gate(graph, name, type)
{}

}
}
