#include <jive/arch/instruction-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>
#include <jive/vsdg/sequence.h>

#include <string.h>

const jive_node_class JIVE_INSTRUCTION_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_instruction_node_fini, /* override */
	.get_label = _jive_instruction_node_get_label, /* override */
	.get_attrs = _jive_instruction_node_get_attrs, /* override */
	.match_attrs = _jive_instruction_node_match_attrs, /* override */
	.create = _jive_instruction_node_create, /* override */
	.get_aux_rescls = _jive_instruction_node_get_aux_rescls, /* override */
};

void
_jive_instruction_node_init_simple(
	jive_instruction_node * self,
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * const operands[],
	const long immediates[])
{
	jive_context * context = region->graph->context;
	const jive_type * input_types[icls->ninputs];
	const jive_type * output_types[icls->noutputs];
	
	size_t n;
	for(n=0; n<icls->ninputs; n++) input_types[n] = jive_register_class_get_type(icls->inregs[n]);
	for(n=0; n<icls->noutputs; n++) output_types[n] = jive_register_class_get_type(icls->outregs[n]);
	
	_jive_node_init(&self->base,
		region,
		icls->ninputs, input_types, operands,
		icls->noutputs, output_types);
	
	for(n=0; n<icls->ninputs; n++)
		self->base.inputs[n]->required_rescls = &icls->inregs[n]->base;
	for(n=0; n<icls->noutputs; n++)
		self->base.outputs[n]->required_rescls = &icls->outregs[n]->base;
	
	self->attrs.icls = icls;
	self->attrs.immediates = jive_context_malloc(context, sizeof(self->attrs.immediates[0]) * icls->nimmediates);
	for(n=0; n<icls->nimmediates; n++)
		jive_immediate_init(&self->attrs.immediates[n], immediates[n], NULL, NULL, NULL);
}

void
_jive_instruction_node_init(
	jive_instruction_node * self,
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * const operands[],
	const jive_immediate immediates[])
{
	jive_context * context = region->graph->context;
	const jive_type * input_types[icls->ninputs];
	const jive_type * output_types[icls->noutputs];
	
	size_t n;
	for(n=0; n<icls->ninputs; n++) input_types[n] = jive_register_class_get_type(icls->inregs[n]);
	for(n=0; n<icls->noutputs; n++) output_types[n] = jive_register_class_get_type(icls->outregs[n]);
	
	_jive_node_init(&self->base,
		region,
		icls->ninputs, input_types, operands,
		icls->noutputs, output_types);
	
	for(n=0; n<icls->ninputs; n++)
		self->base.inputs[n]->required_rescls = &icls->inregs[n]->base;
	for(n=0; n<icls->noutputs; n++)
		self->base.outputs[n]->required_rescls = &icls->outregs[n]->base;
	
	self->attrs.icls = icls;
	self->attrs.immediates = jive_context_malloc(context, sizeof(self->attrs.immediates[0]) * icls->nimmediates);
	for(n=0; n<icls->nimmediates; n++)
		self->attrs.immediates[n] = immediates[n];
}

void
_jive_instruction_node_fini(jive_node * self_)
{
	jive_instruction_node * self = (jive_instruction_node *) self_;
	
	jive_context * context = self->base.graph->context;
	
	jive_context_free(context, self->attrs.immediates);
	_jive_node_fini(&self->base);
}

char *
_jive_instruction_node_get_label(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	return strdup(self->attrs.icls->name);
}

const jive_node_attrs *
_jive_instruction_node_get_attrs(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	return &self->attrs.base;
}

bool
_jive_instruction_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_instruction_node_attrs * first = &((const jive_instruction_node *) self)->attrs;
	const jive_instruction_node_attrs * second = (const jive_instruction_node_attrs *) attrs;
	
	if (first->icls != second->icls) return false;
	size_t n;
	for(n=0; n<first->icls->nimmediates; n++)
		if (!jive_immediate_equals(&first->immediates[n], &second->immediates[n])) return false;
	
	return true;
}


jive_node *
_jive_instruction_node_create(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_instruction_node_attrs * attrs = (const jive_instruction_node_attrs *) attrs_;
	
	jive_instruction_node * other = jive_context_malloc(region->graph->context, sizeof(*other));
	other->base.class_ = &JIVE_INSTRUCTION_NODE;
	_jive_instruction_node_init(other, region, attrs->icls, operands, attrs->immediates);
	
	return &other->base;
}

const struct jive_resource_class *
_jive_instruction_node_get_aux_rescls(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	
	if (self->attrs.icls->flags & jive_instruction_commutative) return 0;
	if (!(self->attrs.icls->flags & jive_instruction_write_input)) return 0;
	return &self->attrs.icls->inregs[0]->base;
}

jive_node *
jive_instruction_node_create_simple(
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const long immediates[const])
{
	jive_instruction_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_INSTRUCTION_NODE;
	_jive_instruction_node_init_simple(node, region, icls, operands, immediates);
	
	return &node->base;
}

jive_node *
jive_instruction_node_create_extended(
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const jive_immediate immediates[])
{
	jive_instruction_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_INSTRUCTION_NODE;
	_jive_instruction_node_init(node, region, icls, operands, immediates);
	
	return &node->base;
}

void
jive_graph_generate_code(jive_graph * graph, struct jive_buffer * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	jive_seq_node * seq_node;
	
	JIVE_LIST_ITERATE(seq_graph->nodes, seq_node, seqnode_list) {
		jive_node * node = seq_node->node;
		if (!jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) continue;
		
		jive_instruction_node * instr = (jive_instruction_node *) node;
		const jive_instruction_class * icls = instr->attrs.icls;
		
		const jive_register_name * inregs[icls->ninputs];
		const jive_register_name * outregs[icls->noutputs];
		size_t n;
		for(n=0; n<icls->ninputs; n++) {
			const jive_resource_name * resname = jive_variable_get_resource_name(node->inputs[n]->ssavar->variable);
			inregs[n] = (const jive_register_name *)resname;
		}
		for(n=0; n<icls->noutputs; n++) {
			const jive_resource_name * resname = jive_variable_get_resource_name(node->outputs[n]->ssavar->variable);
			outregs[n] = (const jive_register_name *)resname;
		}
		
		icls->encode(icls, buffer, inregs, outregs, instr->attrs.immediates);
	}
	
	jive_seq_graph_destroy(seq_graph);
}

static void
jive_encode_PSEUDO_NOP(const jive_instruction_class * icls,
        jive_buffer * target,
        const jive_register_name * inputs[],
        const jive_register_name * outputs[],
        const jive_immediate immediates[])
{
}

const jive_instruction_class JIVE_PSEUDO_NOP = {
	.name = "PSEUDO_NOP",
	.encode = jive_encode_PSEUDO_NOP,
	.mnemonic = jive_encode_PSEUDO_NOP,
	.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0
};
