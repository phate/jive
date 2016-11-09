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

/* graph tail node */

namespace jive {

class gate;

class graph_tail_operation final : public region_tail_op {
public:
	virtual
	~graph_tail_operation() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}

/* graph */

typedef struct jive_graph jive_graph;
typedef struct jive_tracker_depth_state jive_tracker_depth_state;
typedef struct jive_tracker_nodestate_list jive_tracker_nodestate_list;
typedef struct jive_tracker_slot_reservation jive_tracker_slot_reservation;

struct jive_node;
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

	struct {
		struct jive_node * first;
		struct jive_node * last;
	} bottom;

	struct {
		jive::gate * first;
		jive::gate * last;
	} gates;
	
	std::vector<jive_tracker_slot_reservation> tracker_slots;
	
	jive::node_normal_form_hash new_node_normal_forms;
	
	/* FIXME: notifiers should become private, but need to turn more things
	 * into classes first */
	jive::notifier<jive::region *> on_region_create;
	jive::notifier<jive::region *> on_region_destroy;

	jive::notifier<jive_node *> on_node_create;
	jive::notifier<jive_node *> on_node_destroy;
	jive::notifier<jive_node *, size_t> on_node_depth_change;
	
	jive::notifier<jive::input *> on_input_create;
	jive::notifier<
		jive::input *,
		jive::oport* /* old */,
		jive::oport* /* new */
	> on_input_change;
	jive::notifier<jive::input *> on_input_destroy;
	
	jive::notifier<jive::output *> on_output_create;
	jive::notifier<jive::output *> on_output_destroy;
	 
	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_add;
	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_remove;

private:
	bool normalized_;
	jive::region * root_;
};

jive::gate *
jive_graph_create_gate(jive_graph * self, const std::string & name, const jive::base::type & type);

JIVE_EXPORTED_INLINE void
jive_graph_export(struct jive_graph * self, jive::output * operand, const std::string & name)
{
	jive::gate * gate = jive_graph_create_gate(self, name.c_str(), operand->type());
	self->root()->bottom()->add_input(gate, operand);
}

JIVE_EXPORTED_INLINE void
jive_graph_export(struct jive_graph * self, jive::output * operand)
{
	//FIXME: this function should be removed
	jive_graph_export(self, operand, "dummy");
}

void
jive_graph_prune(jive_graph * self);

bool
jive_graph_has_active_traversers(const jive_graph * self);

jive::node_normal_form *
jive_graph_get_nodeclass_form(
	jive_graph * self,
	const std::type_info & type);

#endif
