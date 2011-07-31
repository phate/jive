#include <jive/regalloc/auxnodes.h>

#include <jive/common.h>
#include <jive/arch/registers.h>
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
	_jive_node_init(&self->base, region,
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

const jive_node_class JIVE_AUX_SPLIT_NODE = {
	.parent = &JIVE_NODE,
	.name = "AUX_SPLIT",
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.match_attrs = _jive_node_match_attrs, /* inherit */
	.create = jive_aux_split_node_create_, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
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

void
jive_regalloc_auxnodes_replace(jive_shaped_graph * shaped_graph, const jive_transfer_instructions_factory * gen)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(shaped_graph->graph);
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		if (jive_node_isinstance(node, &JIVE_AUX_SPLIT_NODE)) {
			jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, node);
			replace_aux_split_node(shaped_node, node, gen);
		}
	}
	jive_traverser_destroy(traverser);
}

#if 0




jive_node *
jive_aux_valuecopy_node_create(jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	/* always generalize register class */
	while(regcls->parent) regcls = regcls->parent;
	jive_aux_valuecopy_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	_jive_aux_valuecopy_node_init(self, region, regcls, origin);
	return &self->base;
}

/* spill */

static char *
_jive_aux_spill_node_get_label(const jive_node * self)
{
	return strdup("PSEUDO_SPILL");
}

static void
_jive_aux_spill_node_init(jive_aux_spill_node * self, jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	const jive_type * input_type = jive_regcls_get_type(regcls);
	JIVE_DECLARE_STACKSLOT_TYPE(output_type, regcls);
	self->base.class_ = &JIVE_AUX_SPILL_NODE;
	_jive_node_init(&self->base, region,
		1, &input_type, &origin,
		1, &output_type);
	
	self->attrs.regcls = regcls;
	((jive_value_input *)self->base.inputs[0])->required_regcls = self->attrs.regcls;
}

const jive_node_attrs *
_jive_aux_spill_node_get_attrs(const jive_node * self_)
{
	const jive_aux_spill_node * self = (const jive_aux_spill_node *) self_;
	return &self->attrs.base;
}

static jive_node *
_jive_aux_spill_node_create(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * operands[])
{
	const jive_aux_node_attrs * attrs = (const jive_aux_node_attrs *) attrs_;
	return jive_aux_spill_node_create(region, attrs->regcls, operands[0]);
}

static bool
_jive_aux_spill_node_equiv(const jive_node_attrs * first_, const jive_node_attrs * second_)
{
	const jive_aux_node_attrs * first = (const jive_aux_node_attrs *) first_;
	const jive_aux_node_attrs * second = (const jive_aux_node_attrs *) second_;
	return first->regcls == second->regcls;
}

const jive_node_class JIVE_AUX_SPILL_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_aux_spill_node_get_label, /* override */
	.get_attrs = _jive_aux_spill_node_get_attrs, /* override */
	.create = _jive_aux_spill_node_create, /* override */
	.equiv = _jive_aux_spill_node_equiv, /* override */
	.can_reduce = _jive_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

jive_node *
jive_aux_spill_node_create(jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	/* always generalize register class */
	while(regcls->parent) regcls = regcls->parent;
	jive_aux_spill_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	_jive_aux_spill_node_init(self, region, regcls, origin);
	return &self->base;
}

/* restore */

static char *
_jive_aux_restore_node_get_label(const jive_node * self)
{
	return strdup("PSEUDO_RESTORE");
}

static void
_jive_aux_restore_node_init(jive_aux_restore_node * self, jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	const jive_type * output_type = jive_regcls_get_type(regcls);
	JIVE_DECLARE_STACKSLOT_TYPE(input_type, regcls);
	self->base.class_ = &JIVE_AUX_RESTORE_NODE;
	_jive_node_init(&self->base, region,
		1, &input_type, &origin,
		1, &output_type);
	
	self->attrs.regcls = regcls;
	((jive_value_output *)self->base.outputs[0])->required_regcls = self->attrs.regcls;
}

const jive_node_attrs *
_jive_aux_restore_node_get_attrs(const jive_node * self_)
{
	const jive_aux_restore_node * self = (const jive_aux_restore_node *) self_;
	return &self->attrs.base;
}

static jive_node *
_jive_aux_restore_node_create(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * operands[])
{
	const jive_aux_node_attrs * attrs = (const jive_aux_node_attrs *) attrs_;
	return jive_aux_restore_node_create(region, attrs->regcls, operands[0]);
}

