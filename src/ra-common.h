#ifndef JIVE_INTERNAL_RA_COMMON_H
#define JIVE_INTERNAL_RA_COMMON_H

/**
	\file ra-common.h
	
	\brief Common functions for different phases of register allocator
*/

#include <stdbool.h>

#include <jive/graph.h>
#include <jive/instruction.h>
#include <jive/internal/instruction_str.h>

typedef struct _jive_instruction_list {
	jive_instruction ** items;
	unsigned short nitems, space;
} jive_instruction_list;

static inline void
jive_instruction_list_init(jive_instruction_list * list);

static inline void
jive_instruction_list_remove(jive_instruction_list * list, size_t index);

static inline void
jive_instruction_list_append(jive_instruction_list * list, jive_instruction * instruction);

static inline bool
jive_instruction_list_containts(const jive_instruction_list * list, const jive_instruction * instruction);

void
jive_instruction_list_enlarge(jive_graph * graph, jive_instruction_list * list);

static inline void
jive_instruction_list_clear(jive_instruction_list * list);

typedef struct _jive_register_list {
	jive_value_bits ** items;
	unsigned short nitems, space;
} jive_register_list;

static inline void
jive_register_list_init(jive_register_list * list);

static inline void
jive_register_list_remove(jive_register_list * list, size_t index);

void
jive_register_list_enlarge(jive_graph * graph, jive_register_list * list);

static inline void
jive_register_list_append(jive_register_list * list, jive_value_bits * reg);

static inline void
jive_register_list_clear(jive_register_list * list);

typedef jive_instruction_list jive_instruction_heap;

void
jive_instruction_heap_push(jive_instruction_heap * heap, jive_instruction * instr);

jive_instruction *
jive_instruction_heap_pop(jive_instruction_heap * heap);

jive_instruction *
jive_instruction_heap_peek(const jive_instruction_heap * heap);

static inline void
jive_instruction_heap_init(jive_instruction_heap * heap);

/**
	\brief Create new graph cut
*/
jive_graphcut *
jive_graphcut_create(jive_graph * graph);

/**
	\brief Create bottom cut
*/
jive_graphcut *
jive_graphcut_bottom_alloc(jive_graph * graph, const jive_machine * machine);


/**
	\brief Get graph cut above the current one, possibly creating it if it does not exist yet
*/
static inline jive_graphcut *
jive_graphcut_upper(jive_graphcut *gc);

#define MAX_REGISTER_CLASSES 32

/**
	\brief Horizontal cut through the graph
*/
struct _jive_graphcut {
	/* pointers to upper/lower layer */
	jive_graphcut * upper, * lower;
	/* current subgraph */
	jive_graph * subgraph;
	
	/* instructions available for selection into this cut */
	jive_instruction_list available;
	/* instructions meet all criteria for being available except that they reside
	in a different block than the one considered currently */
	jive_instruction_list postponed;
	/* instructions selected into this cut */
	jive_instruction_list selected;
	/* registers active above (and possibly through) this cut */
	jive_register_list registers;
	
	/* number of registers allowed to be used per class */
	short regs_budget[MAX_REGISTER_CLASSES];
	/* number of registers active above this cut */
	short regs_alive[MAX_REGISTER_CLASSES];
	/* number of registers active below this cut */
	short regs_inherited[MAX_REGISTER_CLASSES];
	
	/* number of registers passed through cut unmodified */
	short regs_passthrough[MAX_REGISTER_CLASSES];
	/* number of registers used during execution of instructions at
	"broadest" point in cut (not including passthrough registers) */
	short regs_used[MAX_REGISTER_CLASSES];
	/* number of registers clobbered, i.e. written to during
	execution of instructions but which can be safely overwritten
	as the value contained is not used */
	short regs_clobbered[MAX_REGISTER_CLASSES];
	
	/* blargh -- this should go away */
	bool block_boundary;
	
	int level;
	
	/* calculated total number of instructions within this cut and all lower cuts */
	int total_instructions;
};

/**
	\brief Compute register usage delta
	
	\param current_instruction Instruction to consider
	\param machine The machine description
	\param input_delta Number of registers (per class) that would be added to the set
		regs_active_above if this instruction were selected
	\param output_delta Number of registers (per class) that would be removed from the set
		of passthrough registers if this instruction were selected
	\param used Number of registers (per class) that must be available for
		the duration of this instruction
	\param clobber Number of registers (per class) that are written to, but
		where the value is not needed and can be discarded
	
	Note: the functions relies on the ra_state of the involved registers
	having useful values (i.e. active registers)
*/
void jive_regalloc_compute_regdelta(
	jive_instruction * current_instruction,
	const jive_machine * machine,
	short input_delta[], short output_delta[],
	short used[], short clobber[]);

#if 0

/* FIXME: does not belong here */

/**
	\brief Close subgraphs
	
	\param g Graph to operate on
	
	"Closes" all subgraphs of a graph: all instructions "above" the bottom node
	will receive an additional state dependency edge moving it above the top node,
	and all instructions "below" the bottem node will receive an additional
	state dependency edge moving it below the bottom node.
	
	The effect is to prevent accidental "migration" of instructions into
	a block where they don't belong.
*/
void jive_regalloc_close_subgraphs(jive_graph *g);

#endif

/* implementations */

static inline jive_graphcut *
jive_graphcut_upper(jive_graphcut *gc)
{
	if (!gc->upper) {
		jive_graphcut * tmp = jive_graphcut_create(gc->subgraph);
		
		gc->upper = tmp;
		tmp->lower = gc;
		tmp->upper = 0;
	}
	
	return gc->upper;
}

static inline void
jive_instruction_list_init(jive_instruction_list * list)
{
	list->items = 0;
	list->nitems = list->space = 0;
}

static inline void
jive_instruction_list_remove(jive_instruction_list * list, size_t index)
{
	memmove(&list->items[index], &list->items[index+1],
		(list->nitems-index-1) * sizeof(jive_instruction *));
	list->nitems--;
}

static inline void
jive_instruction_list_append(jive_instruction_list * list, jive_instruction * instruction)
{
	if (list->nitems == list->space)
		jive_instruction_list_enlarge(instruction->base.graph, list);
	list->items[list->nitems] = instruction;
	list->nitems++;
}

static inline bool
jive_instruction_list_containts(const jive_instruction_list * list, const jive_instruction * instruction)
{
	size_t n;
	for(n=0; n<list->nitems; n++)
		if (list->items[n] == instruction) return true;
	return false;
}

static inline void
jive_instruction_list_clear(jive_instruction_list * list)
{
	list->nitems = 0;
}

static inline void
jive_register_list_init(jive_register_list * list)
{
	list->items = 0;
	list->nitems = list->space = 0;
}

static inline void
jive_register_list_remove(jive_register_list * list, size_t index)
{
	memmove(&list->items[index], &list->items[index+1],
		(list->nitems-index-1) * sizeof(jive_value_bits *));
	list->nitems--;
}

static inline void
jive_register_list_append(jive_register_list * list, jive_value_bits * reg)
{
	if (list->nitems == list->space)
		jive_register_list_enlarge(reg->node->graph, list);
	list->items[list->nitems] = reg;
	list->nitems++;
}

static inline void
jive_register_list_clear(jive_register_list * list)
{
	list->nitems = 0;
}

static inline void
jive_instruction_heap_init(jive_instruction_heap * heap)
{
	jive_instruction_list_init(heap);
}

#endif
