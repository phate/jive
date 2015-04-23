/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shape.h>

#include <jive/arch/instruction.h>
#include <jive/common.h>
#include <jive/regalloc/reroute.h>
#include <jive/regalloc/selector-cost.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/splitnode.h>

static const jive_resource_class *
get_aux_rescls(const jive_node * node) {
	const jive::instruction_op * op =
		dynamic_cast<const jive::instruction_op *>(&node->operation());
	if (!op) {
		return nullptr;
	}
	if (op->icls()->flags & jive_instruction_commutative) {
		return nullptr;
	}
	if (!(op->icls()->flags & jive_instruction_write_input)) {
		return nullptr;
	}
	return &op->icls()->inregs[0]->base;
}

typedef struct jive_region_shaper jive_region_shaper;

enum jive_regalloc_conflict_type {
	jive_regalloc_conflict_none = 0,
	jive_regalloc_conflict_class = 1,
	jive_regalloc_conflict_name = 2
};

typedef struct jive_regalloc_conflict jive_regalloc_conflict;
struct jive_regalloc_conflict {
	enum jive_regalloc_conflict_type type;
	union {
		const jive_resource_class * rescls;
		jive_ssavar * ssavar;
	} item;
};

struct jive_region_shaper {
	jive_region_shaper * parent;
	jive_shaped_graph * shaped_graph;
	jive_region * region;
	jive_shaped_region * shaped_region;
	jive::regalloc::master_selector * master_selector;
	jive::regalloc::region_selector * region_selector;
	jive_node * control_dominator;
};

jive_region_shaper *
jive_region_shaper_create(
	jive_shaped_graph * shaped_graph,
	jive_region_shaper * parent,
	jive_region * region,
	jive::regalloc::master_selector * master_selector)
{
	jive_region_shaper * self = new jive_region_shaper;
	
	self->parent = parent;
	self->shaped_graph = shaped_graph;
	self->region = region;
	self->shaped_region = self->shaped_graph->map_region(region);
	self->master_selector = master_selector;
	self->region_selector = master_selector->map_region(region);
	self->control_dominator = 0;
	
	return self;
}

void
jive_region_shaper_destroy(jive_region_shaper * self)
{
	delete self;
}

static jive_regalloc_conflict
varcut_checked_add(jive_region_shaper * self, jive_varcut * varcut, jive_ssavar * ssavar)
{
	jive_shaped_ssavar * shaped_ssavar = self->shaped_graph->map_ssavar(ssavar);
	
	jive_regalloc_conflict conflict;
	conflict.type = jive_regalloc_conflict_none;
	
	if (varcut->shaped_ssavar_is_active(shaped_ssavar)) {
		return conflict;
	}
	
	jive_shaped_ssavar * other_shaped_ssavar = varcut->map_variable(ssavar->variable);
	if (other_shaped_ssavar) {
		conflict.type = jive_regalloc_conflict_name;
		conflict.item.ssavar = &other_shaped_ssavar->ssavar();
		return conflict;
	};
	
	const jive_resource_class * rescls = jive_variable_get_resource_class(ssavar->variable);
	
	const jive_resource_class * overflow = varcut->use_counts().check_add(rescls);
	if (overflow) {
		conflict.type = jive_regalloc_conflict_class;
		conflict.item.rescls = overflow;
		return conflict;
	}
	
	varcut->ssavar_add(shaped_ssavar, 1);

	return conflict;
}

