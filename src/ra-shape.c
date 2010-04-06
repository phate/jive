#include "ra-shape.h"
#include "ra-graphcut-cache.h"
#include <jive/graph.h>
#include <jive/bitstring.h>
#include <jive/internal/debug.h>
#include <jive/machine.h>

typedef struct _spillrequest {
	/* the register that must be spilled */
	jive_value_bits * reg;
	/* the instruction that is responsible for the spill */
	jive_instruction * forced_instruction;
	/* the subgraph the forced instruction is in (FIXME: do I need that?) */
	jive_graph * subgraph;
	/* the instructions already pushed into a cut on making the
	spill decision */
	jive_instruction_list users;
	/* total number of instructions successfully selected before making a spill request; used to decide on which of two
	"alternate" spill options is better */
	unsigned int total_instructions;
	/* graphcut depth */
	unsigned int level;
} spillrequest;

static void spillrequest_init(spillrequest *req)
{
	req->forced_instruction = 0;
	req->reg = 0;
	req->subgraph = 0;
	jive_instruction_list_init(&req->users);
	req->total_instructions = 0;
	req->level = 0;
}

typedef struct _jive_graphshaper_state {
	jive_graph * graph;
	jive_graphcut_state_cache * cache;
	jive_graphcut * current_cut, * backtrack_boundary;
	spillrequest spill;
	const jive_machine * machine;
	jive_cpureg_class_t contended_regcls;
	jive_stackframe *stackframe;
	jive_instruction_heap available_next;
} jive_graphshaper_state;

/* select one available instruction into the current cut, update
states of input registers, check which new instructions become
available above */
static void
graphcut_select_instruction(jive_graphcut * cut,
	jive_instruction * instr,
	jive_instruction_heap * available_next)
{
	DEBUG_ASSERT(instr->ra_state == jive_regalloc_inststate_available);
	DEBUG_ASSERT(instr->cut == 0);
	DEBUG_ASSERT(jive_instruction_list_containts(&cut->available, instr));
	
	/* mark output registers as SHAPED, so they will be removed from
	active set of current cut */
	jive_value * out = jive_node_iterate_values((jive_node *)instr);
	while(out) {
		jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(out);
		DEBUG_ASSERT(ra_state == jive_regalloc_regstate_readers_done || ra_state == jive_regalloc_regstate_none);
		if (ra_state == jive_regalloc_regstate_readers_done)
			jive_value_set_regalloc_regstate(out, jive_regalloc_regstate_shaped);
		out = out->next;
	}
	
	/* mark instruction as "selected", to enable "new" instructions to
	become available for selection in the next cut */
	instr->ra_state = jive_regalloc_inststate_selected;
	instr->cut = cut;
	jive_instruction_list_append(&cut->selected, instr);
	
	/* iterate through input registers and state-depending instructions;
	mark input registers appropriately, and make new instructions
	available for selection into next cut */
	jive_input_edge_iterator i;
	JIVE_ITERATE_INPUTS(i, (jive_node *)instr) {
		jive_instruction * upper_instruction = 0;
		if (!jive_edge_is_state_edge(i)) {
			jive_value * reg = i->origin.port;
			jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(reg);
			DEBUG_ASSERT(ra_state == jive_regalloc_regstate_newly_active || ra_state == jive_regalloc_regstate_none || ra_state == jive_regalloc_regstate_readers_active);
			
			if (ra_state == jive_regalloc_regstate_none) {
				/* if register is not yet in active set, add it */
				jive_register_list_append(&cut->registers, (jive_value_bits *)reg);
				jive_value_set_regalloc_regstate(reg, jive_regalloc_regstate_newly_active);
			} else if (ra_state == jive_regalloc_regstate_newly_active) {
				/* the register has been introduced by one previous instruction into this cut; change to simply "active", as all required special-casing has already been performed */
				jive_value_set_regalloc_regstate(reg, jive_regalloc_regstate_readers_active);
			}
			
			/* check if all readers are finished with this register */
			jive_output_edge_iterator o;
			JIVE_ITERATE_OUTPUTS(o, i->origin.node) {
				if (o->origin.port != reg) continue;
				DEBUG_ASSERT(o->target.node->type == &JIVE_INSTRUCTION);
				jive_instruction * dep = (jive_instruction *)o->target.node;
				if (dep->ra_state != jive_regalloc_inststate_shaped && dep->ra_state != jive_regalloc_inststate_selected) break;
			}
			/* unless all using instructions have been selected, let register remain
			in ACTIVE state */
			if (o) continue;
			jive_value_set_regalloc_regstate(reg, jive_regalloc_regstate_readers_done);
			upper_instruction = (jive_instruction *) i->origin.node;
		} else {
			DEBUG_ASSERT(i->origin.node->type == &JIVE_INSTRUCTION);
			upper_instruction = (jive_instruction *) i->origin.node;
		}
		
		DEBUG_ASSERT(upper_instruction->ra_state == jive_regalloc_inststate_none || upper_instruction->ra_state == jive_regalloc_inststate_available_next);
		/* it is possible that this instruction has already been selected into the
		next upper cut, if there are two paths connecting two instructions, we
		marked the instruction on the first already and have now found the second
		one -- just ignore the second pass */
		if (upper_instruction->ra_state == jive_regalloc_inststate_available_next) continue;
		
		jive_output_edge_iterator o;
		JIVE_ITERATE_OUTPUTS(o, (jive_node *)upper_instruction) {
			/* check that all dependent instructions have been
			selected already; this automatically implies that
			all output registers are ready  */
			DEBUG_ASSERT(o->target.node->type == &JIVE_INSTRUCTION);
			jive_regalloc_inststate ra_state = ((jive_instruction *)o->target.node)->ra_state;
			if (ra_state != jive_regalloc_inststate_shaped && ra_state != jive_regalloc_inststate_selected) break;
		}
		if (o) continue;
		/* there is nothing obstructing this instruction, so record it for the next round */
		upper_instruction->ra_state = jive_regalloc_inststate_available_next;
		jive_instruction_heap_push(available_next, upper_instruction);
	}
	
	cut->total_instructions++;
}

