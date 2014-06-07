/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/stackframe.h>

#include <jive/arch/immediate-node.h>
#include <jive/arch/instruction.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/util/hash.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

const jive_stackslot *
get_req_stackslot(jive_variable * variable)
{
	const jive_resource_class * rescls = variable->rescls;
	if (!jive_resource_class_isinstance(rescls, &JIVE_STACK_RESOURCE))
		return NULL;
	
	const jive_resource_name * name = jive_variable_get_resource_name(variable);
	if (!name && rescls->limit)
		name = rescls->names[0];
	
	return (const jive_stackslot *) name;
}

static void
layout_stackslot(
	jive_shaped_graph * shaped_graph,
	jive_subroutine_stackframe_info * frame,
	jive_variable * variable)
{
	if (variable->resname)
		return;
	
	/* unless this is a stackslot class, bail out */
	const jive_resource_class * rescls = variable->rescls;
	if (rescls->class_ != &JIVE_STACK_RESOURCE)
		return;
	
	const jive_stackslot_size_class * cls = (const jive_stackslot_size_class *) rescls;
	
	if (rescls->limit) {
		const jive_resource_name * name = rescls->names[0];
		jive_variable_set_resource_name(variable, name);
	}
	
	ssize_t lower_bound = frame->lower_bound & ~(cls->alignment - 1);
	size_t nslots = frame->upper_bound - lower_bound;
	bool allowed_slot[nslots];
	size_t n;
	for (n = 0; n < nslots; n++)
		allowed_slot[n] = true;
	
	jive_shaped_variable * shaped_variable = jive_shaped_graph_map_variable(shaped_graph, variable);
	
	struct jive_variable_interference_hash_iterator i;
	JIVE_HASH_ITERATE(jive_variable_interference_hash, shaped_variable->interference, i) {
		jive_shaped_variable * other_var = i.entry->shaped_variable;
		
		const jive_stackslot * other_slot = get_req_stackslot(other_var->variable);
		if (!other_slot)
			continue;
		const jive_stackslot_size_class * other_cls =
			(const jive_stackslot_size_class *) other_slot->base.resource_class;
		
		for (n = 0; n < other_cls->size; n++)
			allowed_slot[n + other_slot->offset - lower_bound] = false;
	}
	
	ssize_t offset = frame->upper_bound - cls->size;
	offset = offset & ~(cls->alignment - 1);
	
	while (offset >= lower_bound) {
		for (n = 0; n < cls->size; n++) {
			if (!allowed_slot[offset - lower_bound + n])
				break;
		}
		if (n == cls->size)
			break;
		offset -= cls->alignment;
	}
	
	const jive_resource_name * name = jive_stackslot_name_get(cls->size, cls->alignment, offset);
	jive_variable_set_resource_name(variable, name);
	
	if (offset < frame->lower_bound)
		frame->lower_bound = offset;
}

const jive_stackslot *
get_node_frameslot(jive_node * node)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (!input->ssavar)
			continue;
		const jive_resource_name * name = jive_variable_get_resource_name(input->ssavar->variable);
		if (jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_FRAMESLOT_RESOURCE))
			return (const jive_stackslot *) name;
	}
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(output->ssavar->variable);
		if (jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_FRAMESLOT_RESOURCE))
			return (const jive_stackslot *) name;
	}
	return NULL;
}

const jive_callslot *
get_node_callslot(jive_node * node)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (!input->ssavar)
			continue;
		const jive_resource_name * name = jive_variable_get_resource_name(input->ssavar->variable);
		if (jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_CALLSLOT_RESOURCE))
			return (const jive_callslot *) name;
	}
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(output->ssavar->variable);
		if (jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_CALLSLOT_RESOURCE))
			return (const jive_callslot *) name;
	}
	return NULL;
}