static jive_regalloc_conflict
check_crossing_overflow(jive_region_shaper * self,
	const jive_shaped_node & shaped_node,
	const jive_node * new_node)
{
	/*
	  Check for overflow when changing crossed values.
	  
	  Check if the set of crossed values may be modified in the
	  specified ways. Returns either 'None' if the change can
	  be made without overflowing any register class, or one
	  of the exceeded register classes otherwise.
	*/
	jive_varcut active_before = shaped_node.get_active_before();
	jive_varcut active_after = shaped_node.get_active_after();

	for (size_t n = 0; n < new_node->noutputs; n++) {
		jive_shaped_ssavar * shaped_ssavar = self->shaped_graph->map_ssavar(
			new_node->outputs[n]->ssavar);
		active_before.ssavar_remove_full(shaped_ssavar);
		active_after.ssavar_remove_full(shaped_ssavar);
	}
	
	jive_regalloc_conflict conflict;
	conflict.type = jive_regalloc_conflict_none;
	
	for (size_t n = 0; n < new_node->ninputs; n++) {
		jive_ssavar * ssavar = new_node->inputs[n]->ssavar;
		conflict = varcut_checked_add(self, &active_before, ssavar);
		if (conflict.type != jive_regalloc_conflict_none) {
			return conflict;
		}
		conflict = varcut_checked_add(self, &active_after, ssavar);
		if (conflict.type != jive_regalloc_conflict_none) {
			return conflict;
		}
	}

	if (!shaped_node.node()->ninputs) {
		return conflict;
	}

	/*
	  Determine if the value of to the first input
	  of this node is passed through this node. For the
	  register allocator this means that it is forbidden
	  to reuse the first input register as output register,
	  while two-operand architectures require exactly that.
	  This is taken care of in later two-op fixup which in
	  turn may need an additional auxiliary register.
	  Detect this case and make sure the register is
	  available.
	*/
	jive_shaped_ssavar * shaped_ssavar = self->shaped_graph->map_ssavar(shaped_node.node()->inputs[0]->ssavar);
	if (active_after.shaped_ssavar_is_active(shaped_ssavar)) {
		const jive_resource_class * rescls = get_aux_rescls(shaped_node.node());
		const jive_resource_class * overflow = active_before.use_counts().check_add(rescls);
		if (!overflow) {
			overflow = active_after.use_counts().check_add(rescls);
		}
		if (overflow) {
			conflict.type = jive_regalloc_conflict_class;
			conflict.item.rescls = overflow;
		}
	}

	return conflict;
}

static bool
can_move_below_cut(jive_region_shaper * self, const jive_cut & cut, jive_node * new_node);

static bool
can_move_below_region(jive_region_shaper * self, jive_region * region, jive_node * new_node)
{
	jive_shaped_region * shaped_region = self->shaped_graph->map_region(region);
	for (const jive_cut & cut : shaped_region->cuts()) {
		if (!can_move_below_cut(self, cut, new_node)) {
			return false;
		}
	}
	return true;
}

static bool
can_move_below_cut(jive_region_shaper * self, const jive_cut & cut, jive_node * new_node)
{
	for (const jive_shaped_node & shaped_node : cut.nodes()) {
		jive_node * node = shaped_node.node();
		for (size_t n = 0; n < node->ninputs; n++) {
			jive::input * input = node->inputs[n];
			if (dynamic_cast<const jive::achr::type*>(&input->type())) {
				if (!can_move_below_region(self, input->producer()->region, new_node))
					return false;
			}
		}
		
		for (size_t n = 0; n < node->ninputs; n++) {
			if (node->producer(n) == new_node)
				return false;
		}
		
		jive_regalloc_conflict conflict = check_crossing_overflow(self, shaped_node, new_node);
		if (conflict.type != jive_regalloc_conflict_none) {
			return false;
		}
	}
	
	return true;
}

/* check whether "new_node" can be inserted before "shaped_node" without
 * overflowing register budget (including a possible reserve for two-op fixup */