/* called during backtracking, to undo the markings set by graphcut_select;
note that this does not modify any node lists (available instructions/active
registers) as their state is preserved in the cut anyway and need not be
reconstructed "manually"  */
static void
graphcut_undo_select_instruction(jive_graphcut * cut, jive_instruction * instr)
{
	DEBUG_ASSERT(instr->cut);
	instr->cut = 0;
	instr->ra_state = jive_regalloc_inststate_available;
	
	/* first, treat output registers */
	/* set all registers to "none" state ... */
	jive_value * out = jive_node_iterate_values((jive_node *)instr);
	while(out) {
		jive_value_set_regalloc_regstate(out, jive_regalloc_regstate_none);
		out = out->next;
	}
	/* ... except for those that are actually used by some
	instruction below */
	jive_output_edge_iterator o;
	JIVE_ITERATE_OUTPUTS(o, (jive_node *)instr) {
		if (jive_edge_is_state_edge(o)) continue;
		jive_value_set_regalloc_regstate(o->origin.port, jive_regalloc_regstate_readers_done);
	}
	
	/* now restore state of input registers */
	jive_input_edge_iterator i;
	JIVE_ITERATE_INPUTS(i, (jive_node *) instr) {
		jive_instruction * upper_instr=0;
		if (!jive_edge_is_state_edge(i)) {
			jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(i->origin.port);
			/* if this is the first reader of this register,
			also reset state of upper instruction */
			if (ra_state == jive_regalloc_regstate_readers_done)
				upper_instr = (jive_instruction *)i->origin.node;
			/* register must bear "readers_active" mark
			iff there is at least one other reader */
			jive_value_set_regalloc_regstate(i->origin.port, jive_regalloc_regstate_readers_active);
			/* check if there is no other reader */
			JIVE_ITERATE_OUTPUTS(o, i->origin.node) {
				if (o->origin.port != i->origin.port)
					continue;
				jive_instruction * tmp = (jive_instruction *)i->target.node;
				if (tmp->ra_state == jive_regalloc_inststate_shaped) break;
			}
			if (!o) jive_value_set_regalloc_regstate(i->origin.port, jive_regalloc_regstate_none);
		} else {
			DEBUG_ASSERT(i->origin.node->type == &JIVE_INSTRUCTION);
			upper_instr = (jive_instruction *)i->origin.node;
		}
		
		if (!upper_instr) continue;
		upper_instr->ra_state = jive_regalloc_inststate_none;
	}
}

