#include <jive/arch/instruction-private.h>

#include <jive/arch/dataobject.h>
#include <jive/bitstring/symbolic-expression.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>
#include <jive/vsdg/sequence.h>

#include <stdio.h>
#include <string.h>

const jive_node_class JIVE_INSTRUCTION_NODE = {
	.parent = &JIVE_NODE,
	.name = "INSTRUCTION",
	.fini = jive_instruction_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_instruction_node_get_label_, /* override */
	.get_attrs = jive_instruction_node_get_attrs_, /* override */
	.match_attrs = jive_instruction_node_match_attrs_, /* override */
	.create = jive_instruction_node_create_, /* override */
	.get_aux_rescls = jive_instruction_node_get_aux_rescls_, /* override */
};

void
jive_instruction_node_init_simple_(
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
	
	jive_node_init_(&self->base,
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
jive_instruction_node_init_(
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
	
	jive_node_init_(&self->base,
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
jive_instruction_node_fini_(jive_node * self_)
{
	jive_instruction_node * self = (jive_instruction_node *) self_;
	
	jive_context * context = self->base.graph->context;
	
	jive_context_free(context, self->attrs.immediates);
	jive_node_fini_(&self->base);
}

char *
jive_instruction_node_get_label_(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	return strdup(self->attrs.icls->name);
}

const jive_node_attrs *
jive_instruction_node_get_attrs_(const jive_node * self_)
{
	const jive_instruction_node * self = (const jive_instruction_node *) self_;
	return &self->attrs.base;
}

bool
jive_instruction_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
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
jive_instruction_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_instruction_node_attrs * attrs = (const jive_instruction_node_attrs *) attrs_;
	
	jive_instruction_node * other = jive_context_malloc(region->graph->context, sizeof(*other));
	other->base.class_ = &JIVE_INSTRUCTION_NODE;
	jive_instruction_node_init_(other, region, attrs->icls, operands, attrs->immediates);
	
	return &other->base;
}

const struct jive_resource_class *
jive_instruction_node_get_aux_rescls_(const jive_node * self_)
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
	jive_instruction_node_init_simple_(node, region, icls, operands, immediates);
	
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
	jive_instruction_node_init_(node, region, icls, operands, immediates);
	
	return &node->base;
}

static void
generate_code_for_instruction(const jive_instruction * instr, jive_buffer * buffer, uint32_t * flags)
{
	const jive_instruction_class * icls = instr->icls;
	
	jive_instruction_encoding_flags iflags;
	iflags = (jive_instruction_encoding_flags) *flags;
	icls->encode(instr->icls, buffer, instr->inputs, instr->outputs, instr->immediates, &iflags);
	*flags = (uint32_t) iflags;
}

static void
generate_code_for_seq_node(jive_seq_node * seq_node, jive_buffer * buffer)
{
	jive_node * node = seq_node->node;
	if (!jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE))
		return;
	
	jive_instruction_node * instr_node = (jive_instruction_node *) node;
	const jive_instruction_class * icls = instr_node->attrs.icls;
	
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
	
	jive_instruction instr;
	instr.icls = icls;
	instr.inputs = inregs;
	instr.outputs = outregs;
	instr.immediates = instr_node->attrs.immediates;
	
	jive_addr addr = buffer->size;
	generate_code_for_instruction(&instr, buffer, &seq_node->flags);
	size_t size = buffer->size - addr;
	
	if (addr != seq_node->base.address || size != seq_node->base.size) {
		seq_node->base.address = addr;
		seq_node->base.size = size;
		seq_node->base.seq_region->seq_graph->addrs_changed = true;
	}
}

static void
generate_code_for_seq_instruction(jive_seq_instruction * seq_instr, jive_buffer * buffer)
{
	jive_addr addr = buffer->size;
	generate_code_for_instruction(&seq_instr->instr, buffer, &seq_instr->flags);
	size_t size = buffer->size - addr;
	
	if (addr != seq_instr->base.address || size != seq_instr->base.size) {
		seq_instr->base.address = addr;
		seq_instr->base.size = size;
		seq_instr->base.seq_region->seq_graph->addrs_changed = true;
	}
}

static void
generate_code(jive_seq_graph * seq_graph, struct jive_buffer * buffer)
{
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		jive_seq_node * seq_node = jive_seq_node_cast(seq_point);
		if (seq_node) {
			generate_code_for_seq_node(seq_node, buffer);
			continue;
		}
		jive_seq_instruction * seq_instr = jive_seq_instruction_cast(seq_point);
		if (seq_instr) {
			generate_code_for_seq_instruction(seq_instr, buffer);
			continue;
		}
	}
}