static jive_regalloc_conflict
check_unshaped_crossing_overflow(
	jive_region_shaper * self,
	jive_shaped_node * shaped_node,
	jive_node * new_node)
{
	jive_varcut active_before = shaped_node->get_active_before();
	jive_varcut active_after = shaped_node->get_active_before();

	for (size_t n = 0; n < new_node->noutputs; ++n) {
		jive_ssavar * ssavar = new_node->outputs[n]->ssavar;
		active_before.ssavar_remove_full(self->shaped_graph->map_ssavar(ssavar));
	}
	for (size_t n = 0; n < new_node->ninputs; ++n) {
		jive_ssavar * ssavar = new_node->inputs[n]->ssavar;
		jive_regalloc_conflict conflict = varcut_checked_add(self, &active_before, ssavar);
		if (conflict.type != jive_regalloc_conflict_none) {
			return conflict;
		}
	}

	if (new_node->ninputs) {
		jive_ssavar * ssavar = new_node->inputs[0]->ssavar;
		jive_shaped_ssavar * shaped_ssavar = self->shaped_graph->map_ssavar(ssavar);
		if (active_after.shaped_ssavar_is_active(shaped_ssavar)) {
			const jive_resource_class * rescls = get_aux_rescls(new_node);
			const jive_resource_class * overflow = active_before.use_counts().check_add(rescls);
			if (overflow) {
				jive_regalloc_conflict conflict;
				conflict.type = jive_regalloc_conflict_class;
				conflict.item.rescls = overflow;
				return conflict;
			}
		}
	}

	jive_regalloc_conflict conflict;
	conflict.type = jive_regalloc_conflict_none;
	return conflict;
}

void
jive_region_shaper_pushdown_node(jive_region_shaper * self, jive_node * new_node)
{
	JIVE_DEBUG_ASSERT(!self->shaped_graph->is_node_placed(new_node));
	/* make sure nothing gets between predicate consumer and producer */
	
	JIVE_DEBUG_ASSERT(new_node == self->control_dominator || self->control_dominator == 0);
	
	jive_cut * allowed_cut = 0;
	
	size_t n;
	
	/* if this node either has subregions attached or has a control input,
	then it must go into a cut of its own */
	bool force_proper_cut = false;
	for (n = 0; n < new_node->ninputs; n++) {
		jive::input * input = new_node->inputs[n];
		if (dynamic_cast<const jive::achr::type*>(&input->type())
		|| dynamic_cast<const jive::ctl::type*>(&input->type()))
			force_proper_cut = true;
	}

	jive_shaped_region::cut_list & cuts = self->shaped_region->cuts();
	for (auto i = cuts.begin(); i != cuts.end(); ++i) {
		jive_cut * cut = i.ptr();
		/* test whether required inputs of new node can be passed
		through all nodes of this cut */
		if (!can_move_below_cut(self, *cut, new_node) || std::next(i) == cuts.end()) {
			break;
		}
		
		/* compute set of resources active after current cut
		(which equals the set of resources active before the first node
		of the next cut) */
		jive_shaped_node * next_loc = std::next(i)->nodes().begin().ptr();
		jive_node * next_node = next_loc->node();
		
		/* test whether new node can be placed into this position */
		jive_regalloc_conflict conflict = check_unshaped_crossing_overflow(self, next_loc, new_node);
		
		/* don't put node between a control dependency edge */
		bool is_control = false;
		for (size_t n = 0; n < next_node->ninputs; n++) {
			if (dynamic_cast<const jive::ctl::type*>(&next_node->inputs[n]->type())) {
				is_control = true;
			}
		}
		if (conflict.type == jive_regalloc_conflict_none && !is_control) {
			allowed_cut = cut;
		}
	}
	
	/* if moving node to any lower cut fails, create new topmost cut */
	if (!allowed_cut || force_proper_cut) {
		allowed_cut = self->shaped_region->create_top_cut();
	}
	
	/* FIXME: make sure nothings gets added to a cut containing just a region anchor */
	allowed_cut->append(new_node);
	
	if (new_node == self->control_dominator)
		self->control_dominator = 0;
	
	for (n = 0; n < new_node->ninputs; n++) {
		jive::input * input = new_node->inputs[n];
		if (!dynamic_cast<const jive::ctl::type*>(&input->type()))
			continue;
		
		self->control_dominator = input->producer();
	}
	
	for (n = 0; n < new_node->ninputs; n++) {
		JIVE_DEBUG_ASSERT(self->shaped_graph->map_ssavar(
			new_node->inputs[n]->ssavar)->boundary_region_depth()
				<= self->region->depth);
	}
}

