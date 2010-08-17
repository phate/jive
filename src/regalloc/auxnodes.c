#include <jive/regalloc/auxnodes.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/cut-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/traverser.h>
#include <jive/arch/registers.h>
#include <jive/arch/transfer-instructions.h>

#include <string.h>

static char *
_jive_aux_valuecopy_node_get_label(const jive_node * self)
{
	return strdup("PSEUDO_COPY");
}

static void
_jive_aux_valuecopy_node_init(jive_aux_valuecopy_node * self, jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	const jive_type * type = jive_regcls_get_type(regcls);
	self->base.class_ = &JIVE_AUX_VALUECOPY_NODE;
	_jive_node_init(&self->base, region,
		1, &type, &origin,
		1, &type);
	
	self->regcls = regcls;
	((jive_value_input *)self->base.inputs[0])->required_regcls = self->regcls;
	((jive_value_output *)self->base.outputs[0])->required_regcls = self->regcls;
}

static jive_node *
_jive_aux_valuecopy_node_copy(const jive_node * self_,
	struct jive_region * region,
	struct jive_output * operands[])
{
	const jive_aux_valuecopy_node * self = (const jive_aux_valuecopy_node *) self_;
	
	return jive_aux_valuecopy_node_create(region, self->regcls, operands[0]);
}

static bool
_jive_aux_valuecopy_node_equiv(const jive_node * self_, const jive_node * other_)
{
	const jive_aux_valuecopy_node * self = (const jive_aux_valuecopy_node *) self_;
	const jive_aux_valuecopy_node * other = (const jive_aux_valuecopy_node *) other_;
	return self->regcls == other->regcls;
}

const jive_node_class JIVE_AUX_VALUECOPY_NODE = {
	.parent = 0,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_aux_valuecopy_node_get_label, /* override */
	.copy = _jive_aux_valuecopy_node_copy, /* override */
	.equiv = _jive_aux_valuecopy_node_equiv, /* override */
	.get_aux_regcls = _jive_node_get_aux_regcls /* inherit */
};

jive_node *
jive_aux_valuecopy_node_create(jive_region * region, const jive_regcls * regcls, jive_output * origin)
{
	/* always generalize register class */
	while(regcls->parent) regcls = regcls->parent;
	jive_aux_valuecopy_node * self = jive_context_malloc(region->graph->context, sizeof(*self));
	_jive_aux_valuecopy_node_init(self, region, regcls, origin);
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
replace_pseudo_nodes(jive_graph * graph, const jive_transfer_instructions_factory * gen)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(graph);
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		if (jive_node_isinstance(node, &JIVE_AUX_VALUECOPY_NODE)) replace_pseudo_copy_node(node, gen);
	}
	jive_traverser_destroy(traverser);
}

void
jive_regalloc_auxnodes_replace(jive_graph * graph, const jive_transfer_instructions_factory * gen)
{
	replace_pseudo_nodes(graph, gen);
}