void
jive_seq_graph_generate_code(jive_seq_graph * seq_graph, jive_buffer * buffer)
{
	size_t size = buffer->size;
	
	/* redo until no labels change anymore; this is actually a bit too
	pessimistic, as we only need to redo if
	- a *forward* reference label may have changed AND
	- the encoding of at least one instruction depends on
	  the value of one of the changed labels */
	while (seq_graph->addrs_changed) {
		jive_buffer_resize(buffer, size);
		seq_graph->addrs_changed = false;
		generate_code(seq_graph, buffer);
	}
}

void
jive_graph_generate_code(jive_graph * graph, struct jive_buffer * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	
	jive_seq_graph_generate_code(seq_graph, buffer);
	
	jive_seq_graph_destroy(seq_graph);
}

static void
emit_region_start_attrs(const jive_region * region, jive_buffer * buffer)
{
	switch (region->attrs.section) {
		default:
		case jive_region_section_inherit:
			break;
		case jive_region_section_code:
			jive_buffer_putstr(buffer, ".section .text\n");
			break;
		case jive_region_section_data:
			jive_buffer_putstr(buffer, ".section .data\n");
			break;
		case jive_region_section_rodata:
			jive_buffer_putstr(buffer, ".section .rodata\n");
			break;
		case jive_region_section_bss:
			jive_buffer_putstr(buffer, ".section .bss\n");
			break;
	};
	if (region->attrs.align > 1) {
		char tmp[80];
		snprintf(tmp, sizeof(tmp), ".align %zd\n", region->attrs.align);
		jive_buffer_putstr(buffer, tmp);
	}
}

static void
emit_labels(jive_seq_point * seq_point, jive_buffer * buffer)
{
	if (seq_point == seq_point->seq_region->first_point)
		emit_region_start_attrs(seq_point->seq_region->region, buffer);
	
	size_t n;
	for(n = 0; n < seq_point->attached_labels.nitems; n++) {
		
		const jive_label_internal * label = seq_point->attached_labels.items[n];
		const char * name = jive_label_internal_get_asmname(label);
		
		if (label->base.flags & jive_label_flags_global) {
			jive_buffer_putstr(buffer, ".globl ");
			jive_buffer_putstr(buffer, name);
			jive_buffer_putstr(buffer, "\n");
		}
		
		jive_buffer_putstr(buffer, name);
		jive_buffer_putstr(buffer,":\n");
	}
}

static void
jive_instruction_generate_assembler(const jive_instruction * instr, jive_buffer * buffer, uint32_t * flags)
{
	const jive_instruction_class * icls = instr->icls;
	jive_buffer_putstr(buffer, "\t");
	if (icls->write_asm) {
		jive_instruction_encoding_flags iflags;
		iflags = (jive_instruction_encoding_flags) *flags;
		icls->write_asm(icls, buffer, instr->inputs, instr->outputs, instr->immediates, &iflags);
		*flags = (uint32_t) iflags;
	} else {
		jive_buffer_putstr(buffer, "; ");
		jive_buffer_put(buffer, icls->name, strlen(icls->name));
	}
	
	jive_buffer_putstr(buffer, "\n");
}

static void
jive_instruction_node_generate_assembler(jive_seq_node * seq_node, jive_node * node, jive_buffer * buffer)
{
	jive_instruction_node * instr_node = (jive_instruction_node *) node;
	const jive_instruction_class * icls = instr_node->attrs.icls;
	
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
	jive_instruction instr;
	instr.icls = icls;
	instr.inputs = inregs;
	instr.outputs = outregs;
	instr.immediates = instr_node->attrs.immediates;
	
	jive_instruction_generate_assembler(&instr, buffer, &seq_node->flags);
}

static void
jive_bitstring_generate_assembler(const jive_bitstring_type * data_type, jive_output * data_item, jive_buffer * buffer)
{
	const char * prefix = NULL;
	switch(data_type->nbits) {
		case 8:
			prefix = "\t.byte ";
			break;
		case 16:
			prefix = "\t.value ";
			break;
		case 32:
			prefix = "\t.long ";
			break;
		case 64:
			prefix = "\t.quad ";
			break;
	}
	if (!prefix)
		return;
	
	jive_context * context = buffer->context;
	
	char * repr = jive_bitstring_symbolic_expression(context, data_item);
	jive_buffer_putstr(buffer, prefix);
	jive_buffer_putstr(buffer, repr);
	jive_buffer_putstr(buffer, "\n");
	jive_context_free(context, repr);
}

static void
jive_dataitem_generate_assembler(jive_output * data_item, jive_buffer * buffer)
{
	const jive_type * data_type = jive_output_get_type(data_item);
	
	if (data_type->class_ == &JIVE_BITSTRING_TYPE)
		jive_bitstring_generate_assembler((const jive_bitstring_type *) data_type, data_item, buffer);
	else
		jive_context_fatal_error(buffer->context, "Type mismatch: don't know how to generate assembler for type");
}

