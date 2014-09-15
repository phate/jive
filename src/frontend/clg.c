/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/util/buffer.h>
#include <jive/util/list.h>

#include <stdio.h>

namespace jive {
namespace frontend {

clg::~clg()
{
	while (nodes.first)
		delete nodes.first;
}

clg::clg() noexcept
{
	nodes.first = 0;
	nodes.last = 0;
}

}
}

void
jive_clg_convert_dot(const jive::frontend::clg & self, jive::buffer & buffer)
{
	buffer.append("digraph clg {\n");

	char tmp[96];
	jive::frontend::clg_node * node;
	JIVE_LIST_ITERATE(self.nodes, node, clg_node_list) {
		snprintf(tmp, sizeof(tmp), "%zu", (size_t)node);
		buffer.append(tmp).append("[label = \"");
		buffer.append(node->name.c_str());
		buffer.append("\"];\n");

		for (auto c : node->calls) {
			snprintf(tmp, sizeof(tmp), "%zu -> %zu;\n", (size_t)node, (size_t)c);
			buffer.append(tmp);
		}
	}

	buffer.append("}\n");
}

void
jive_clg_view(const jive::frontend::clg & self)
{
	jive::buffer buffer;

	FILE * file = popen("tee /tmp/clg.dot | dot -Tps > /tmp/clg.ps ; gv /tmp/clg.ps", "w");
	jive_clg_convert_dot(self, buffer);
	fwrite(buffer.c_str(), buffer.size(), 1, file);
	pclose(file);
}