static inline int max(int a, int b) {return a>b ? a : b;}

/* test whether given instruction can be added to the cut,
constrained by the register budget; returns either NULL
if instruction fits in budget, or (one of) the contended
register class(es) */
static jive_cpureg_class_t
graphcut_try_select_instruction(
	jive_graphcut * cut,
	jive_instruction * instr,
	const jive_machine * machine,
	jive_instruction_heap * available_next)
{
	short input_delta[MAX_REGISTER_CLASSES],
		output_delta[MAX_REGISTER_CLASSES],
		used[MAX_REGISTER_CLASSES],
		clobber[MAX_REGISTER_CLASSES];
	jive_regalloc_compute_regdelta(instr, machine,
		input_delta, output_delta, used, clobber);
	
	short new_passthrough[MAX_REGISTER_CLASSES],
		new_clobber[MAX_REGISTER_CLASSES],
		new_alive[MAX_REGISTER_CLASSES],
		new_used[MAX_REGISTER_CLASSES];
	size_t n;
	for(n=0; n<machine->nregcls; n++) {
		new_passthrough[n]=cut->regs_passthrough[n]-output_delta[n];
		new_clobber[n]=max(cut->regs_clobbered[n], clobber[n]);
		new_alive[n]=cut->regs_alive[n]-output_delta[n]+input_delta[n];
		new_used[n]=cut->regs_used[n]+used[n];
		
		/* may only issue if number of registers concurrently in use
		does not exceed architecture limit */
		if (new_used[n]+new_passthrough[n]+new_clobber[n] > machine->regcls[n].nregs)
			return n;
		/* if the number of registers alive above this cut exceeds
		architecture limit, disallow */
		if (new_alive[n] > machine->regcls[n].nregs)
			return n;
		/* if the number of registers alive above this cut exceeds set
		budget disallow *except* if this instruction reduces the number
		of registers in a contended class (so that we may ultimately
		still return into budget) -- note that this implicitly also
		tests for the architecture limit */
		if (new_alive[n] > cut->regs_budget[n] && input_delta[n]>=output_delta[n])
			return n;
		/* otherwise, allow */
		
	}
	
	graphcut_select_instruction(cut, instr, available_next);
	
	for(n=0; n<machine->nregcls; n++) {
		cut->regs_passthrough[n]=new_passthrough[n];
		cut->regs_clobbered[n]=new_clobber[n];
		cut->regs_alive[n]=new_alive[n];
		cut->regs_used[n]=new_used[n];
	}
	return 0;
}













static void
jive_graphshaper_state_init(
	jive_graphshaper_state * state,
	jive_graph * graph,
	const jive_machine * machine,
	jive_stackframe * stackframe)
{
	state->graph = graph;
	state->cache = jive_graphcut_state_cache_create(graph);
	spillrequest_init(&state->spill);
	state->machine = machine;
	state->contended_regcls = 0;
	state->stackframe = stackframe;
	jive_instruction_heap_init(&state->available_next);
	
	/* start at the bottom of the graph with the last instruction
	to be executed and propagate backwards */
	jive_graphcut * bottom = jive_graphcut_bottom_alloc(graph, machine);
	state->current_cut = bottom;
	/* initially, allow backtracking up to the bottom node */
	state->backtrack_boundary = bottom; 
}

typedef enum {
	Success=0,
	AlreadySelected=1,
	Contention=2
} selection_state_t;

