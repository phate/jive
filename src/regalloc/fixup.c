/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/fixup.h>

#include <jive/arch/instruction.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

static void
pre_op_transfer(
	jive_shaped_graph * shaped_graph,
	jive_node * node,
	const jive_resource_name * new_cpureg)
{
	jive_output * origin = node->inputs[0]->origin();
	
	const jive_resource_class * resource_class =
		jive_variable_get_resource_class(origin->ssavar->variable);
	resource_class = jive_resource_class_relax(resource_class);
	const jive::base::type * type = jive_resource_class_get_type(resource_class);
	
	jive_node * xfer_node = jive_splitnode_create(
		node->region,
		type, origin, resource_class,
		type, resource_class);
	
	jive_input * xfer_input = xfer_node->inputs[0];
	jive_output * xfer_output = xfer_node->outputs[0];
	
	jive_input_auto_assign_variable(xfer_input);
	jive_input_unassign_ssavar(node->inputs[0]);
	
	/* divert & insert */
	node->inputs[0]->divert_origin(xfer_output);
	jive_ssavar * new_var = jive_output_auto_merge_variable(xfer_output);
	
	jive_shaped_node * p = jive_shaped_graph_map_node(shaped_graph, node);
	jive_cut_append(jive_cut_split(p->cut, p), xfer_node);
	
	/* assign register now that the split is complete */
	jive_variable_set_resource_name(new_var->variable, new_cpureg);
}

static void
post_op_transfer(
	jive_shaped_graph * shaped_graph,
	jive_node * node,
	const jive_resource_name * new_cpureg)
{
	jive_output * origin = node->outputs[0];
	
	const jive_resource_class * resource_class =
		jive_variable_get_resource_class(origin->ssavar->variable);
	resource_class = jive_resource_class_relax(resource_class);
	const jive::base::type * type = jive_resource_class_get_type(resource_class);
	
	jive_node * xfer_node = jive_splitnode_create(node->region,
		type, origin, resource_class,
		type, resource_class);
	
	jive_output * xfer_output = xfer_node->outputs[0];
	
	jive_ssavar * moved_var = origin->ssavar;
	
	/* divert & insert */
	jive_ssavar_divert_origin(moved_var, xfer_output);
	
	jive_shaped_node * p = jive_shaped_graph_map_node(shaped_graph, node);
	jive_cut_append(jive_cut_split(p->cut, p->cut_location_list.next), xfer_node);
	
	jive_ssavar * out_var = jive_output_auto_merge_variable(origin);
	/* assign register now that the split is complete */
	jive_variable_set_resource_name(out_var->variable, new_cpureg);
}

static void
process_node(jive_shaped_graph * shaped_graph, jive_node * node)
{
	if (!jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) return;
	const struct jive_instruction_class * icls = ((jive_instruction_node *) node)->operation().icls();
	
	jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, node);
	
	if ((icls->flags & jive_instruction_write_input) == 0) return;
	
	const jive_resource_name * inreg0 = jive_variable_get_resource_name(
		node->inputs[0]->ssavar->variable);
	const jive_resource_name * outreg0 = jive_variable_get_resource_name(
		node->outputs[0]->ssavar->variable);
	
	if (inreg0 == outreg0) {
		jive_variable_merge(node->inputs[0]->ssavar->variable, node->outputs[0]->ssavar->variable);
		return;
	}
	
	const jive_resource_class * rescls = &icls->inregs[0]->base;
	
	if (icls->flags & jive_instruction_commutative) {
		const jive_resource_name * inreg1 = jive_variable_get_resource_name(
			node->inputs[1]->ssavar->variable);
		/* if it is possible to satify constraints by simply swapping inputs, do it */
		if (outreg0 == inreg1) {
			node->inputs[0]->swap(node->inputs[1]);
			jive_variable_merge(node->inputs[0]->ssavar->variable, node->outputs[0]->ssavar->variable);
			return;
		}
		
		jive_shaped_variable * var1 = jive_shaped_graph_map_variable(
			shaped_graph, node->inputs[0]->ssavar->variable);
		jive_shaped_variable * var2 = jive_shaped_graph_map_variable(
			shaped_graph, node->inputs[1]->ssavar->variable);
		
		/* if swapping makes the first operand overwritable, do it */
		if (jive_shaped_variable_is_crossing(var1, shaped_node)
			&& !jive_shaped_variable_is_crossing(var2, shaped_node))
			node->inputs[0]->swap(node->inputs[1]);
		inreg0 = jive_variable_get_resource_name(node->inputs[0]->ssavar->variable);
	}
	
	if (inreg0 == outreg0) {
		jive_variable_merge(node->inputs[0]->ssavar->variable, node->outputs[0]->ssavar->variable);
		return;
	}
	
	if (jive_shaped_node_is_resource_name_active_after(shaped_node, inreg0)) {
		/* input register #0 is used after this instruction, therefore
		must not overwrite it; move calculation into a temporary
		register, preferably using the final destination register
		outright */
		
		const jive_resource_name * reg = 0;
		if (jive_resource_class_count_check_add(&shaped_node->use_count_before,
			outreg0->resource_class) == 0) {
			reg = outreg0;
		}
		
		if (!reg) {
			size_t n;
			for(n=0; n<rescls->limit; n++) {
				const jive_resource_name * name = rescls->names[n];
				if (jive_shaped_node_is_resource_name_active_before(shaped_node, name)) continue;
				if (jive_shaped_node_is_resource_name_active_after(shaped_node, name)) continue;
				reg = name;
			}
		}
		
		pre_op_transfer(shaped_graph, node, reg);
		inreg0 = reg;
	}
	
	if (inreg0 == outreg0) {
		jive_variable_merge(node->inputs[0]->ssavar->variable, node->outputs[0]->ssavar->variable);
		return;
	}
	
	/* if still necessary, move result to destination
	register after the operation */
	post_op_transfer(shaped_graph, node, inreg0);
	
	jive_variable_merge(node->inputs[0]->ssavar->variable, node->outputs[0]->ssavar->variable);
}

void
jive_regalloc_fixup(jive_shaped_graph * shaped_graph)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(shaped_graph->graph);
	
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		process_node(shaped_graph, node);
	}
	jive_traverser_destroy(traverser);
}