static jive_regalloc_conflict
check_ssavar_replacement_conflict(
	const jive_varcut & ssavar_set,
	jive_shaped_ssavar * shaped_ssavar,
	jive_variable * new_var)
{
	const jive_varcut::value_type * original =
		ssavar_set.ssavar_map().find(shaped_ssavar).ptr();
	auto i = ssavar_set.variable_map().find(new_var);
	const jive_varcut::value_type * other =
		(i != ssavar_set.variable_map().end()) ? i.ptr() : nullptr;
	
	if (other && (original != other)) {
		jive_regalloc_conflict conflict;
		conflict.type = jive_regalloc_conflict_name;
		conflict.item.ssavar = &other->shaped_ssavar()->ssavar();
		return conflict;
	}
	
	const jive_resource_class * new_rescls = jive_variable_get_resource_class(new_var);
	const jive_resource_class * overflow = ssavar_set.use_counts().check_change(
		original ? original->rescls() : nullptr, new_rescls);
	
	if (overflow) {
		jive_regalloc_conflict conflict;
		conflict.type = jive_regalloc_conflict_class;
		conflict.item.rescls = overflow;
		return conflict;
	}
	
	jive_regalloc_conflict conflict;
	conflict.type = jive_regalloc_conflict_none;
	
	return conflict;
}

static bool
gate_is_unbound(const jive::gate * gate)
{
	jive::input * input;
	JIVE_LIST_ITERATE(gate->inputs, input, gate_inputs_list)
		if (input->ssavar) return false;
	jive::output * output;
	JIVE_LIST_ITERATE(gate->outputs, output, gate_outputs_list)
		if (output->ssavar) return false;
	return true;
}

void
jive_region_shaper_undo_setup_node(jive_region_shaper * self, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		jive_ssavar * ssavar = input->ssavar;
		if (ssavar) {
			jive_variable * variable = ssavar->variable;
			jive_ssavar_unassign_input(ssavar, input);
			jive::gate * gate = input->gate;
			if (gate && gate_is_unbound(gate))
				jive_gate_split(gate);
			jive_variable_recompute_rescls(variable);
		}
	}
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		jive_ssavar * ssavar = output->ssavar;
		if (ssavar) {
			jive_variable * variable = ssavar->variable;
			jive_ssavar_unassign_output(ssavar, output);
			jive::gate * gate = output->gate;
			if (gate && gate_is_unbound(gate))
				jive_gate_split(gate);
			jive_variable_recompute_rescls(variable);
		}
	}
}

static jive_ssavar *
select_spill(
	jive_region_shaper * self,
	jive_regalloc_conflict conflict,
	jive_node * disallow_origins)
{
	switch(conflict.type) {
		case jive_regalloc_conflict_class: {
			return self->region_selector->select_spill(
				conflict.item.rescls, disallow_origins);
		}
		case jive_regalloc_conflict_name: {
			return conflict.item.ssavar;
		}
		default: {
			JIVE_DEBUG_ASSERT(false);
			return 0;
		}
	}
}

static jive::output *
do_split_begin(
	jive_region_shaper * self,
	jive_ssavar * ssavar,
	const jive_resource_class_demotion * demotion)
{
	/*
		FIXME: reroute
		ssavar = reroute.reroute_at_point(ssavar, self.shaped_region.begin())
	*/
	jive::output * origin = ssavar->origin;
	
	const jive_resource_class * in_rescls = demotion->path[0];
	size_t n;
	for (n = 1; demotion->path[n]; n++) {
		const jive_resource_class * out_rescls = demotion->path[n];
		const jive::base::type * in_type = jive_resource_class_get_type(in_rescls);
		const jive::base::type * out_type = jive_resource_class_get_type(out_rescls);
		
		jive_node * node = jive_splitnode_create(origin->node()->region,
			in_type, origin, in_rescls,
			out_type, out_rescls);
		origin = node->outputs[0];
		in_rescls = out_rescls;
	}
	
	return origin;
}

