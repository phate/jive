#include <jive/arch/instruction-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/traverser.h>

#include <string.h>

const jive_node_class JIVE_INSTRUCTION_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_instruction_node_fini, /* override */
	.get_label = _jive_instruction_node_get_label, /* override */
	.get_attrs = _jive_instruction_node_get_attrs, /* override */
	.create = _jive_instruction_node_create, /* override */
	.equiv = _jive_instruction_node_equiv, /* override */
	.can_reduce = _jive_node_can_reduce, /* inherit */
	.reduce = _jive_node_reduce, /* inherit */
	.get_aux_regcls = _jive_instruction_node_get_aux_regcls, /* override */
};

void
_jive_instruction_node_init(
	jive_instruction_node * self,
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const long immediates[const])
{
	const jive_type * input_types[icls->ninputs];
	const jive_type * output_types[icls->noutputs];
	
	size_t n;
	for(n=0; n<icls->ninputs; n++) input_types[n] = jive_regcls_get_type(icls->inregs[n]);
	for(n=0; n<icls->noutputs; n++) output_types[n] = jive_regcls_get_type(icls->outregs[n]);
	
	_jive_node_init(&self->base,
		region,
		icls->ninputs, input_types, operands,
		icls->noutputs, output_types);
	
	for(n=0; n<icls->ninputs; n++)
		((jive_value_input *)self->base.inputs[n])->required_regcls = icls->inregs[n];
	for(n=0; n<icls->noutputs; n++)
		((jive_value_output *)self->base.outputs[n])->required_regcls = icls->outregs[n];
	
	self->attrs.icls = icls;
	self->attrs.immediates = jive_context_malloc(self->base.graph->context, sizeof(long) * icls->nimmediates);
	for(n=0; n<icls->nimmediates; n++) self->attrs.immediates[n] = immediates[n];
}

void
_jive_instruction_node_fini(jive_node * self_)
{
	jive_instruction_node * self = (jive_instruction_node *) self_;
	
	jive_context_free(self->base.graph->context, self->attrs.immediates);
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

jive_node *
_jive_instruction_node_create(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * operands[])
{
	const jive_instruction_node_attrs * attrs = (const jive_instruction_node_attrs *) attrs_;
	
	jive_instruction_node * other = jive_context_malloc(region->graph->context, sizeof(*other));
	other->base.class_ = &JIVE_INSTRUCTION_NODE;
	_jive_instruction_node_init(other, region, attrs->icls, operands, attrs->immediates);
	
	return &other->base;
}

bool
_jive_instruction_node_equiv(const jive_node_attrs * first_, const jive_node_attrs * second_)
{
	const jive_instruction_node_attrs * first = (const jive_instruction_node_attrs *) first_;
	const jive_instruction_node_attrs * second = (const jive_instruction_node_attrs *) second_;
	
	if (first->icls != second->icls) return false;
	size_t n;
	for(n=0; n<first->icls->nimmediates; n++)
		if (first->immediates[n] != second->immediates[n]) return false;
	
	return true;
}

const struct jive_regcls *
_jive_instruction_node_get_aux_regcls(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	
	if (self->attrs.icls->flags & jive_instruction_commutative) return 0;
	if (!(self->attrs.icls->flags & jive_instruction_write_input)) return 0;
	return self->attrs.icls->inregs[0];
}

jive_instruction_node *
jive_instruction_node_create(
	jive_region * region,
	const jive_instruction_class * icls,
	jive_output * operands[const],
	const long immediates[const])
{
	jive_instruction_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_INSTRUCTION_NODE;
	_jive_instruction_node_init(node, region, icls, operands, immediates);
	
	return node;
}

void
jive_graph_generate_code(jive_graph * graph, struct jive_buffer * buffer)
{
	jive_traverser * trav = jive_topdown_traverser_create(graph);
	
	jive_node * node;
	while( (node = jive_traverser_next(trav)) != 0) {
		if (!jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) continue;
		
		jive_instruction_node * instr = (jive_instruction_node *) node;
		const jive_instruction_class * icls = instr->attrs.icls;
		
		const jive_cpureg * inregs[icls->ninputs];
		const jive_cpureg * outregs[icls->noutputs];
		size_t n;
		for(n=0; n<icls->ninputs; n++) {
			jive_value_resource * resource = (jive_value_resource *) node->inputs[n]->resource;
			inregs[n] = resource->cpureg;
		}
		for(n=0; n<icls->noutputs; n++) {
			jive_value_resource * resource = (jive_value_resource *) node->outputs[n]->resource;
			outregs[n] = resource->cpureg;
		}
		
		icls->encode(icls, buffer, inregs, outregs, instr->attrs.immediates);
	}
	
	jive_traverser_destroy(trav);
}

static void
jive_encode_PSEUDO_NOP(const jive_instruction_class * icls,
        jive_buffer * target,
        const jive_cpureg * inputs[],
        const jive_cpureg * outputs[],
        const long immediates[])
{
}

const jive_instruction_class JIVE_PSEUDO_NOP = {
	.name = "PSEUDO_NOP",
	.encode = jive_encode_PSEUDO_NOP,
	.mnemonic = jive_encode_PSEUDO_NOP,
	.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0
};
