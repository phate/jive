/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CLG_NODE_H
#define JIVE_FRONTEND_CLG_NODE_H

#include <string>
#include <unordered_set>

namespace jive {
namespace frontend {

class cfg;
class clg;

class clg_node final {
public:
	~clg_node();

	clg_node(jive::frontend::clg & clg, const char * name);

	jive::frontend::clg * clg;

	std::unordered_set<jive::frontend::clg_node*> calls;

	struct {
		jive::frontend::clg_node * prev;
		jive::frontend::clg_node * next;
	} clg_node_list;

	std::string name;
	jive::frontend::cfg * cfg;
};

}
}

void
jive_clg_node_add_call(jive::frontend::clg_node & self, jive::frontend::clg_node & callee);

#endif