static void
do_split_end(
	jive_region_shaper * self,
	jive_ssavar * ssavar,
	const jive_resource_class_demotion * demotion,
	jive::output * split_origin)
{
	jive::output * origin = split_origin;
	size_t n = 0;
	while(demotion->path[n+1]) n++;
	
	jive_region * region = self->region;
	
	const jive_resource_class * in_rescls = demotion->path[n];
	while (n != 0) {
		n--;
		const jive_resource_class * out_rescls = demotion->path[n];
		const jive::base::type * in_type = jive_resource_class_get_type(in_rescls);
		const jive::base::type * out_type = jive_resource_class_get_type(out_rescls);
		
		jive_node * node = jive_splitnode_create(region,
			in_type, origin, in_rescls,
			out_type, out_rescls);
		origin = node->outputs[0];
		in_rescls = out_rescls;
		self->region_selector->push_node_stack(node);
	}
	jive_ssavar_divert_origin(ssavar, origin);
}

static const jive_resource_class_demotion *
select_split_path(
	jive_region_shaper * self,
	jive_ssavar * ssavar,
	const jive_resource_class * required_rescls)
{
	const jive_resource_class * from_rescls = jive_variable_get_resource_class(ssavar->variable);
	const jive_resource_class_demotion * demotion = from_rescls->demotions;
	while(demotion->target) {
		/* pick one target such that it does no longer interfere with the
		resource class we need to free */
		if (jive_resource_class_intersection(demotion->target, required_rescls) != required_rescls)
			return demotion;
		demotion ++;
	}
	JIVE_DEBUG_ASSERT(false);
	return 0;
}

static void
resolve_conflict_spill(
	jive_region_shaper * self,
	const jive_resource_class * rescls,
	jive_regalloc_conflict conflict,
	jive_node * disallow_origins)
{
	jive_ssavar * to_spill = select_spill(self, conflict, disallow_origins);
	JIVE_DEBUG_ASSERT(to_spill);
	to_spill = jive_regalloc_reroute_at_point(to_spill, self->shaped_region->first_in_region());
	JIVE_DEBUG_ASSERT(to_spill);
	
	const jive_resource_class_demotion * demotion = select_split_path(self, to_spill, rescls);
	
	jive::output * split_origin = do_split_begin(self, to_spill, demotion);
	do_split_end(self, to_spill, demotion, split_origin);
}

