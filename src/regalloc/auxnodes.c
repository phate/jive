#include <jive/regalloc/auxnodes.h>

#include <jive/common.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine.h>
#include <jive/arch/transfer-instructions.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/statetype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>

#include <string.h>


static void
jive_aux_split_node_init_(
	jive_aux_split_node * self,
	jive_region * region,
	const jive_type * in_type,
	jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive_type * out_type,
	const struct jive_resource_class * out_class)
{
	self->base.class_ = &JIVE_AUX_SPLIT_NODE;
	jive_node_init_(&self->base, region,
		1, &in_type, &in_origin,
		1, &out_type);
	
	self->attrs.in_class = in_class;
	self->attrs.out_class = out_class;
	self->base.inputs[0]->required_rescls = in_class;
	self->base.outputs[0]->required_rescls = out_class;
}

static jive_node *
jive_aux_split_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive_aux_split_node_attrs * attrs = (const jive_aux_split_node_attrs *) attrs_;
	jive_node * node = jive_aux_split_node_create(region,
		jive_resource_class_get_type(attrs->in_class),
		operands[0],
		attrs->in_class,
		jive_resource_class_get_type(attrs->out_class),
		attrs->out_class);
	
	node->inputs[0]->required_rescls = attrs->in_class;
	node->outputs[0]->required_rescls = attrs->out_class;
	
	return node;
}

static const jive_node_attrs *
jive_aux_split_node_get_attrs_(const jive_node * self_)
{
	const jive_aux_split_node * self = (const jive_aux_split_node *) self_;
	return &self->attrs.base;
}

static bool
jive_aux_split_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_aux_split_node * self = (const jive_aux_split_node *) self_;
	const jive_aux_split_node_attrs * attrs = (const jive_aux_split_node_attrs *) attrs_;
	return (self->attrs.in_class == attrs->in_class) && (self->attrs.out_class == attrs->out_class);
}

const jive_node_class JIVE_AUX_SPLIT_NODE = {
	.parent = &JIVE_NODE,
	.name = "AUX_SPLIT",
	.fini = jive_node_fini_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_aux_split_node_get_attrs_, /* override */
	.match_attrs = jive_aux_split_node_match_attrs_, /* override */
	.create = jive_aux_split_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

jive_node *
jive_aux_split_node_create(jive_region * region,
	const jive_type * in_type,
	jive_output * in_origin,
	const struct jive_resource_class * in_class,
	const jive_type * out_type,
	const struct jive_resource_class * out_class)
{
	jive_aux_split_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	jive_aux_split_node_init_(self, region, in_type, in_origin, in_class, out_type, out_class);
	
	return &self->base;
}

static void
replace_aux_split_node(jive_shaped_node * shaped_node, jive_node * node, const jive_transfer_instructions_factory * gen)
{
	jive_xfer_block xfer = gen->create_xfer(node->region, node->inputs[0]->origin,
		node->inputs[0]->required_rescls, node->outputs[0]->required_rescls);
	if (!xfer.input)
		xfer.input = jive_node_add_input(xfer.node, jive_input_get_type(node->inputs[0]), node->inputs[0]->origin);
	if (!xfer.output)
		xfer.output = jive_node_add_output(xfer.node, jive_output_get_type(node->outputs[0]));
	
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

static void
check_fp_sp_dependency(jive_node * node)
{
	const jive_subroutine * subroutine = lookup_subroutine_by_node(node);
	if (!subroutine)
		return;
	
	if (node == &subroutine->enter->base || node == &subroutine->leave->base)
		return;
	
	bool need_fp_dependency = false;
	bool need_sp_dependency = false;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(input->ssavar->variable);
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_CALLSLOT_RESOURCE))
			need_sp_dependency = true;
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_FRAMESLOT_RESOURCE))
			need_sp_dependency = true;
	}
	for (n = 0; n < node->noutputs; n++) {
		jive_input * output = node->inputs[n];
		const jive_resource_name * name = jive_variable_get_resource_name(output->ssavar->variable);
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_CALLSLOT_RESOURCE))
			need_sp_dependency = true;
		if (name && jive_resource_class_isinstance(name->resource_class, &JIVE_STACK_FRAMESLOT_RESOURCE))
			need_sp_dependency = true;
	}
	
	if (need_fp_dependency) {
		jive_input * input = jive_subroutine_add_fp_dependency(subroutine, node);
		if (input)
			jive_input_auto_merge_variable(input);
	}
	if (need_sp_dependency) {
		jive_input * input = jive_subroutine_add_sp_dependency(subroutine, node);
		if (input)
			jive_input_auto_merge_variable(input);
	}
}

void
jive_regalloc_auxnodes_replace(jive_shaped_graph * shaped_graph, const jive_transfer_instructions_factory * gen)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(shaped_graph->graph);
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		if (jive_node_isinstance(node, &JIVE_AUX_SPLIT_NODE)) {
			jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, node);
			replace_aux_split_node(shaped_node, node, gen);
		} else
			check_fp_sp_dependency(node);
	}
	jive_traverser_destroy(traverser);
}
