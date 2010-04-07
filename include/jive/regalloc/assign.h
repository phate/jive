#ifndef JIVE_REGALLOC_ASSIGN_H
#define JIVE_REGALLOC_ASSIGN_H

#include <jive/types.h>
#include <jive/instruction.h>
#include <jive/machine.h>

void
jive_regalloc_assign(
	jive_instruction_sequence * seq,
	const jive_machine * machine,
	jive_stackframe * stackframe);

#include <jive/internal/metacontainers.h>

typedef struct jive_interference_graph jive_interference_graph;
typedef struct jive_reg_candidate jive_reg_candidate;
typedef struct jive_interference_set jive_interference_set;
typedef struct jive_cand_set jive_cand_set;

static inline void *
jive_cand_realloc(void * ptr, size_t old_size, size_t new_size, jive_reg_candidate * cand);

DEFINE_MULTISET_TYPE(jive_interference_set, jive_reg_candidate *, jive_cand_realloc);

DEFINE_SET_TYPE(jive_cand_set, jive_reg_candidate *, jive_cand_realloc);

struct jive_reg_candidate {
	jive_value * value;
	jive_interference_graph * igraph;
	
	/* class from which to pick */
	const jive_cpureg_class * regcls;
	
	/* assigned register; NULL if not processed yet */
	const jive_cpureg * reg;
	
	/* number of registers available in the class required for this value
	minus number of neighbours which can pick from the same class (but
	have not picked yet) */
	int squeeze;
	
	jive_interference_set interference;
	
	/* FIXME: bitmask needs to be large enough */
	uint64_t allowed_regs;
	/* number of registers in the class required for this value not assigned
	to any neighbour yet */
	unsigned int allowed_regs_count;
	
	/* position of node within priority heap */
	// size_t index;
};

struct jive_interference_graph {
	jive_graph * graph;
	const jive_machine * machine;
	
	jive_instruction_sequence seq;
	
	/* nodes to be processed; FIXME: should be "list", not "set" */
	jive_interference_set cand;
	
	/* FIXME: use a more appropriate data structure */
	/* all nodes, to determine mapping between values and candidates */
	jive_cand_set map;
};

static inline void *
jive_cand_realloc(void * ptr, size_t old_size, size_t new_size, jive_reg_candidate * cand)
{
	void * new_ptr = jive_malloc(cand->igraph->graph, new_size);
	memcpy(new_ptr, ptr, old_size);
	return new_ptr;
}

jive_reg_candidate *
jive_interference_graph_map_value(jive_interference_graph * igraph, jive_value * value);

#endif
