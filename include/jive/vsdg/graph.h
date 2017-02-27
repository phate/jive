/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GRAPH_H
#define JIVE_VSDG_GRAPH_H

#include <stdbool.h>
#include <stdlib.h>

#include <typeindex>

#include <jive/common.h>
#include <jive/util/callbacks.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node-normal-form.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/tracker.h>

/* graph */

typedef struct jive_graph jive_graph;
typedef struct jive_tracker_depth_state jive_tracker_depth_state;
typedef struct jive_tracker_nodestate_list jive_tracker_nodestate_list;
typedef struct jive_tracker_slot_reservation jive_tracker_slot_reservation;

struct jive_resource;
struct jive_resource_name;

struct jive_graph {
public:
	~jive_graph();

	jive_graph();

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

	void
	normalize();

	std::unique_ptr<jive_graph>
	copy() const;

	jive::node_normal_form *
	node_normal_form(const std::type_info & type) noexcept;

	bool
	has_active_traversers() const noexcept;

	jive::gate *
	create_gate(
		const jive::base::type & type,
		const std::string & name,
		const jive_resource_class * rescls = &jive_root_resource_class);

	inline jive::argument *
	import(const jive::base::type & type, const std::string & name)
	{
		auto gate = create_gate(type, name);
		return root()->add_argument(nullptr, gate);
	}

	inline jive::iport *
	export_port(jive::oport * operand, const std::string & name)
	{
		jive::gate * gate = create_gate(operand->type(), name);
		return root()->add_result(operand, nullptr, gate);
	}

	void
	prune();

	struct {
		jive::gate * first;
		jive::gate * last;
	} gates;
	
	std::vector<jive_tracker_slot_reservation> tracker_slots;

	/* FIXME: notifiers should become private, but need to turn more things
	 * into classes first */
	jive::notifier<jive::region *> on_region_create;
	jive::notifier<jive::region *> on_region_destroy;

	jive::notifier<jive::node *> on_node_create;
	jive::notifier<jive::node *> on_node_destroy;
	jive::notifier<jive::node *, size_t> on_node_depth_change;

	jive::notifier<jive::iport *> on_iport_create;
	jive::notifier<
		jive::iport *,
		jive::oport *,	/* old */
		jive::oport *		/* new */
	> on_iport_change;
	jive::notifier<jive::iport *> on_iport_destroy;
	
	jive::notifier<jive::oport *> on_oport_create;
	jive::notifier<jive::oport *> on_oport_destroy;

	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_add;
	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_remove;

private:
	bool normalized_;
	jive::region * root_;
	jive::node_normal_form_hash node_normal_forms_;
};

#endif