bool
jive_region_shaper_setup_node(jive_region_shaper * self, jive_node * node)
{
	jive_varcut active = self->shaped_region->active_top();
	
	for (size_t n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		
		JIVE_DEBUG_ASSERT(!output->ssavar);
		jive_shaped_ssavar * shaped_ssavar = active.map_output(output);
		
		jive_variable * new_constraint = jive_output_get_constraint(output);
		
		jive_regalloc_conflict conflict;
		conflict = check_ssavar_replacement_conflict(active, shaped_ssavar, new_constraint);
		
		if (conflict.type != jive_regalloc_conflict_none) {
			jive_region_shaper_undo_setup_node(self, node);
			resolve_conflict_spill(self, jive_variable_get_resource_class(new_constraint), conflict, NULL);
			return false;
		}
		
		jive_shaped_variable * shaped_variable = 0;
		if (shaped_ssavar) {
			shaped_variable = self->shaped_graph->map_variable(shaped_ssavar->ssavar().variable);
		}
		bool merge_conflict =
			shaped_variable && !shaped_variable->can_merge(new_constraint);
		
		if (merge_conflict) {
			jive_region_shaper_undo_setup_node(self, node);
			
			const jive_resource_class * current_rescls =
				jive_variable_get_resource_class(shaped_ssavar->ssavar().variable);
			const jive_resource_class * new_rescls = jive_variable_get_resource_class(new_constraint);
			new_rescls = jive_resource_class_relax(new_rescls);
			const jive::base::type * type = &output->type();
			
			jive_node * split_node = jive_splitnode_create(self->region,
				type, output, new_rescls,
				type, current_rescls);
			
			jive_ssavar_divert_origin(&shaped_ssavar->ssavar(), split_node->outputs[0]);
			return false;
		}
		
		if (!shaped_ssavar) {
			jive_ssavar * ssavar = jive_ssavar_create(output, new_constraint);
			jive_ssavar_assign_output(ssavar, output);
			shaped_ssavar = self->shaped_graph->map_ssavar(ssavar);
			active.ssavar_add(shaped_ssavar, 1);
		} else {
			jive_ssavar & ssavar = shaped_ssavar->ssavar();
			jive_variable_merge(ssavar.variable, new_constraint);
			jive_ssavar_assign_output(&ssavar, output);
			active.ssavar_rescls_change(
				shaped_ssavar,
				jive_variable_get_resource_class(ssavar.variable));
		}
		
		jive_ssavar_assert_consistent(&shaped_ssavar->ssavar());
	}
	
	for (size_t n = 0; n < node->noutputs ; n++) {
		jive::output * output = node->outputs[n];
		active.ssavar_remove_full(self->shaped_graph->map_ssavar(output->ssavar));
	}
	
	for (size_t n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		JIVE_DEBUG_ASSERT(!input->ssavar);
		jive_shaped_ssavar * shaped_ssavar = active.map_output(input->origin());
		jive_variable * new_constraint = jive_input_get_constraint(input);
		
		jive_shaped_variable * shaped_variable = 0;
		if (shaped_ssavar) {
			shaped_variable = self->shaped_graph->map_variable(shaped_ssavar->ssavar().variable);
		}
		bool merge_conflict =
			shaped_variable && !shaped_variable->can_merge(new_constraint);
		
		if (merge_conflict) {
			jive_region_shaper_undo_setup_node(self, node);
			
			/* FIXME: reroute is missing here!!! */
			jive::output * output = shaped_ssavar->ssavar().origin;
			const jive_resource_class * current_rescls =
				jive_variable_get_resource_class(shaped_ssavar->ssavar().variable);
			const jive_resource_class * new_rescls = jive_variable_get_resource_class(new_constraint);
			new_rescls = jive_resource_class_relax(new_rescls);
			const jive::base::type * type = &output->type();
			
			jive_node * split_node = jive_splitnode_create(self->region,
				type, output, new_rescls,
				type, current_rescls);
			
			jive_ssavar_divert_origin(&shaped_ssavar->ssavar(), split_node->outputs[0]);
			
			self->region_selector->push_node_stack(split_node);
			
			return false;
		}
		
		jive_regalloc_conflict conflict;
		conflict = check_ssavar_replacement_conflict(active, shaped_ssavar, new_constraint);
		
		if (conflict.type != jive_regalloc_conflict_none) {
			jive_region_shaper_undo_setup_node(self, node);
			resolve_conflict_spill(self, jive_variable_get_resource_class(new_constraint), conflict, node);
			return false;
		}
		
		if (!shaped_ssavar) {
			jive_ssavar * ssavar = jive_ssavar_create(input->origin(), new_constraint);
			jive_ssavar_assign_input(ssavar, input);
			shaped_ssavar = self->shaped_graph->map_ssavar(ssavar);
			active.ssavar_add(shaped_ssavar, 1);
		} else {
			jive_ssavar & ssavar = shaped_ssavar->ssavar();
			jive_variable_merge(ssavar.variable, new_constraint);
			jive_ssavar_assign_input(&ssavar, input);
			active.ssavar_rescls_change(
				shaped_ssavar,
				jive_variable_get_resource_class(ssavar.variable));
		}
		
		shaped_ssavar->lower_boundary_region_depth(self->region->depth);
	}
	
	if (node->ninputs) {
		jive_shaped_ssavar * shaped_ssavar =
			self->shaped_graph->map_ssavar(node->inputs[0]->ssavar);
		if (self->shaped_region->active_top().shaped_ssavar_is_active(shaped_ssavar)) {
			const jive_resource_class * aux_rescls = get_aux_rescls(node);
			
			const jive_resource_class * overflow = active.use_counts().check_add(aux_rescls);
			if (overflow) {
				jive_region_shaper_undo_setup_node(self, node);
				jive_regalloc_conflict conflict;
				conflict.type = jive_regalloc_conflict_class;
				conflict.item.rescls = overflow;
				resolve_conflict_spill(self, aux_rescls, conflict, node);
				return false;
			}
		}
	}
	
	return true;
}

