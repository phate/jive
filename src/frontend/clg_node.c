/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/common.h>
#include <jive/frontend/cfg.h>
#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/list.h>

namespace jive {
namespace frontend {

clg_node::~clg_node()
{
	JIVE_LIST_REMOVE(clg->nodes, this, clg_node_list);
	delete cfg;
}

clg_node::clg_node(jive::frontend::clg & _clg, const char * _name)
	: clg(&_clg)
	, name(_name)
{
	JIVE_LIST_PUSH_BACK(clg->nodes, this, clg_node_list);
	cfg = new jive::frontend::cfg;
}

}
}

void
jive_clg_node_add_call(jive::frontend::clg_node & self, jive::frontend::clg_node & callee)
{
	self.calls.insert(&callee);
}
