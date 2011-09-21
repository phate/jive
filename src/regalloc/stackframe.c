#include <jive/regalloc/stackframe.h>

#include <jive/arch/instruction.h>
#include <jive/arch/memory.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/util/hash.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

static jive_subroutine *
lookup_subroutine_by_node(jive_node * node)
{
	while (node) {
		jive_subroutine_node * sub = jive_subroutine_node_cast(node);
		if (sub)
			return sub->attrs.subroutine;
		if (node->region->anchor)
			node = node->region->anchor->node;
		else
			node = NULL;
	}
	return NULL;
}

const jive_stackslot *
get_req_stackslot(jive_variable * variable)
{
	const jive_resource_class * rescls = variable->rescls;
	if (!jive_resource_class_issubclass(rescls, &jive_root_stackslot_class))
		return NULL;
	
	const jive_resource_name * name = jive_variable_get_resource_name(variable);
	if (!name && rescls->limit)
		name = rescls->names[0];
	
	return (const jive_stackslot *) name;
}

static void
layout_stackslot(jive_shaped_graph * shaped_graph, jive_subroutine * subroutine, jive_variable * variable)
{
	if (variable->resname)
		return;
	
	/* unless this is a stackslot class, bail out */
	const jive_resource_class * rescls = variable->rescls;
	if (!jive_resource_class_issubclass(rescls, &jive_root_stackslot_class))
		return;
	const jive_stackslot_size_class * cls = (const jive_stackslot_size_class *) rescls;
	
	if (rescls->limit) {
		const jive_resource_name * name = rescls->names[0];
		jive_variable_set_resource_name(variable, name);
	}
	
	ssize_t lower_bound = subroutine->frame.lower_bound & ~(cls->alignment - 1);
	size_t nslots = subroutine->frame.upper_bound - lower_bound;
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
		const jive_stackslot_size_class * other_cls = (const jive_stackslot_size_class *) other_slot->base.resource_class;
		
		for (n = 0; n < other_cls->size; n++)
			allowed_slot[n + other_slot->offset - lower_bound] = false;
	}
	
	ssize_t offset = subroutine->frame.upper_bound - cls->size;
	offset = offset & ~(cls->alignment - 1);
	
	while (offset >= lower_bound) {
		for (n = 0; n < cls->size; n++) {
			if (!allowed_slot[offset + n])
				break;
		}
		if (n == cls->size)
			break;
		offset -= cls->alignment;
	}
	
	const jive_resource_name * name = jive_stackslot_name_get(cls->size, cls->alignment, offset);
	jive_variable_set_resource_name(variable, name);
	
	if (offset < subroutine->frame.lower_bound)
		subroutine->frame.lower_bound = offset;
}

const jive_stackslot *
get_node_stackslot(jive_node * node)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(input->ssavar->variable);
		if (jive_resource_class_issubclass(name->resource_class, &jive_root_stackslot_class))
			return (const jive_stackslot *) name;
	}
	for (n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(output->ssavar->variable);
		if (jive_resource_class_issubclass(name->resource_class, &jive_root_stackslot_class))
			return (const jive_stackslot *) name;
	}
	return NULL;
}

void
jive_regalloc_stackframe(jive_shaped_graph * shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	
	jive_traverser * trav = jive_bottomup_traverser_create(graph);
	
	jive_node * node;
	for ( node = jive_traverser_next(trav); node; node = jive_traverser_next(trav) ) {
		jive_subroutine * subroutine = lookup_subroutine_by_node(node);
		if (!subroutine)
			continue;
		size_t n;
		for (n = 0; n < node->noutputs; n++) {
			jive_output * output = node->outputs[n];
			layout_stackslot(shaped_graph, subroutine, output->ssavar->variable);
		}
	}
	
	jive_traverser_destroy(trav);
}

static void
reloc_stack_access(jive_node * node)
{
	const jive_instruction_node * inode = jive_instruction_node_cast(node);
	if (!inode)
		return;
	
	size_t n;
	for (n = 0; n < inode->attrs.icls->nimmediates; n++) {
		jive_immediate imm = inode->attrs.immediates[n];
		if (imm.add_label == &jive_label_fpoffset) {
			const jive_stackslot * slot = get_node_stackslot(node);
			const jive_subroutine * subroutine = lookup_subroutine_by_node(node);
			imm = jive_immediate_add_offset(&imm, slot->offset - subroutine->frame.frame_pointer_offset);
			imm.add_label = 0;
		}
		if (imm.sub_label == &jive_label_fpoffset) {
			const jive_stackslot * slot = get_node_stackslot(node);
			const jive_subroutine * subroutine = lookup_subroutine_by_node(node);
			imm = jive_immediate_add_offset(&imm, -slot->offset - subroutine->frame.frame_pointer_offset);
			imm.add_label = 0;
		}
		inode->attrs.immediates[n] = imm;
	}
}

void
jive_regalloc_relocate_stackslots(struct jive_shaped_graph * shaped_graph)
{
	jive_graph * graph = shaped_graph->graph;
	
	jive_traverser * trav = jive_bottomup_traverser_create(graph);
	
	jive_node * node;
	for ( node = jive_traverser_next(trav); node; node = jive_traverser_next(trav) ) {
		reloc_stack_access(node);
	}
	jive_traverser_destroy(trav);
}