static void
jive_dataitems_node_generate_assembler(jive_seq_node * seq_node, jive_node * node, jive_buffer * buffer)
{
	size_t n;
	for (n = 0; n < node->ninputs; n++)
		jive_dataitem_generate_assembler(node->inputs[n]->origin, buffer);
}

static void
jive_seq_instruction_generate_assembler(jive_seq_instruction * seq_instr, jive_buffer * buffer)
{
	jive_instruction_generate_assembler(&seq_instr->instr, buffer, &seq_instr->flags);
}

static void
jive_seq_node_generate_assembler(jive_seq_node * seq_node, jive_buffer * buffer)
{
	jive_node * node = seq_node->node;
	if (jive_node_isinstance(node, &JIVE_INSTRUCTION_NODE)) {
		jive_instruction_node_generate_assembler(seq_node, node, buffer);
	} else if (jive_node_isinstance(node, &JIVE_DATAITEMS_NODE)) {
		jive_dataitems_node_generate_assembler(seq_node, node, buffer);
	}
}

void
jive_seq_graph_generate_assembler(jive_seq_graph * seq_graph, jive_buffer * buffer)
{
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq_graph->points, seq_point, seqpoint_list) {
		emit_labels(seq_point, buffer);
		jive_seq_node * seq_node = jive_seq_node_cast(seq_point);
		if (seq_node) {
			jive_seq_node_generate_assembler(seq_node, buffer);
			continue;
		}
		jive_seq_instruction * seq_instr = jive_seq_instruction_cast(seq_point);
		if (seq_instr) {
			jive_seq_instruction_generate_assembler(seq_instr, buffer);
			continue;
		}
	}
}

void
jive_graph_generate_assembler(jive_graph * graph, jive_buffer * buffer)
{
	jive_seq_graph * seq_graph = jive_graph_sequentialize(graph);
	
	jive_seq_graph_generate_assembler(seq_graph, buffer);
	
	jive_seq_graph_destroy(seq_graph);
}

static void
jive_encode_PSEUDO_NOP(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_register_name * inputs[],
	const jive_register_name * outputs[],
	const jive_immediate immediates[],
	jive_instruction_encoding_flags * flags)
{
}

const jive_instruction_class JIVE_PSEUDO_NOP = {
	.name = "PSEUDO_NOP",
	.mnemonic = "",
	.encode = jive_encode_PSEUDO_NOP,
	.write_asm = jive_encode_PSEUDO_NOP,
	.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none, .ninputs = 0, .noutputs = 0, .nimmediates = 0
};

static void
jive_seq_instruction_fini_(jive_seq_point * self_)
{
	jive_seq_instruction * self = (jive_seq_instruction *) self_;
	jive_context * context = self->base.seq_region->seq_graph->context;
	jive_context_free(context, self->instr.inputs);
	jive_context_free(context, self->instr.outputs);
	jive_context_free(context, self->instr.immediates);
	jive_seq_point_fini_(&self->base);
}

const jive_seq_point_class JIVE_SEQ_INSTRUCTION = {
	.parent = &JIVE_SEQ_POINT,
	.fini = jive_seq_instruction_fini_
};

jive_seq_instruction *
jive_seq_instruction_create(
	jive_seq_point * before,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_immediate immediates[])
{
	jive_seq_region * seq_region = before->seq_region;
	jive_seq_graph * seq = seq_region->seq_graph;
	jive_context * context = seq->context;
	
	jive_seq_instruction * seq_instr = jive_context_malloc(context, sizeof(*seq_instr));
	jive_seq_point_init(&seq_instr->base, seq_region);
	seq_instr->instr.icls = icls;
	seq_instr->instr.inputs = jive_context_malloc(context, icls->ninputs * sizeof(seq_instr->instr.inputs[0]));
	seq_instr->instr.outputs = jive_context_malloc(context, icls->noutputs * sizeof(seq_instr->instr.outputs[0]));
	seq_instr->instr.immediates = jive_context_malloc(context, icls->nimmediates * sizeof(seq_instr->instr.immediates[0]));
	seq_instr->flags = 0;
	
	size_t n;
	for (n = 0; n < icls->ninputs; n++)
		seq_instr->instr.inputs[n] = inputs[n];
	for (n = 0; n < icls->noutputs; n++)
		seq_instr->instr.outputs[n] = outputs[n];
	for (n = 0; n < icls->nimmediates; n++)
		seq_instr->instr.immediates[n] = immediates[n];
	
	JIVE_LIST_INSERT(seq->points, before, &seq_instr->base, seqpoint_list);
	if (before->seq_region->first_point == before)
		before->seq_region->first_point = &seq_instr->base;
	
	return seq_instr;
}

