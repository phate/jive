#include <jive/vsdg/statetype.h>
#include <jive/vsdg/statetype-private.h>

#include <jive/common.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

const jive_state_type jive_state_type_singleton = {
	.base = { .class_ = &JIVE_STATE_TYPE }
};

const jive_type_class JIVE_STATE_TYPE = {
	.parent = &JIVE_TYPE,
	.fini = _jive_state_type_fini, /* override */
	.copy = _jive_state_type_copy, /* override */
	.get_label = _jive_type_get_label, /* inherit */
	.create_input = _jive_state_type_create_input, /* override */
	.create_output = _jive_state_type_create_output, /* override */
	.create_gate = _jive_state_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
};

const jive_input_class JIVE_STATE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_state_input_get_type, /* override */
};

const jive_output_class JIVE_STATE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_state_output_get_type, /* override */
};

const jive_gate_class JIVE_STATE_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = _jive_state_gate_get_type, /* override */
};

void
_jive_state_type_fini(jive_type * self_)
{
	jive_state_type * self = (jive_state_type *) self_;
	_jive_type_fini( &self->base ) ;
}

jive_type *
_jive_state_type_copy(const jive_type * self_, jive_context * context)
{
	const jive_state_type * self = (const jive_state_type *) self_;
	
	jive_state_type * type = jive_context_malloc(context, sizeof(*type));
	
	type->base = self->base;
	
	return &type->base;
}

jive_input *
_jive_state_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_state_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_STATE_INPUT;
	_jive_state_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_state_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_state_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_STATE_OUTPUT;
	_jive_state_output_init(output, node, index);
	return &output->base; 
}

jive_gate *
_jive_state_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_state_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.class_ = &JIVE_STATE_GATE;
	_jive_state_gate_init(gate, graph, name);
	return &gate->base; 
}

void
_jive_state_input_init(jive_state_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_input_init(&self->base, node, index, origin);
}

const jive_type *
_jive_state_input_get_type(const jive_input * self)
{
	return &jive_state_type_singleton.base;
}

void
_jive_state_output_init(jive_state_output * self, struct jive_node * node, size_t index)
{
	_jive_output_init(&self->base, node, index);
}

const jive_type *
_jive_state_output_get_type(const jive_output * self)
{
	return &jive_state_type_singleton.base;
}

void
_jive_state_gate_init(jive_state_gate * self, struct jive_graph * graph, const char * name)
{
	_jive_gate_init(&self->base, graph, name);
}

const jive_type *
_jive_state_gate_get_type(const jive_gate * self)
{
	return &jive_state_type_singleton.base;
}














static void
jive_statemux_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_statemux_node_get_attrs_(const jive_node * self_);

static bool
jive_statemux_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_statemux_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_STATEMUX_NODE = {
	.parent = &JIVE_NODE,
	.name = "STATEMUX",
	.fini = jive_statemux_node_fini_, /* override */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_statemux_node_get_attrs_, /* override */
	.match_attrs = jive_statemux_node_match_attrs_, /* override */
	.create = jive_statemux_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_statemux_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_statemux_node * self = (jive_statemux_node *) self_;
	
	jive_type_fini(self->attrs.type);
	jive_context_free(context, self->attrs.type);
	
	jive_node_fini_(&self->base);
}

static const jive_node_attrs *
jive_statemux_node_get_attrs_(const jive_node * self_)
{
	const jive_statemux_node * self = (const jive_statemux_node *) self_;
	return &self->attrs.base;
}

static bool
jive_statemux_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_statemux_node * self = (const jive_statemux_node *) self_;
	const jive_statemux_node_attrs * attrs = (const jive_statemux_node_attrs *) attrs_;
	return jive_type_equals(self->attrs.type, attrs->type);
}

static jive_node *
jive_statemux_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_statemux_node_attrs * attrs = (const jive_statemux_node_attrs *) attrs_;
	return jive_statemux_node_create(region, attrs->type, noperands, operands, attrs->noutputs);
}

jive_node *
jive_statemux_node_create(jive_region * region,
	const jive_type * statetype,
	size_t noperands, jive_output * const operands[],
	size_t noutputs)
{
	jive_context * context = region->graph->context;
	jive_statemux_node * node = jive_context_malloc(context, sizeof(*node));
	
	node->base.class_ = &JIVE_STATEMUX_NODE;
	JIVE_DEBUG_ASSERT(jive_type_isinstance(statetype, &JIVE_STATE_TYPE));
	
	const jive_type * operand_types[noperands];
	const jive_type * output_types[noutputs];
	size_t n;
	for (n = 0; n < noperands; n++)
		operand_types[n] = statetype;
	for (n = 0; n < noutputs; n++)
		output_types[n] = statetype;
		
	jive_node_init_(&node->base, region,
		noperands, operand_types, operands,
		noutputs, output_types);
	node->attrs.type = jive_type_copy(statetype, context);
	node->attrs.noutputs = noutputs;
	
	return &node->base;
}

jive_output *
jive_state_merge(const jive_type * statetype, size_t nstates, jive_output * const states[])
{
	jive_region * region = jive_region_innermost(nstates, states);
	return jive_statemux_node_create(region, statetype, nstates, states, 1)->outputs[0];
}

jive_node *
jive_state_split(const jive_type * statetype, jive_output * state, size_t nstates)
{
	jive_region * region = state->node->region;
	return jive_statemux_node_create(region, statetype, 1, &state, nstates);
}