bool
jive_region_shaper_process_node(jive_region_shaper * self, jive_node * node)
{
	/*
		make sure that register constraints of node
		outputs are satisfied; merge existing constraints
		or copy values as necessary
	*/
	bool success = jive_region_shaper_setup_node(self, node);
	
	if (success)
		jive_region_shaper_pushdown_node(self, node);
	
	return success;
}

jive_node *
jive_region_shaper_pick_node(jive_region_shaper * self)
{
	if (self->control_dominator)
		return self->control_dominator;
	
	return self->region_selector->select_node();
}

static void
flush_pending(jive_region_shaper * self)
{
	for (;;) {
		jive_node * node = self->region_selector->select_node();
		if (!node)
			break;
		
		if (jive_region_shaper_setup_node(self, node))
			jive_region_shaper_pushdown_node(self, node);
	}
}

static void
split_merge_subregion(
	jive_region_shaper * self,
	jive_region_shaper * subregion,
	jive_ssavar * ssavar)
{
	const jive_resource_class * from_rescls = jive_variable_get_resource_class(ssavar->variable);
	const jive_resource_class_demotion * demotion = from_rescls->demotions;
	
	const jive_resource_class_count & parent_use_counts =
		self->shaped_region->active_top().use_counts();
	const jive_resource_class_count & child_use_counts =
		subregion->shaped_region->active_top().use_counts();
	
	while(demotion->target) {
		/* demote to target resource class that... */
		const jive_resource_class * tgt_rescls = demotion->target;
		/* ... can be added to the current position in parent region ... */
		if (parent_use_counts.check_add(tgt_rescls)) {
			demotion ++;
			continue;
		}
		/* ... and that can replace the original one in sub region ... */
		if (child_use_counts.check_change(from_rescls, tgt_rescls)) {
			demotion ++;
			continue;
		}
		break;
	}
	JIVE_DEBUG_ASSERT(demotion->target);
	
	jive::output * split_origin = do_split_begin(self, ssavar, demotion);
	do_split_end(subregion, ssavar, demotion, split_origin);
	flush_pending(subregion);
}

