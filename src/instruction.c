#include <string.h>

#include <jive/internal/instruction_str.h>
#include <jive/internal/subroutinestr.h>
#include <jive/machine.h>
#include <jive/nodeclass.h>

#include "debug.h"

/* private helpers */

static inline jive_operand_list *
jive_input_reg_list_create(jive_graph * graph, const jive_instruction_class * icls, jive_value * const outputs[], jive_operand_bits ** operands)
{
	*operands = 0;
	if (!icls->ninputs) return 0;
	
	*operands = (jive_operand_bits *) jive_malloc(graph, sizeof(jive_operand_bits) * icls->ninputs);
	
	size_t n;
	for(n=0; n<icls->ninputs; n++) {
		jive_cpureg_class_t regcls = icls->inregs[n];
		jive_operand_bits * op = n + *operands;
		op->type = &JIVE_OPERAND_BITS;
		jive_operand_bits_init(op, outputs[n], n);
		jive_operand_set_cpureg_class((jive_operand *) op, regcls);
	}
	
	return (jive_operand_list *)(*operands);
}

static char *
jive_instruction_repr(const void * _self)
{
	const jive_instruction * self = _self;
	const jive_instruction_class * icls = self->icls;
	
	return strdup(icls->name);
}


/* instruction node classes */

const jive_node_class JIVE_INSTRUCTION = {
	0, "INSTRUCTION", sizeof(jive_instruction), 0,
	
	.repr = jive_instruction_repr,
	.equiv = 0,
	.invalidate_inputs = 0,
	.revalidate_outputs = 0
};

jive_node *
jive_instruction_create(jive_graph * graph,
	const jive_instruction_class * icls,
	jive_value * const inout[],
	const long immediates[])
{
	size_t n;
	jive_operand_bits * inputs;
	jive_operand_list * list = jive_input_reg_list_create(graph, icls, inout, &inputs);
	jive_node * _node = jive_node_create(graph, &JIVE_INSTRUCTION, icls->ninputs, list);
	jive_instruction * node = (jive_instruction *) _node;
	
	node->icls = icls;
	node->inregs = inputs;
	node->outregs = jive_malloc(graph, sizeof(jive_value_bits) * icls->noutputs);
	for(n=0; n<icls->noutputs; n++) {
		jive_cpureg_class_t regcls = icls->outregs[n];
		node->outregs[n].type = &JIVE_VALUE_BITS;
		jive_value_bits_init(&node->outregs[n], _node, icls->outregs[n]->nbits);
		jive_value_set_cpureg_class((jive_value *)&node->outregs[n], regcls);
	}
	
	node->immediates = jive_malloc(graph, sizeof(long) * icls->nimmediates);
	memcpy(node->immediates, immediates, sizeof(long) * icls->nimmediates);
	
	node->prev = node->next = 0;
	node->ra_state = jive_regalloc_inststate_none;
	node->cut = 0;
	
	jive_value_multiset_init(&node->active_before);
	jive_regcls_count_init(node->use_count_before);
	jive_regcls_count_init(node->use_count_after);
	
	node->stackslot_prev = node->stackslot_next;
	node->stackslot = 0;
	
	return (jive_node *)node;
}

void
jive_instruction_swap_inputs(jive_node * _node)
{
	jive_instruction * node = (jive_instruction *) _node;
	
	jive_value_bits * value = node->inregs[0].value;
	node->inregs[0].value = node->inregs[1].value;
	node->inregs[1].value = value;
}

void
jive_instruction_use_stackslot(jive_node * node, jive_stackslot * slot)
{
	DEBUG_ASSERT(node->type == &JIVE_INSTRUCTION);
	jive_instruction * instruction = (jive_instruction *)node;
	DEBUG_ASSERT(instruction->stackslot == 0);
	
	instruction->stackslot = slot;
	instruction->stackslot_prev = 0;
	instruction->stackslot_next = slot->first_user;
	if (slot->first_user) slot->first_user->stackslot_prev = instruction;
	else slot->last_user = instruction;
	slot->first_user = instruction;
}

jive_stackslot *
jive_instruction_get_stackslot(const jive_node * node)
{
	DEBUG_ASSERT(node->type == &JIVE_INSTRUCTION);
	jive_instruction * instruction = (jive_instruction *)node;
	return instruction->stackslot;
}

jive_operand_bits *
jive_instruction_input(const void * _node, unsigned int index)
{
	DEBUG_ASSERT( ((const jive_node *)_node)->type == &JIVE_INSTRUCTION );
	const jive_instruction * node = (const jive_instruction *) _node;
	DEBUG_ASSERT(index < node->icls->ninputs);
	return &node->inregs[index];
}