/* attempt to place a single instruction into the cut, and update state */
static selection_state_t
fill_graphcut_single(jive_graphshaper_state * state,
	jive_graphcut * cut,
	jive_instruction * instr,
	jive_cpureg_class_t* contended_regcls,
	jive_instruction ** forced_instruction)
{
	if (((jive_instruction *)instr)->ra_state != jive_regalloc_inststate_available) return AlreadySelected;

#if 0
	/* FIXME: need to reconsider block boundary handling */
	/* selection of a block boundary instruction precludes selecting any
	other instruction into the same cut */
	bool instr_block_boundary=graph_instruction_block_boundary((instruction *)instr);
	if (instr_block_boundary && cut->selected.count>0) return Contention;
#endif
	DEBUG_PRINTF("consider: %p\n", instr);
	jive_cpureg_class_t tmp = graphcut_try_select_instruction(state->current_cut,
		instr, state->machine, &state->available_next);
	if (tmp) {
		if (!*contended_regcls) {
			*contended_regcls = tmp;
			*forced_instruction = instr;
		}
		return Contention;
	}
#ifdef RA_SHAPE_DEBUG
	DEBUG_PRINTF("select %p: %s", instr, ((instruction *)instr)->type->name);
	edge_iterator i;
	ITERATE_VALUE_INPUTS(i, instr)
		dprintf(" %p(%s)", i.node, graph_bitstring_register_class(i.node)->name);
	dprintf(" ->");
	ITERATE_OUTPUTS(i, instr) {
		if (i.node->type!=&VIRTUAL_REGISTER_NODE) continue;
		dprintf(" %p(%s)", i.node, graph_bitstring_register_class(i.node)->name);
	}
	DEBUG_PRINTF("\n");
	size_t k;
	for(k=0; k<state->machine->regset->nclasses; k++)
	DEBUG_PRINTF("%s: %d/%d+%d,%d/%d ", state->machine->regset->classes[k]->name,
		cut->regs_alive[k], cut->regs_passthrough[k], cut->regs_used[k],
		cut->regs_clobbered[k], cut->regs_budget[k]);
	DEBUG_PRINTF("\n");
#endif
#if 0
	/* FIXME: block boundary handling */
	if (instr_block_boundary) cut->block_boundary=true;
#endif
	return Success;
}


static void
select_spill_register(jive_graphshaper_state * state, jive_instruction * forced_instruction)
{
	jive_cpureg_class_t contended_regcls = state->contended_regcls;
	jive_graphcut * cut = state->current_cut;
	const jive_register_list * lower_regs = &cut->lower->registers;
	/* check if the current state is an improvement over previous
	attempts. This requires that
	
	& more instructions were successfully selected than last time
	& and
	  | no other spill selection yet
	  | OR the same register class is contended as last time
	  | OR we made it past the previous bottleneck
	
	actually I think the middle condition could be dropped */
	if (cut->total_instructions > state->spill.total_instructions &&
	    (!state->spill.reg
		|| contended_regcls == jive_value_get_cpureg_class((jive_value *)state->spill.reg)
		|| state->spill.forced_instruction->ra_state == jive_regalloc_inststate_selected)) {
		state->spill.total_instructions = cut->total_instructions;
		state->spill.forced_instruction = forced_instruction;
		int n = lower_regs->nitems;
		do {
			n--;
			jive_value_bits * tmp = lower_regs->items[n];
			/* choose candidate for spilling; it
			must have the same register class, must not
			obstruct the forced instruction, and must 
			be spillable */
			if (jive_value_get_cpureg_class((jive_value *)tmp) != contended_regcls)
				continue;
			if (tmp->node == (jive_node *)forced_instruction)
				continue;
			if (!jive_value_get_mayspill((jive_value *)tmp))
				continue;
			state->spill.reg=tmp;
			break;
		} while(n>=0);
		DEBUG_ASSERT(n>=0);
		
		/* build list of instructions that use the spilled value */
		jive_instruction_list_clear(&state->spill.users);
		jive_output_edge_iterator i;
		JIVE_ITERATE_OUTPUTS(i, state->spill.reg->node) {
			if (i->origin.port != (jive_value *)state->spill.reg)
				continue;
			jive_instruction * instr = (jive_instruction *)i->target.node;
			if (instr->ra_state == jive_regalloc_inststate_shaped)
				jive_instruction_list_append(&state->spill.users, instr);
		}
		
		state->spill.subgraph=cut->subgraph;
	}
}

