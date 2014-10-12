/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/auxnodes.h>

#include <jive/arch/instructionset.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/common.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/variable.h>

#include <string.h>

static void
replace_splitnode(jive_shaped_node * shaped_node, jive_node * node)
{
	const jive_instructionset * isa = jive_region_get_instructionset(node->region);
	
	jive_xfer_description xfer = jive_instructionset_create_xfer(isa, node->region,
		node->inputs[0]->origin(), node->inputs[0]->required_rescls, node->outputs[0]->required_rescls);
	if (!xfer.input)
		xfer.input = jive_node_add_input(xfer.node, &node->inputs[0]->type(), node->inputs[0]->origin());
	if (!xfer.output)
		xfer.output = jive_node_add_output(xfer.node, &node->outputs[0]->type());
	
	jive_input_auto_assign_variable(xfer.input);
	jive_ssavar * outvar = node->outputs[0]->ssavar;
	jive_variable_merge(outvar->variable, jive_output_get_constraint(xfer.output));
	jive_ssavar_divert_origin(outvar, xfer.output);
	
	jive_cut * cut = shaped_node->cut;
	jive_shaped_node * pos = shaped_node->cut_location_list.next;
	jive_shaped_node_destroy(shaped_node);
	jive_cut_insert(cut, pos, xfer.node);
	
	jive_node_destroy(node);
}

static void
check_fp_sp_dependency(jive_node * node)
{
	/* FIXME: this is conceptually rather broken: the nodes depend on the
	stack *slots*, not the value of the stackptr/frameptr per se; this
	means that it would be much better to have the "stackptr add" operation
	perform a "write" to all stack slots to mark them as invalidated,
	and let "reuse" introduce ordering edges accordingly */
	const jive_subroutine_node * sub = jive_region_get_subroutine_node(node->region);
	if (!sub)
		return;
	jive_node * leave = sub->producer(0);
	jive_node * enter = leave->region->top;
	
	if (node == enter || node == leave)
		return;
	
	bool need_fp_dependency = false;
	bool need_sp_dependency = false;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(input->ssavar->variable);
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_CALLSLOT_RESOURCE))
			need_sp_dependency = true;
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_FRAMESLOT_RESOURCE))
			need_sp_dependency = true;
	}
	for (n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(output->ssavar->variable);
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_CALLSLOT_RESOURCE))
			need_sp_dependency = true;
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_FRAMESLOT_RESOURCE))
			need_sp_dependency = true;
	}
	
	if (need_fp_dependency) {
		jive::input * input = jive_subroutine_node_add_fp_dependency(sub, node);
		if (input)
			jive_input_auto_merge_variable(input);
	}
	if (need_sp_dependency) {
		jive::input * input = jive_subroutine_node_add_sp_dependency(sub, node);
		if (input)
			jive_input_auto_merge_variable(input);
	}
}

void
jive_regalloc_auxnodes_replace(jive_shaped_graph * shaped_graph)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(shaped_graph->graph);
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		if (dynamic_cast<const jive::split_operation *>(&node->operation())) {
			jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, node);
			replace_splitnode(shaped_node, node);
		} else
			check_fp_sp_dependency(node);
	}
	jive_traverser_destroy(traverser);
}
