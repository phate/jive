#ifndef JIVE_INTERNAL_INSTRUCTION_STR_H
#define JIVE_INTERNAL_INSTRUCTION_STR_H

#include <jive/instruction.h>
#include <jive/bitstring.h>

#include <jive/machine.h>
#include <jive/internal/metacontainers.h>

typedef struct jive_value_multiset jive_value_multiset;

static inline void *
jive_value_multiset_realloc(void * ptr, size_t old_size, size_t new_size, jive_value * value)
{
	void * new_ptr = jive_malloc(value->node->graph, new_size);
	memcpy(new_ptr, ptr, old_size);
	return new_ptr;
}

DEFINE_MULTISET_TYPE(jive_value_multiset, jive_value *, jive_value_multiset_realloc);

struct jive_instruction {
	jive_node base;
	const jive_instruction_class * icls;
	long * immediates;
	jive_operand_bits * inregs;
	jive_value_bits * outregs;
	
	/* linked list of instructions */
	jive_instruction * prev, * next;
	
	/* the following members are used by the register allocator */
	
	/* cut this instruction has been assigned to */
	jive_graphcut * cut;
	
	jive_regalloc_inststate ra_state;
	
	/* set of registers active just before this instruction */
	jive_value_multiset active_before;
	/* usage summary of register classes before and after this
	instruction; in addition to the actual values used, defined
	or passed by this instruction, this also includes any
	"auxiliary" values (caused by "reload" instructions to
	satisfy register constraints, or values "clobbered" as
	side-effects of this instruction)  */
	jive_regcls_count use_count_before;
	jive_regcls_count use_count_after;
	
	jive_instruction * stackslot_prev, * stackslot_next;
	jive_stackslot * stackslot;
};

#endif