/* greedily fill this cut with instructions; returns true if at least
one instruction could be filled in, or false otherwise */
static bool
fill_graphcut(jive_graphshaper_state * state)
{
	bool progress = false;
	jive_cpureg_class_t contended_regcls = 0;
	jive_instruction * forced_instruction = 0;
	size_t n;
	
	jive_graphcut * cut = state->current_cut;
	const jive_register_list * lower_regs;
	if (cut->lower) lower_regs = &cut->lower->registers;
	else {
		static const jive_register_list empty_list = {0, 0};
		lower_regs = &empty_list;
	}
	
#ifdef RA_SHAPE_DEBUG
	DEBUG_PRINTF("begin: %d\n", cut->level);
	for(n=0; n<lower_regs->count; n++) {
		virtual_register *vreg=(virtual_register *)lower_regs->nodes[n];
		ASSERT(vreg->ra_state==RA_STATE_ACTIVE || vreg->ra_state==RA_STATE_READY);
		dprintf("%p:%s(%d) ", vreg,
			graph_bitstring_register_class(lower_regs->nodes[n])->name,
			vreg->ra_state);
	}
	DEBUG_PRINTF("\n");
	for(n=0; n<cut->available.count; n++)
		dprintf("%p:%s ", cut->available.nodes[n],
			((instruction *)cut->available.nodes[n])->type->name);
	DEBUG_PRINTF("\n");
	for(n=0; n<state->machine->regset->nclasses; n++)
		dprintf("%s: %d/%d ", state->machine->regset->classes[n]->name,
			cut->regs_inherited[n], cut->regs_budget[n]);
	DEBUG_PRINTF("\n");
#endif
	
	/* examine set of active registers, and try to "eliminate"
	them from the active set by scheduling the instructions
	that use and define them; honour priority order of registers */
	for(n=0; n<lower_regs->nitems && !cut->block_boundary; n++) {
		jive_value_bits * reg = lower_regs->items[n];
		jive_instruction * defining_instr = (jive_instruction *)reg->node;
		
		selection_state_t result = fill_graphcut_single(state, cut,
			defining_instr, &contended_regcls, &forced_instruction);
		if (result == Success) progress = true;
	}
	
	for(n=0; n<cut->available.nitems && !cut->block_boundary; n++) {
		jive_instruction * instr = cut->available.items[n];
		
		selection_state_t result = fill_graphcut_single(state, state->current_cut,
			instr, &contended_regcls, &forced_instruction);
		/* for state dependencies, maintain strict priority order,
		i.e. break off if an instruction of higher priority remains
		unselected */
		if (result == Contention) break;
		if (result == Success) progress = true;
	}
	
	DEBUG_PRINTF("finished: %d %sprogress\n", cut->level, progress?"":"no ");
	if (progress) {
		/* mark all "newly active" registers as simply "active" */
		for(n=0; n<cut->registers.nitems; n++) {
			jive_value * reg = (jive_value *)cut->registers.items[n];
			jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(reg);
			DEBUG_ASSERT(ra_state == jive_regalloc_regstate_shaped || ra_state == jive_regalloc_regstate_readers_active || ra_state == jive_regalloc_regstate_newly_active || ra_state == jive_regalloc_regstate_readers_done);
			
			if (ra_state == jive_regalloc_regstate_newly_active)
				jive_value_set_regalloc_regstate(reg, jive_regalloc_regstate_readers_active);
			
			/* FIXME: assign cpureg here */
		}
		/* carry over remaining registers */
		for(n=0; n<lower_regs->nitems; n++) {
			jive_value * reg = (jive_value *)lower_regs->items[n];
			jive_regalloc_regstate ra_state = jive_value_get_regalloc_regstate(reg);
			if (ra_state == jive_regalloc_regstate_shaped) continue;
			jive_register_list_append(&cut->registers, lower_regs->items[n]);
		}
	} else {
		DEBUG_PRINTF("contended: %d\n", contended_regcls);
		state->contended_regcls = contended_regcls;
		select_spill_register(state, forced_instruction);
	}
	
	#if 0
	/* FIXME: this is broken as registers must be chosen
	for "most constrained" values first to ensure that
	a valid assignment can be found */
	if (progress) {
		/* now assign registers */
		size_t n;
		for(n=0; n<cut->registers.nitems; n++) {
			jive_value * val = (jive_value *) cut->registers.items[n];
			
			jive_cpureg_class_t regcls = jive_value_get_cpureg_class(val);
			jive_cpureg_t reg = state->machine->regcls[regcls].first_reg;
			jive_value_set_cpureg(val, reg);
		}
	}
	#endif
	
	return progress;
}