static void
update_call_area_size(
	jive_graph * graph,
	jive_subroutine_stackframe_info * frame,
	jive_variable * variable)
{
	/* unless this is a callslot class, bail out */
	const jive_resource_class * rescls = variable->rescls;
	if (rescls->class_ != &JIVE_STACK_CALLSLOT_RESOURCE)
		return;
	
	const jive_callslot_class * cls = (const jive_callslot_class *) rescls;
	size_t limit = cls->offset + cls->base.size;
	if (frame->call_area_size < limit)
		frame->call_area_size = limit;
}

static void
layout_stackslots(jive_shaped_graph * shaped_graph, jive_graph * graph)
{
	jive_traverser * trav = jive_bottomup_traverser_create(graph);
	
	jive_node * node;
	for ( node = jive_traverser_next(trav); node; node = jive_traverser_next(trav) ) {
		jive_subroutine_node * sub = jive_region_get_subroutine_node(node->region);
		if (!sub)
			continue;
		jive_subroutine_stackframe_info * frame = jive_subroutine_node_get_stackframe_info(sub);
		size_t n;
		for (n = 0; n < node->noutputs; n++) {
			jive_output * output = node->outputs[n];
			if (!output->ssavar)
				continue;
			jive_variable * variable = output->ssavar->variable;
			layout_stackslot(shaped_graph, frame, variable);
			update_call_area_size(graph, frame, variable);
		}
	}
	
	jive_traverser_destroy(trav);
}

void
jive_regalloc_stackframe(jive_shaped_graph * shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	
	layout_stackslots(shaped_graph, graph);
}

static void
reloc_stack_access(jive_node * node)
{
	
	const jive_instruction_node * inode = jive_instruction_node_cast(node);
	if (!inode)
		return;
	
	const jive_instruction_class * icls = inode->operation().icls();
	size_t n;
	for (n = 0; n < icls->nimmediates; n++) {
		jive::input * imm_input = node->inputs[n + icls->ninputs];
		jive_immediate_node * immnode = jive_immediate_node_cast(imm_input->producer());
		JIVE_DEBUG_ASSERT(immnode);
		
		jive_subroutine_node * sub = jive_region_get_subroutine_node(node->region);
		JIVE_DEBUG_ASSERT(sub);
		jive_subroutine_stackframe_info * frame = jive_subroutine_node_get_stackframe_info(sub);
		
		jive_immediate imm = immnode->operation().value();
		if (imm.add_label == &jive_label_fpoffset) {
			const jive_stackslot * slot = get_node_frameslot(node);
			imm = jive_immediate_add_offset(&imm, slot->offset - frame->frame_pointer_offset);
			imm.add_label = 0;
		}
		if (imm.sub_label == &jive_label_fpoffset) {
			const jive_stackslot * slot = get_node_frameslot(node);
			imm = jive_immediate_add_offset(&imm, -slot->offset + frame->frame_pointer_offset);
			imm.sub_label = 0;
		}
		if (imm.add_label == &jive_label_spoffset) {
			const jive_callslot * slot = get_node_callslot(node);
			imm = jive_immediate_add_offset(&imm, slot->offset - frame->stack_pointer_offset);
			imm.add_label = 0;
		}
		if (imm.sub_label == &jive_label_spoffset) {
			const jive_callslot * slot = get_node_callslot(node);
			imm = jive_immediate_add_offset(&imm, -slot->offset + frame->stack_pointer_offset);
			imm.sub_label = 0;
		}
		if (!jive_immediate_equals(&imm, &immnode->operation().value())) {
			jive_output * new_immval = jive_immediate_create(node->region->graph, &imm);
			imm_input->divert_origin(new_immval);
		}
	}
}

typedef struct jive_regalloc_stackframe_transforms {
	jive_subroutine_late_transforms base;
	jive_shaped_graph * shaped_graph;
	jive_region * region;
} jive_regalloc_stackframe_transforms;


