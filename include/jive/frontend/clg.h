/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CLG_H
#define JIVE_FRONTEND_CLG_H

namespace jive {
	class buffer;

namespace frontend {

class clg_node;

class clg final {
public:
	~clg();

	clg() noexcept;

	struct {
		jive::frontend::clg_node * first;
		jive::frontend::clg_node * last;
	} nodes;
};

}
}

void
jive_clg_convert_dot(const jive::frontend::clg & self, jive::buffer & buffer);

void
jive_clg_view(const jive::frontend::clg & self);

#endif