static void
graphcut_revert(jive_graphcut * cut)
{
	size_t n;
	for(n=0; n<cut->selected.nitems; n++)
		graphcut_undo_select_instruction(cut, cut->selected.items[n]);
	
	jive_instruction_list_clear(&cut->selected);
	jive_register_list_clear(&cut->registers);
	
	/* reset register usage */
	for(n=0; n<MAX_REGISTER_CLASSES; n++) {
		cut->regs_alive[n] = cut->regs_passthrough[n] = cut->regs_inherited[n];
		cut->regs_used[n] = cut->regs_clobbered[n] = 0;
	}
	cut->block_boundary = false;
}

static int safe_depth(const jive_instruction * instr)
{
	if (instr) return instr->base.depth_from_root;
	else return -1;
}

/* given a completed cut, prepare the next cut above */
static jive_graphcut *
prepare_upper_cut(jive_graphcut * cut, jive_graph * graph,
	const jive_machine * machine,
	jive_instruction_heap * available_next,
	jive_graph * subgraph)
{
	jive_graphcut * upper = jive_graphcut_upper(cut);
	
	upper->level = cut->level+1;
	upper->subgraph = cut->subgraph;
	upper->total_instructions = cut->total_instructions;
	upper->block_boundary = 0;
	
	jive_instruction_list_clear(&upper->available);
	jive_instruction_list_clear(&upper->postponed);
	jive_instruction_list_clear(&upper->selected);
	jive_register_list_clear(&upper->registers);
	
	/* fill list of instructions available for selection next cut;
	these are instructions carried over from the previous cut plus any
	newly-available instructions */
	size_t count = cut->available.nitems + cut->postponed.nitems + available_next->nitems;
	size_t pos_available=0, pos_postponed=0;
	/* interleave nodes from all sources properly to keep the list sorted */
	while(count--) {
		jive_instruction * available = 0, * postponed = 0, * next = 0;
		if (pos_available<cut->available.nitems) {
			available = cut->available.items[pos_available];
			/* only carry over those nodes that have *not* been selected
			into lower cut */
			if (available->ra_state == jive_regalloc_inststate_selected) {
				available->ra_state = jive_regalloc_inststate_shaped;
				pos_available++;
				continue;
			}
		}
		if (pos_postponed<cut->postponed.nitems) {
			postponed = cut->postponed.items[pos_postponed];
			DEBUG_ASSERT(postponed->ra_state == jive_regalloc_inststate_available);
		}
		if (available_next->nitems) {
			next = jive_instruction_heap_peek(available_next);
			DEBUG_ASSERT(next->ra_state == jive_regalloc_inststate_available_next);
		}
		
		/* out of the candidate nodes, thoose the one with highest
		depth from root */
		jive_instruction * pick;
		if (safe_depth(available)>safe_depth(postponed)) {
			if (safe_depth(available)>safe_depth(next)) {
				pick = available;
				pos_available++;
			} else {
				jive_instruction_heap_pop(available_next);
				pick = next;
			}
		} else {
			if (safe_depth(postponed)>safe_depth(next)) {
				pick = postponed;
				pos_postponed++;
			} else {
				jive_instruction_heap_pop(available_next);
				pick = next;
			}
		}
		pick->ra_state = jive_regalloc_inststate_available;
#if 0 /* FIXME: block boundary handling */
		if (instruction_in_subgraph(pick, cut->subgraph))
			nodelist_add(&upper->available, pick, alloc);
		else
			nodelist_add(&upper->postponed, pick, alloc);
#endif
		jive_instruction_list_append(&upper->available, pick);
	}
		
	size_t n;
#ifdef RA_SHAPE_DEBUG
	short regs_alive[MAX_REGISTER_CLASSES];
	memset(regs_alive, 0, sizeof(regs_alive));
	for(n=0; n<cut->registers.count; n++)
		regs_alive[graph_bitstring_register_class(cut->registers.nodes[n])->index]++;
	for(n=0; n<regset->nclasses; n++)
		ASSERT(regs_alive[n]==cut->regs_alive[n]);
#endif
	
	/* fill register budget */
	for(n=0; n<machine->nregcls; n++) {
		upper->regs_budget[n] = machine->regcls[n].nregs;
		upper->regs_inherited[n] = cut->regs_alive[n];
		upper->regs_alive[n] = upper->regs_inherited[n];
		upper->regs_passthrough[n] = upper->regs_inherited[n];
		upper->regs_used[n] = 0;
		upper->regs_clobbered[n] = 0;
	}
	
	return upper;
}