static void
do_split(
	const jive_subroutine_late_transforms * self_,
	jive_output * port_in, jive::input * port_out,
	const jive_value_split_factory * enter_split,
	const jive_value_split_factory * leave_split)
{
	const jive_regalloc_stackframe_transforms * self =
		(const jive_regalloc_stackframe_transforms *) self_;
	
	size_t n;
	
	jive_ssavar * ssavar_interior = port_in->ssavar;
	jive_output * value_interior = enter_split->split(enter_split, port_in);
	jive_node * enter_split_node = value_interior->node();
	
	jive_ssavar_divert_origin(ssavar_interior, value_interior);
	jive_ssavar * ssavar_enter = jive_ssavar_create(port_in, ssavar_interior->variable);
	jive_ssavar_assign_output(ssavar_enter, port_in);
	for (n = 0; n < enter_split_node->ninputs; n++) {
		jive::input * input = enter_split_node->inputs[n];
		if (input->origin() == port_in)
			jive_ssavar_assign_input(ssavar_enter, input);
		else
			jive_input_auto_merge_variable(input);
	}
	for (n = 0; n < enter_split_node->noutputs; n++) {
		jive_output * output = enter_split_node->outputs[n];
		if (output != value_interior)
			jive_output_auto_merge_variable(output);
	}
	
	/* FIXME: in case of "dominated" gates, may have to create multiple
	"leave_split" nodes */
	
	jive_output * value_leave = leave_split->split(leave_split, value_interior);
	jive_ssavar * ssavar_leave = jive_ssavar_create(value_leave, ssavar_interior->variable);
	jive_node * leave_split_node = value_leave->node();
	for (n = 0; n < leave_split_node->ninputs; n++) {
		jive::input * input = leave_split_node->inputs[n];
		if (input->origin() == value_interior)
			jive_ssavar_assign_input(ssavar_interior, input);
		else
			jive_input_auto_merge_variable(input);
	}
	for (n = 0; n < leave_split_node->noutputs; n++) {
		jive_output * output = leave_split_node->outputs[n];
		if (output == value_leave)
			jive_ssavar_assign_output(ssavar_leave, output);
		else
			jive_output_auto_merge_variable(output);
	}
	jive_ssavar_unassign_input(ssavar_interior, port_out);
	port_out->divert_origin(value_leave);
	jive_ssavar_assign_input(ssavar_leave, port_out);
	
	/* insert at beginning/end of region */
	jive_shaped_node * p;
	
	p = jive_shaped_graph_map_node(self->shaped_graph, self->region->top);
	p = jive_shaped_node_next_in_region(p);
	
	jive_cut_append(jive_cut_split(p->cut, p), enter_split_node);
	
	p = jive_shaped_graph_map_node(self->shaped_graph, self->region->bottom);
	jive_cut_append(jive_cut_split(p->cut, p), leave_split_node);
}

static void
region_subroutine_prepare_stackframe(jive_shaped_graph * shaped_graph, jive_region * region)
{
	if (!region->anchor)
		return;
	jive_node * node_ = region->anchor->node;
	jive_subroutine_node * node = jive_subroutine_node_cast(node_);
	if (!node)
		return;
	
	jive_regalloc_stackframe_transforms xfrm;
	xfrm.base.value_split = do_split;
	xfrm.shaped_graph = shaped_graph;
	xfrm.region = region;
	
	jive_subroutine_node_prepare_stackframe(node, &xfrm.base);
}

static void
region_subroutine_prepare_stackframe_recursive(
	jive_shaped_graph * shaped_graph,
	jive_region * region)
{
	region_subroutine_prepare_stackframe(shaped_graph, region);
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list) {
		region_subroutine_prepare_stackframe_recursive(shaped_graph, subregion);
	}
}

void
jive_regalloc_relocate_stackslots(struct jive_shaped_graph * shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	region_subroutine_prepare_stackframe_recursive(shaped_graph, graph->root_region);
	
	jive_traverser * trav = jive_bottomup_traverser_create(graph);
	
	jive_node * node;
	for ( node = jive_traverser_next(trav); node; node = jive_traverser_next(trav) ) {
		reloc_stack_access(node);
	}
	jive_traverser_destroy(trav);
}

