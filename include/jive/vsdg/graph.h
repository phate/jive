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
struct jive_region;
struct jive_resource;
struct jive_resource_name;

struct jive_graph {
public:
	~jive_graph();

	jive_graph();

	struct {
		struct jive_node * first;
		struct jive_node * last;
	} bottom;
	
	struct jive_region * root_region;
	
	struct {
		jive::gate * first;
		jive::gate * last;
	} gates;
	
	struct {
		struct jive_variable * first;
		struct jive_variable * last;
	} variables;
	struct {
		struct jive_variable * first;
		struct jive_variable * last;
	} unused_variables;
	
	bool resources_fully_assigned;
	bool normalized;

	std::vector<jive_tracker_slot_reservation> tracker_slots;
	
	jive::node_normal_form_hash new_node_normal_forms;
	
	/* FIXME: notifiers should become private, but need to turn more things
	 * into classes first */
	jive::notifier<jive_region *> on_region_create;
	jive::notifier<jive_region *> on_region_destroy;
	jive::notifier<jive_region *, jive_ssavar *> on_region_add_used_ssavar;
	jive::notifier<jive_region *, jive_ssavar *> on_region_remove_used_ssavar;
	
	jive::notifier<jive_node *> on_node_create;
	jive::notifier<jive_node *> on_node_destroy;
	jive::notifier<jive_node *, size_t> on_node_depth_change;
	
	jive::notifier<jive::input *> on_input_create;
	jive::notifier<
		jive::input *,
		jive::output * /* old */, 
		jive::output * /* new */
	> on_input_change;
	jive::notifier<jive::input *> on_input_destroy;
	
	jive::notifier<jive::output *> on_output_create;
	jive::notifier<jive::output *> on_output_destroy;
	 
	jive::notifier<jive_variable *> on_variable_create;
	jive::notifier<jive_variable *> on_variable_destroy;
	jive::notifier<jive_variable *, jive::gate *> on_variable_assign_gate;
	jive::notifier<jive_variable *, jive::gate *> on_variable_unassign_gate;
	jive::notifier<
		jive_variable *,
		const jive_resource_class * /* old */,
		const jive_resource_class * /* new */
	> on_variable_resource_class_change;
	jive::notifier<
		jive_variable *,
		const jive_resource_name * /* old */,
		const jive_resource_name * /* new */
	> on_variable_resource_name_change;

	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_add;
	jive::notifier<jive::gate *, jive::gate *> on_gate_interference_remove;

	jive::notifier<jive_ssavar *> on_ssavar_create;
	jive::notifier<jive_ssavar *> on_ssavar_destroy;
	jive::notifier<jive_ssavar *, jive::input *> on_ssavar_assign_input;
	jive::notifier<jive_ssavar *, jive::input *> on_ssavar_unassign_input;
	jive::notifier<jive_ssavar *, jive::output *> on_ssavar_assign_output;
	jive::notifier<jive_ssavar *, jive::output *> on_ssavar_unassign_output;
	jive::notifier<
		jive_ssavar *,
		jive::output * /* old */,
		jive::output * /* new */
	> on_ssavar_divert_origin;
	jive::notifier<
		jive_ssavar *,
		jive_variable * /* old */,
		jive_variable * /* new */
	> on_ssavar_variable_change;

};

JIVE_EXPORTED_INLINE struct jive_region *
jive_graph_get_root_region(const jive_graph * self)
{
	return self->root_region;
}

jive::gate *
jive_graph_create_gate(jive_graph * self, const std::string & name, const jive::base::type & type);

JIVE_EXPORTED_INLINE void
jive_graph_export(struct jive_graph * self, jive::output * operand, const std::string & name)
{
	jive::gate * gate = jive_graph_create_gate(self, name.c_str(), operand->type());
	jive_node_gate_input(self->root_region->bottom, gate, operand);
}

JIVE_EXPORTED_INLINE void
jive_graph_export(struct jive_graph * self, jive::output * operand)
{
	//FIXME: this function should be removed
	jive_graph_export(self, operand, "dummy");
}

jive_graph *
jive_graph_create();

void
jive_graph_destroy(jive_graph * self);

jive_graph *
jive_graph_copy(jive_graph * self);

void
jive_graph_prune(jive_graph * self);

bool
jive_graph_has_active_traversers(const jive_graph * self);

jive::node_normal_form *
jive_graph_get_nodeclass_form(
	jive_graph * self,
	const std::type_info & type);

void
jive_graph_mark_denormalized(jive_graph * self);

void
jive_graph_normalize(jive_graph * self);

#endif
