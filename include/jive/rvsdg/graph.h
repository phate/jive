/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_GRAPH_H
#define JIVE_RVSDG_GRAPH_H

#include <stdbool.h>
#include <stdlib.h>

#include <typeindex>

#include <jive/common.h>
#include <jive/rvsdg/gate.h>
#include <jive/rvsdg/node-normal-form.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/tracker.h>

/* graph */

namespace jive {

typedef jive::detail::intrusive_list<
	jive::gate,
	jive::gate::graph_gate_accessor
> graph_gate_list;

class graph {
public:
	~graph();

	graph();

	inline jive::region *
	root() const noexcept
	{
		return root_;
	}

	inline void
	mark_denormalized() noexcept
	{
		normalized_ = false;
	}

	inline void
	normalize()
	{
		root()->normalize(true);
		normalized_ = true;
	}

	std::unique_ptr<jive::graph>
	copy() const;

	jive::node_normal_form *
	node_normal_form(const std::type_info & type) noexcept;

	inline jive::argument *
	add_import(const jive::type & type, const std::string & name)
	{
		auto gate = gate::create(this, name, type);
		return root()->add_argument(nullptr, gate);
	}

	inline jive::input *
	add_export(jive::output * operand, const std::string & name)
	{
		auto gate = gate::create(this, name, operand->type());
		return root()->add_result(operand, nullptr, gate);
	}

	inline void
	prune()
	{
		root()->prune(true);
	}

	graph_gate_list gates;

private:
	bool normalized_;
	jive::region * root_;
	jive::node_normal_form_hash node_normal_forms_;
};

}

#endif