static bool
merge_single_ssavar_from_subregion(
	jive_region_shaper * self,
	jive_region_shaper * subregion,
	jive_shaped_ssavar * shaped_ssavar)
{
	jive_ssavar & ssavar = shaped_ssavar->ssavar();
		
	if (shaped_ssavar->boundary_region_depth() <= self->region->depth) {
		/* if already merged into this region, then nothing to do */
		return true;
	}
	
	jive_regalloc_conflict conflict;
	
	/* determine ssavar in parent leading to same origin (if active at all) */
		
	jive_shaped_ssavar * parent_origin_shaped_ssavar =
		self->shaped_region->active_top().map_output(ssavar.origin);
	conflict = check_ssavar_replacement_conflict(
		self->shaped_region->active_top(),
		parent_origin_shaped_ssavar,
		ssavar.variable);
	if (conflict.type == jive_regalloc_conflict_name) {
		/* some other variable in parent cannot be active at the same
		time as this variable -- need to split in subregion */
		
		jive::output * output = ssavar.origin;
		const jive::base::type * type = &output->type();
		
		const jive_resource_class * rescls = jive_variable_get_resource_class(ssavar.variable);
		rescls = jive_resource_class_relax(rescls);
		
		jive_node * split_node = jive_splitnode_create(subregion->region,
			type, output, rescls,
			type, rescls);
		
		jive_ssavar_divert_origin(&ssavar, split_node->outputs[0]);
		subregion->region_selector->push_node_stack(split_node);
		flush_pending(subregion);
		
		return false;
	} else if (conflict.type == jive_regalloc_conflict_class) {
		/* cannot put ssavar from subregion into parent due to resource
		limitations - must spill between the two */
		
		split_merge_subregion(self, subregion, &ssavar);
		return false;
	}
	
	/* we are now certain that the value from the subregion could be put into the parent,
	but it might still not be possible to merge the parent's constraints with the
	value in the subregion */
	
	jive_shaped_variable * shaped_variable =
		self->shaped_graph->map_variable(ssavar.variable);
	if (parent_origin_shaped_ssavar &&
		!shaped_variable->can_merge(parent_origin_shaped_ssavar->ssavar().variable)) {
		/* nope, applying parent constraint to var in subregion causes some kind
		of conflict, split */
		split_merge_subregion(self, subregion, &ssavar);
		return false;
	}
	
	if (parent_origin_shaped_ssavar) {
		/* can unify constraints between parent and subregion, so trivial
		merge does it already */
		jive_variable_merge(parent_origin_shaped_ssavar->ssavar().variable, ssavar.variable);
		jive_ssavar_merge(&parent_origin_shaped_ssavar->ssavar(), &ssavar);
		return true;
	}
	
	/* no conflicts, just pass through to parent */
	shaped_ssavar->lower_boundary_region_depth(self->region->depth);
	return true;
}

static void
merge_ssavars_from_subregion(jive_region_shaper * self, jive_region_shaper * sub)
{
	bool redo = true;
	const jive_varcut & active_top = sub->shaped_region->active_top();
	while (redo) {
		redo = false;
		auto i = active_top.begin();
		while (i != active_top.end()) {
			const jive_cutvar_xpoint * xpoint = i.ptr();
			++i;
			bool success = merge_single_ssavar_from_subregion(self, sub, xpoint->shaped_ssavar());
			if (!success) {
				redo = true;
				break;
			}
		}
	}
}

void
jive_region_shaper_process(jive_region_shaper * self);

void
jive_region_shaper_process_subregions(jive_region_shaper * self, jive_node * new_node)
{
	jive_region_shaper * subshapers[new_node->ninputs];
	size_t nsubshapers = 0;
	
	size_t n;
	for (n = 0; n < new_node->ninputs; n++) {
		jive::input * input = new_node->inputs[n];
		if (!dynamic_cast<const jive::achr::type*>(&input->type()))
			continue;
		jive_region_shaper * subshaper = jive_region_shaper_create(
			self->shaped_graph,
			self,
			input->producer()->region,
			self->master_selector);
		subshapers[nsubshapers++] = subshaper;
	}
	
	for (n = 0; n < nsubshapers; n++)
		jive_region_shaper_process(subshapers[n]);
	
	for (n = 0; n < nsubshapers; n++) {
		merge_ssavars_from_subregion(self, subshapers[n]);
		jive_region_shaper_destroy(subshapers[n]);
	}
}

void
jive_region_shaper_process(jive_region_shaper * self)
{
	for (;;) {
		jive_node * node = jive_region_shaper_pick_node(self);
		
		if (!node)
			break;
		
		bool success = jive_region_shaper_process_node(self, node);
		if (!success)
			continue;
		
		jive_region_shaper_process_subregions(self, node);
	}
}

jive_shaped_graph *
jive_regalloc_shape(jive_graph * graph)
{
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive::regalloc::master_selector_cost selector(shaped_graph);
	jive_region_shaper * region_shaper = jive_region_shaper_create(
		shaped_graph, nullptr, graph->root_region, &selector);
	jive_region_shaper_process(region_shaper);
	
	jive_region_shaper_destroy(region_shaper);
	
	return shaped_graph;
}