jive_value *
jive_instruction_output(const void * _node, unsigned int index)
{
	DEBUG_ASSERT( ((const jive_node *)_node)->type == &JIVE_INSTRUCTION );
	const jive_instruction * node = (const jive_instruction *) _node;
	DEBUG_ASSERT(index < node->icls->noutputs);
	return (jive_value *) &node->outregs[index];
}

jive_cpureg_t
jive_instruction_inputreg(const void * node, unsigned int index)
{
	return jive_value_get_cpureg((const jive_value *)jive_instruction_input(node, index)->value);
}

jive_cpureg_t
jive_instruction_outputreg(const void * node, unsigned int index)
{
	return jive_value_get_cpureg((const jive_value *)jive_instruction_output(node, index));
}


long
jive_instruction_immediate(const void * _node, unsigned int index)
{
	DEBUG_ASSERT( ((const jive_node *)_node)->type == &JIVE_INSTRUCTION );
	const jive_instruction * node = (const jive_instruction *) _node;
	DEBUG_ASSERT(index < node->icls->nimmediates);
	return node->immediates[index];
}

const jive_instruction_class *
jive_instruction_get_class(const jive_node * _node)
{
	DEBUG_ASSERT( ((const jive_node *)_node)->type == &JIVE_INSTRUCTION );
	const jive_instruction * node = (const jive_instruction *) _node;
	return node->icls;
}

/* instruction sequences */

void
jive_instruction_sequence_init(jive_instruction_sequence * seq)
{
	seq->first = seq->last = 0;
}

void
jive_graph_sequentialize(jive_graph * graph, jive_instruction_sequence * seq)
{
	/* FIXME: not quite correct, as it does not honor regions */
	jive_instruction_sequence_init(seq);
	
	jive_graph_traverser * trav = jive_graph_traverse_topdown(graph);
	jive_node * current;
	
	while ( (current = jive_graph_traverse_next(trav)) != 0) {
		jive_instruction_sequence_append(seq, (jive_instruction *)current);
	}
	
	jive_graph_traverse_finish(trav);
}


void
jive_instruction_sequence_append(jive_instruction_sequence * seq, jive_instruction * instruction)
{
	/* FIXME: should check here that all inputs/outputs have fitting
	and matching cpu registers assigned */
	instruction->prev = seq->last;
	instruction->next = 0;
	if (seq->last) seq->last->next = instruction;
	else seq->first = instruction;
	seq->last = instruction;
}

bool
jive_instruction_sequence_encode(const jive_instruction_sequence * seq, jive_buffer * buffer, const jive_machine * machine)
{
	jive_encode_result result = jive_encode_ok;
	jive_instruction * instruction = seq->first;
	while(instruction) {
		size_t n;
		const jive_cpureg * inputs[instruction->icls->ninputs];
		const jive_cpureg * outputs[instruction->icls->noutputs];
		for(n=0; n<instruction->icls->ninputs; n++) {
			inputs[n] = jive_instruction_inputreg(instruction, n);
		}
		for(n=0; n<instruction->icls->noutputs; n++)
			outputs[n] = jive_instruction_outputreg(instruction, n);
		result = instruction->icls->encode(
			instruction->icls,
			buffer, inputs, outputs, instruction->immediates);
		if (result == jive_encode_out_of_memory)
			return false;
		
		instruction = instruction->next;
	}
	
	return true;
}

static jive_encode_result
jive_encode_pseudo_nop(const jive_instruction_class * icls,
	jive_buffer * target,
	const jive_cpureg * inputs[],
	const jive_cpureg * outputs[],
	const long immediates[])
{
	return jive_encode_ok;
}

/* nop pseudo-instruction */
jive_instruction_class jive_pseudo_nop = {
	.name = "pseudo_nop",
	
	.encode = &jive_encode_pseudo_nop,
	.mnemonic = 0,
	.inregs = 0, .outregs = 0, .flags = jive_instruction_flags_none,
	.ninputs = 0, .noutputs = 0, .nimmediates = 0,
	
	.code = 0
};

const jive_cpureg_class *
jive_cpureg_class_intersect(const jive_cpureg_class * c1, const jive_cpureg_class * c2)
{
	jive_cpureg_classmask_t shared = c1->class_mask | c2->class_mask;
	if (shared == c1->class_mask) return c1;
	if (shared == c2->class_mask) return c2;
	return 0;
}

bool
jive_cpureg_class_contains(const jive_cpureg_class * superior, const jive_cpureg_class * inferior)
{
	jive_cpureg_classmask_t shared = superior->class_mask | inferior->class_mask;
	return (shared == superior->class_mask);
}