static bool
_jive_aux_restore_node_equiv(const jive_node_attrs * first_, const jive_node_attrs * second_)
{
	const jive_aux_node_attrs * first = (const jive_aux_node_attrs *) first_;
	const jive_aux_node_attrs * second = (const jive_aux_node_attrs *) second_;
	return first->regcls == second->regcls;
}

const jive_node_class JIVE_AUX_RESTORE_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_aux_restore_node_get_label, /* override */
	.get_attrs = _jive_aux_restore_node_get_attrs, /* override */
	.create = _jive_aux_restore_node_create, /* override */
	.equiv = _jive_aux_restore_node_equiv, /* override */
	.can_reduce = _jive_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

jive_node *
jive_aux_restore_node_create(jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	/* always generalize register class */
	while(regcls->parent) regcls = regcls->parent;
	jive_aux_restore_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	_jive_aux_restore_node_init(self, region, regcls, origin);
	return &self->base;
}





static void
replace_pseudo_copy_node(jive_node * node, const jive_transfer_instructions_factory * gen)
{
	jive_input * xfer_in;
	jive_output * xfer_out;
	jive_node * xfer_nodes[gen->max_nodes];
	size_t nxfer_nodes = gen->create_copy(node->region, node->inputs[0]->origin,
		&xfer_in, xfer_nodes, &xfer_out);
	
	jive_resource_assign_input(node->inputs[0]->resource, xfer_in);
	jive_resource_assign_output(node->outputs[0]->resource, xfer_out);
	
	jive_cut * cut = node->shape_location->cut;
	jive_node_location * pos = node->shape_location->cut_nodes_list.next;
	jive_node_location_destroy(node->shape_location);
	
	size_t n;
	for(n=0; n<nxfer_nodes; n++) jive_cut_insert(cut, pos, xfer_nodes[n]);
	
	jive_output_replace(node->outputs[0], xfer_out);
	jive_node_destroy(node);
}

static void
replace_pseudo_spill_node(jive_node * node, const jive_transfer_instructions_factory * gen)
{
	jive_input * spill_input;
	jive_node * store_node;
	jive_node * nodes[gen->max_nodes];
	size_t nnodes = gen->create_spill(node->region, node->inputs[0]->origin,
		&spill_input, nodes, &store_node);
	
	jive_output * spill_output = jive_node_add_output(store_node, jive_output_get_type(node->outputs[0]));
	
	jive_resource_assign_input(node->inputs[0]->resource, spill_input);
	jive_resource_assign_output(node->outputs[0]->resource, spill_output);
	
	jive_cut * cut = node->shape_location->cut;
	jive_node_location * pos = node->shape_location->cut_nodes_list.next;
	jive_node_location_destroy(node->shape_location);
	
	size_t n;
	for(n=0; n<nnodes; n++) jive_cut_insert(cut, pos, nodes[n]);
	
	jive_output_replace(node->outputs[0], spill_output);
	jive_node_destroy(node);
}

static void
replace_pseudo_restore_node(jive_node * node, const jive_transfer_instructions_factory * gen)
{
	jive_node * load_node;
	jive_output * restore_output;
	jive_node * nodes[gen->max_nodes];
	ssize_t nnodes = gen->create_restore(node->region, node->inputs[0]->origin,
		&load_node, nodes, &restore_output);
	
	jive_input * restore_input = jive_node_add_input(load_node,
		jive_input_get_type(node->inputs[0]), node->inputs[0]->origin);
	
	jive_resource_assign_input(node->inputs[0]->resource, restore_input);
	
	jive_resource_assign_output(node->outputs[0]->resource, restore_output);
	
	jive_cut * cut = node->shape_location->cut;
	jive_node_location * pos = node->shape_location->cut_nodes_list.next;
	jive_node_location_destroy(node->shape_location);
	
	size_t n;
	for(n=0; n<nnodes; n++) jive_cut_insert(cut, pos, nodes[n]);
	
	jive_output_replace(node->outputs[0], restore_output);
	jive_node_destroy(node);
}

static void
replace_pseudo_nodes(jive_graph * graph, const jive_transfer_instructions_factory * gen)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(graph);
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		if (jive_node_isinstance(node, &JIVE_AUX_VALUECOPY_NODE)) replace_pseudo_copy_node(node, gen);
		else if (jive_node_isinstance(node, &JIVE_AUX_SPILL_NODE)) replace_pseudo_spill_node(node, gen);
		else if (jive_node_isinstance(node, &JIVE_AUX_RESTORE_NODE)) replace_pseudo_restore_node(node, gen);
	}
	jive_traverser_destroy(traverser);
}

void
jive_regalloc_auxnodes_replace(jive_graph * graph, const jive_transfer_instructions_factory * gen)
{
	replace_pseudo_nodes(graph, gen);
}

#endif
