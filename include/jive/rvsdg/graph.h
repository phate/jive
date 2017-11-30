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
#include <jive/util/callbacks.h>
#include <jive/rvsdg/gate.h>
#include <jive/rvsdg/node-normal-form.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/tracker.h>

/* graph */

struct jive_resource;
struct jive_resource_name;

namespace jive {

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

	inline jive::gate *
	create_gate(const std::string & name, const jive::type & type)
	{
		return gate::create(this, name, type);
	}

	inline jive::gate *
	create_gate(const std::string & name, const jive::resource_class * rescls)
	{
		return gate::create(this, name, rescls);
	}

	inline jive::gate *
	create_gate(const jive::gate * gate)
	{
		return gate::create(this, gate);
	}

	inline jive::argument *
	import(const jive::type & type, const std::string & name)
	{
		auto gate = create_gate(name, type);
		return root()->add_argument(nullptr, gate);
	}

	inline jive::input *
	export_port(jive::output * operand, const std::string & name)
	{
		jive::gate * gate = create_gate(name, operand->type());
		return root()->add_result(operand, nullptr, gate);
	}

	inline void
	prune()
	{
		root()->prune(true);
	}

	struct {
		jive::gate * first;
		jive::gate * last;
	} gates;

	/* FIXME: notifiers should become private, but need to turn more things
	 * into classes first */
	jive::notifier<jive::region *> on_region_create;
	jive::notifier<jive::region *> on_region_destroy;

	jive::notifier<jive::node *> on_node_create;
	jive::notifier<jive::node *> on_node_destroy;
	jive::notifier<jive::node *, size_t> on_node_depth_change;

	jive::notifier<jive::input *> on_input_create;
	jive::notifier<
		jive::input *,
		jive::output *,	/* old */
		jive::output *		/* new */
	> on_input_change;
	jive::notifier<jive::input *> on_input_destroy;
	
	jive::notifier<jive::output *> on_output_create;
	jive::notifier<jive::output *> on_output_destroy;

	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_add;
	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_remove;

private:
	bool normalized_;
	jive::region * root_;
	jive::node_normal_form_hash node_normal_forms_;
};

}

#endif