static bool
do_graphshape(jive_graphshaper_state * state)
{
	unsigned int backtrack_cutoff=1024;
	
	while(state->current_cut->available.nitems != 0) {
		bool progress = fill_graphcut(state);
		
		if (progress) {
			jive_graph * subgraph = state->current_cut->subgraph;
#if 0 /* FIXME: block boundary handling */
			jive_instruction * first = state->current_cut->selected.items[0];
			if (graph_instruction_block_boundary(first)) {
				dprintf("block_boundary\n");
				ASSERT(state->current_cut->selected.count==1);
				subgraph=first->subgraph;
			}
#endif
			state->current_cut = prepare_upper_cut(state->current_cut,
				state->graph, state->machine, &state->available_next,
				subgraph);
			continue;
		}
		
		jive_cpureg_class_t contended_regcls = state->contended_regcls;
		
		/* failed to issue any nodes in previous cut -- backtrack, unless
		limits reached */
		
		/* retry with state above */
		state->current_cut = state->current_cut->lower;
		/* don't backtrack beyond current lower limit */
		if (state->current_cut == state->backtrack_boundary) return false;
		/* don't backtrack if backtracking limit exceeded */
		if (!backtrack_cutoff--) return false;
		
		/* reduce budget in contended register class (and all
		arent classes!) to one less than previously used so
		fewer registers of this type will be active the next
		time around */
		while(contended_regcls != -1 /* FIXME: constant! */) {
			state->current_cut->regs_budget[contended_regcls]=
			state->current_cut->upper->regs_alive[contended_regcls]-1;
			contended_regcls = state->machine->regcls[contended_regcls].parent;
			
		}
		graphcut_revert(state->current_cut);
		DEBUG_PRINTF("reduce budget: %d %d\n", state->current_cut->level, state->current_cut->regs_budget[contended_regcls]);
	}
	
	/* we are now one past the last cut containing an instruction,
	so back up */
	state->current_cut=state->current_cut->lower;
	
	return true;
}








jive_graphcut *
jive_graphshape(
	jive_graph * graph,
	const jive_machine * machine,
	jive_stackframe * stackframe)
{
#if 0
	regalloc_close_subgraphs(g);
	graph_prune(g);
	
	/* FIXME: loop dieting still missing */
	
#endif
	/* setup state and scratch variables used by all subsequent
	functions */
	jive_graphshaper_state state;
	jive_graphshaper_state_init(&state, graph, machine, stackframe);
	jive_graphcut * bottom = state.current_cut;
	
	size_t nspills=0;
	/* repeat until we have shaped the whole graph without requiring
	further spills */
	while(!do_graphshape(&state)) {
		while(state.current_cut) {
			graphcut_revert(state.current_cut);
			state.current_cut=state.current_cut->lower;
		}
		/* spill something */
		DEBUG_PRINTF("spill %zd %p %s\n", nspills, state.spill.reg,
			machine->regcls[jive_value_get_cpureg_class((jive_value *)state.spill.reg)].name);
		
		/* FIXME: spilling */
		#if 0
		/* FIXME: needs to take the forced_instruction as an argument so
		the spilling instructions can be forced above through a state
		edge */
		regalloc_spill(state.spill.subgraph, machine, frame, state.spill.reg,
			state.spill.instr.items, state.spill.instr.nitems);
		
		/* clean up after modifying the graph */
		regalloc_close_subgraphs(g);
		graph_prune(g);
		#endif
		abort();
		
		/* FIXME: replay state and such things. */
		state.current_cut = bottom;
		spillrequest_init(&state.spill);
		/* restart at bottom? no... */
		state.backtrack_boundary=state.current_cut;
		jive_graphcut_state_cache_clear(state.cache);
		
		/* reset register budget */
		size_t n;
		for(n=0; n<machine->nregcls; n++)
			state.current_cut->regs_budget[n] = machine->regcls[n].nregs;
	}
	
	return state.current_cut;
}

