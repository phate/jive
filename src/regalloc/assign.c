#include <jive/regalloc/assign.h>
#include <jive/regalloc/cut.h>
#include <jive/regalloc/spill.h>
#include <jive/machine.h>

#include "debug.h"

#include <jive/graphdebug.h>

#include <stdio.h>

/* debugging aids! this function recalculates all incrementally updated
data and verifies that the stored data matches */
void
jive_interference_graph_validate(jive_interference_graph * igraph)
{
	/* verify that the list and count of active values before each
	instruction is consistent */
	jive_value_multiset active_values;
	jive_value_multiset_init(&active_values);
	jive_instruction * instr = igraph->seq.last;
	while(instr) {
		jive_value * value = jive_node_iterate_values((jive_node *) instr);
		while(value) {
			jive_value_multiset_remove_all(&active_values, value);
			value = value->next;
		}
		jive_operand * operand = jive_node_iterate_operands((jive_node *) instr);
		while(operand) {
			value = operand->value;
			jive_value_multiset_add(&active_values, value);
			operand = operand->next;
		}
		
		DEBUG_ASSERT(jive_value_multiset_equals_relaxed(&active_values, &instr->active_before));
		
		instr = instr->prev;
	}
	
	/* verify that the list and count of interfering values for
	each register candidate is consistent */
	size_t n;
	jive_interference_set interfere;
	jive_interference_set_init(&interfere);
	
	for(n=0; n<igraph->map.nitems; n++) {
		jive_reg_candidate * cand = igraph->map.items[n];
		jive_interference_set_clear(&interfere);
		
		jive_instruction * instr = igraph->seq.last;
		while(instr) {
			jive_value_multiset * active = &instr->active_before;
			instr = instr->prev;
			
			if (!jive_value_multiset_contains(active, cand->value))
				continue;
			size_t k;
			for(k=0; k<active->nitems; k++)
				if (active->items[k].value != cand->value)
					jive_interference_set_add(&interfere, jive_interference_graph_map_value(igraph, active->items[k].value));
		}
		
		DEBUG_ASSERT(jive_interference_set_equals(&interfere, &cand->interference));
		
		DEBUG_ASSERT(jive_value_get_cpureg_class_shared(cand->value) == cand->regcls);
		int squeeze = 0;
		
		jive_cpureg_bitmask allowed_regs;
		jive_cpureg_bitmask_init(&allowed_regs);
		unsigned int allowed_regs_count = cand->regcls->nregs;
		size_t k;
		for(k=0; k<cand->regcls->nregs; k++)
			jive_cpureg_bitmask_set(&allowed_regs, &cand->regcls->regs[k]);
		
		for(k=0; k<interfere.nitems; k++) {
			jive_reg_candidate * other = interfere.items[k].value;
			
			if (other->reg) {
				if (jive_cpureg_bitmask_isset(&allowed_regs, other->reg)) {
					jive_cpureg_bitmask_clear(&allowed_regs, other->reg);
					allowed_regs_count --;
				}
			} else if (jive_cpureg_class_intersect(cand->regcls, other->regcls))
				squeeze --;
		}
		
		squeeze += allowed_regs_count;
		
		DEBUG_ASSERT(jive_cpureg_bitmask_equals(&allowed_regs, &cand->allowed_regs));
		DEBUG_ASSERT(allowed_regs_count == cand->allowed_regs_count);
		DEBUG_ASSERT(squeeze == cand->squeeze);
	}
}

static void
jive_reg_candidate_reset(jive_reg_candidate * cand)
{
	jive_interference_set_clear(&cand->interference);
	cand->squeeze = 0;
	jive_cpureg_bitmask_init(&cand->allowed_regs);
	cand->allowed_regs_count = cand->regcls->nregs;
	cand->squeeze = cand->allowed_regs_count;
	size_t k;
	for(k=0; k<cand->regcls->nregs; k++)
		jive_cpureg_bitmask_set(&cand->allowed_regs, &cand->regcls->regs[k]);
}

jive_reg_candidate *
jive_interference_graph_map_value(jive_interference_graph * igraph, jive_value * value)
{
	size_t n;
	jive_reg_candidate * cand;
	for(n=0; n<igraph->map.nitems; n++) {
		cand = igraph->map.items[n];
		if (cand->value == value) return cand;
	}
	
	return 0;
}

static jive_reg_candidate *
jive_reg_candidate_lookup(jive_interference_graph * igraph, jive_value * value)
{
	jive_reg_candidate * cand = jive_interference_graph_map_value(igraph, value);
	if (cand) return cand;
	
	cand = jive_malloc(igraph->graph, sizeof(*cand));
	cand->value = value;
	cand->igraph = igraph;
	cand->reg = 0;
	cand->regcls = jive_value_get_cpureg_class_shared(value);
	jive_reg_candidate_reset(cand);
	
	jive_interference_set_init(&cand->interference);
	
	/* FIXME: should really be a priority heap */
	jive_interference_set_add(&igraph->cand, cand);
	
	/* FIXME: should be a map */
	jive_cand_set_add(&igraph->map, cand);
	//cand->index = 0;
	
	return cand;
}

static void
record_interference_edge(jive_interference_graph * igraph, jive_value * _first, jive_value * _second)
{
	if (_first == _second) return;
	jive_reg_candidate * first = jive_reg_candidate_lookup(igraph, _first);
	jive_reg_candidate * second = jive_reg_candidate_lookup(igraph, _second);
	
	size_t a = jive_interference_set_add(&first->interference, second);
	size_t b = jive_interference_set_add(&second->interference, first);
	
	DEBUG_ASSERT( a == b );
	if (a) return;
	(void)b;
	
	if (jive_cpureg_class_intersect(first->regcls, second->regcls)) {
		first->squeeze --;
		second->squeeze --;
	}
}

static void
remove_interference_edge(jive_interference_graph * igraph, jive_value * _first, jive_value * _second)
{
	jive_reg_candidate * first = jive_reg_candidate_lookup(igraph, _first);
	jive_reg_candidate * second = jive_reg_candidate_lookup(igraph, _second);
	
	size_t a = jive_interference_set_remove(&first->interference, second);
	size_t b = jive_interference_set_remove(&second->interference, first);
	
	DEBUG_ASSERT( a!=0 && b!=0 );
	
	DEBUG_ASSERT( a == b );
	
	if ( a > 1 ) return;
	(void)b;
	
	if (jive_cpureg_class_intersect(first->regcls, second->regcls)) {
		first->squeeze++;
		second->squeeze++;
	}
}

static void
record_instruction_interference(jive_interference_graph * igraph, jive_instruction * instr)
{
	jive_value_multiset * active = &instr->active_before;
	size_t j, k;
	for(j=0; j<active->nitems; j++) {
		for(k=j+1; k<active->nitems; k++) {
			record_interference_edge(igraph, active->items[j].value, active->items[k].value);
		}
	}
}

static jive_interference_graph *
build_interference_graph(
	jive_graph * graph,
	jive_instruction_sequence * seq,
	const jive_machine * machine)
{
	jive_interference_graph * igraph = jive_malloc(graph, sizeof(*igraph));
	
	igraph->graph = graph;
	igraph->machine = machine;
	igraph->seq = * seq;
	jive_interference_set_init(&igraph->cand);
	jive_cand_set_init(&igraph->map);
	
	jive_instruction * instr = seq->first;
	
	while(instr) {
		record_instruction_interference(igraph, instr);
		instr = instr->next;
	}
	
	return igraph;
}

static bool
can_assign(jive_reg_candidate * cand, jive_cpureg_t reg, const jive_machine * machine)
{
	size_t n;
	if (! jive_cpureg_bitmask_isset(&cand->allowed_regs, reg)) return false;
	
	for(n=0; n<cand->interference.nitems; n++) {
		jive_reg_candidate * other = cand->interference.items[n].value;
		
		if (other->reg == reg) return false;
		if (other->reg) continue;
		
		if (jive_cpureg_bitmask_isset(&other->allowed_regs, reg) && (other->allowed_regs_count == 1))
			return false;
	}
	return true;
}

static jive_cpureg_t
select_register(jive_reg_candidate * cand, const jive_machine * machine)
{
	jive_cpureg_class_t regcls = jive_value_get_cpureg_class_shared(cand->value);	
	size_t n;
	for(n=0; n<regcls->nregs; n++) {
		if (can_assign(cand, &regcls->regs[n], machine))
			return &regcls->regs[n];
	}
	
	return 0;
}

/* adds "value" to active set before "instr" */
static void
mark_active_before(jive_interference_graph * igraph, jive_instruction * instr,
	jive_reg_candidate * cand)
{
	jive_value * value = cand->value;
	if (!jive_value_multiset_contains(&instr->active_before, value)) {
		size_t n;
		for(n=0; n<instr->active_before.nitems; n++)
			record_interference_edge(igraph, instr->active_before.items[n].value, value);
		
		jive_value_multiset_add(&instr->active_before, value);
		jive_regcls_count_add(instr->use_count_before, cand->regcls);
		jive_regcls_count_add(instr->prev->use_count_after, cand->regcls);
	}
}

static void
unmark_active_before(jive_interference_graph * igraph, jive_instruction * instr,
	jive_reg_candidate * cand)
{
	jive_value * value = cand->value;
	if (jive_value_multiset_remove_all(&instr->active_before, value)) {
		size_t n;
		for(n=0; n<instr->active_before.nitems; n++)
			remove_interference_edge(igraph, instr->active_before.items[n].value, value);
		
		jive_regcls_count_sub(instr->use_count_before, cand->regcls);
		jive_regcls_count_sub(instr->prev->use_count_after, cand->regcls);
	}
}

static bool
jive_instruction_uses(jive_instruction * instr, jive_value * value)
{
	jive_operand * operand = jive_node_iterate_operands((jive_node *) instr);
	while(operand) {
		if (operand->value == value) return true;
		operand = operand->next;
	}
	return false;
}

static void
do_spill(jive_interference_graph * igraph, jive_reg_candidate * cand,
	const jive_machine * machine, jive_stackframe * frame,
	jive_instruction * defining,
	jive_instruction * last_user_nospill,
	jive_instruction * last_user)
{
	jive_value * value = cand->value;
	
	/* make sure we have found the last user */
	while(last_user_nospill != defining && !jive_instruction_uses(last_user_nospill, value)) {
		unmark_active_before(igraph, last_user_nospill, cand);
		last_user_nospill = last_user_nospill->prev;
	}
	/* find first user after split */
	jive_instruction * first_user_after_split = last_user_nospill->next;
	while(!jive_instruction_uses(first_user_after_split, value))
		first_user_after_split = first_user_after_split->next;
	
	DEBUG_PRINTF("spill %p: def=%p, last_user_nospill=%p, first_after_split=%p, last=%p\n", value, defining, last_user_nospill, first_user_after_split, last_user);
	
	/* add spill instruction immediately after defining instruction
	FIXME: avoid spilling twice */
	jive_instruction * spill = (jive_instruction *) jive_regalloc_spill(value, machine, frame);
	jive_regalloc_add_instruction_between(spill, defining, defining->next, machine);
	record_instruction_interference(igraph, spill);
	
	mark_active_before(igraph, spill, cand);
	
	jive_instruction * instr = first_user_after_split;
	
	/* create restore instruction */
	jive_value * restored = jive_regalloc_restore((jive_node *)spill, machine, frame);
	jive_cpureg_class_t regcls = jive_value_get_cpureg_class(restored);
	/* divert inputs of all users after split & find common register
	class */
	while(1) {
		jive_input_edge_iterator i;
		JIVE_ITERATE_INPUTS(i, (jive_node *) instr) {
			if (i->origin.port != cand->value) continue;
			jive_edge_divert_origin(i, restored);
			regcls = jive_cpureg_class_intersect(regcls, jive_operand_get_cpureg_class(i->target.port));
		}
		
		if (instr == last_user) break;
		instr = instr->next;
	}
	
	jive_value_set_cpureg_class_shared(restored, regcls);
	jive_reg_candidate * restored_cand = jive_reg_candidate_lookup(igraph, restored);
	jive_instruction * restore = (jive_instruction *) restored->node;
	jive_state_edge_create((jive_node *) spill, (jive_node *) restore);
	
	/* insert it immediately before first user after live-range split */
	instr = first_user_after_split;
	jive_regalloc_add_instruction_between(restore, instr->prev, instr, machine);
	record_instruction_interference(igraph, restore);
	
	/* insert register into active set, compute intersections */
	while(instr) {
		mark_active_before(igraph, instr, restored_cand);
		if (instr == last_user) break;
		instr = instr->next;
	}

#if 1 /* FIXME: make conditional on debug */
	//jive_igraph_view(igraph);
	
	jive_interference_graph_validate(igraph);
#endif
	
	instr = first_user_after_split;
	
}

static void
do_assign(jive_interference_graph * igraph, jive_reg_candidate * cand, const jive_machine * machine, jive_stackframe * frame)
{
	jive_value * value = cand->value;
	
	/* determine defining instruction and "last user" of this value;
	these determine the "live range" of the value */
	jive_instruction * definer, * last_user;
	definer = (jive_instruction *) value->node;
	last_user = definer;
	while(last_user->next && jive_value_multiset_contains(&last_user->next->active_before, value)) {
		last_user = last_user->next;
	}
	jive_instruction * last_user_nospill = last_user;
	
	/* select register, possibly shortening live range until it fits */
	jive_cpureg_t reg = select_register(cand, machine);
	while(!reg) {
		DEBUG_ASSERT(last_user_nospill != definer);
		
		DEBUG_PRINTF("unmark active %p before %p\n", cand->value, last_user_nospill);
		unmark_active_before(igraph, last_user_nospill, cand);
		last_user_nospill = last_user_nospill->prev;
		reg = select_register(cand, machine);
	}
	DEBUG_PRINTF("choose %s\n", reg->name);
	if (last_user_nospill != last_user) {
		do_spill(igraph, cand, machine, frame, definer, last_user_nospill, last_user);
	}
	
	/* assign register */
	jive_value_set_cpureg(cand->value, reg);
	cand->reg = reg;
	
	/* update availability states in neighbouring registers */
	size_t n;
	for(n=0; n<cand->interference.nitems; n++) {
		jive_reg_candidate * other = cand->interference.items[n].value;
		other->squeeze ++;
		if (jive_cpureg_bitmask_isset(&other->allowed_regs, reg) ) {
			jive_cpureg_bitmask_clear(&other->allowed_regs, reg);
			other->allowed_regs_count --;
			other->squeeze --;
			DEBUG_ASSERT(other->allowed_regs_count);
		}
	}
	
	DEBUG_PRINTF("assign %p %p %s\n", cand, reg, reg->name);
}

static bool
jive_cpureg_available(const jive_value_multiset * active, jive_cpureg_t reg)
{
	size_t n;
	for(n=0; n<active->nitems; n++)
		if (jive_value_get_cpureg(active->items[n].value) == reg)
			return false;
	return true;
}

static const jive_cpureg *
jive_cpureg_pick(const jive_value_multiset * active, jive_cpureg_class_t regcls)
{
	size_t n;
	for(n=0; n<regcls->nregs; n++)
		if (jive_cpureg_available(active, &regcls->regs[n]))
			return &regcls->regs[n];
	return 0;
}

static void
jive_two_operand_fixup(
	jive_instruction_sequence * seq,
	jive_interference_graph * igraph,
	const jive_machine * machine,
	jive_stackframe * stackframe)
{
	jive_instruction * instr = seq->first;
	for(;instr; instr = instr->next) {
		const jive_instruction_class * icls;
		icls = jive_instruction_get_class((jive_node *)instr);
		
		if ( ! (icls->flags & jive_instruction_write_input) )
			continue;
		jive_cpureg_t in0 = jive_instruction_inputreg(instr, 0);
		jive_cpureg_t out0 = jive_instruction_outputreg(instr, 0);
		
		if (in0 == out0) continue;
		
		if (icls->flags & jive_instruction_commutative) {
			jive_cpureg_t in1 = jive_instruction_inputreg(instr, 1);
			if (out0 == in1) {
				jive_instruction_swap_inputs((jive_node *)instr);
				continue;
			}
		}
		
		/* result would not be written into the desired target
		register; fixup by issuing (one or more) transfer instructions */
		
		jive_cpureg_t pre_transfer_reg = 0;
		if (jive_cpureg_available(&instr->active_before, out0))
			pre_transfer_reg = out0;
		else if (!jive_cpureg_available(&instr->next->active_before, in0))
			pre_transfer_reg = jive_cpureg_pick(&instr->active_before, icls->inregs[0]);
		
		DEBUG_PRINTF("fixup %p\n", instr);
		
		if (pre_transfer_reg) {
			jive_value * value = (jive_value *) (jive_instruction_input(instr, 0)->value);
			jive_value * out;
			jive_instruction * xfer = machine->transfer(machine, value, &out);
			jive_regalloc_add_instruction_between(xfer, instr->prev, instr, machine);
			
			jive_value_set_cpureg(out, pre_transfer_reg);
			jive_reg_candidate * cand = jive_reg_candidate_lookup(igraph, out);
			cand->reg = pre_transfer_reg;
			
			in0 = pre_transfer_reg;
			
			jive_input_edge_iterator i;
			i = jive_node_iterate_inputs((jive_node *) instr);
			while(i) {
				jive_edge * e = i;
				i = jive_input_edge_iterator_next(i);
				if (e->target.port != (jive_operand *)instr->inregs)
					continue;
				jive_edge_divert_origin(e, out);
				break;
			}
		}
		
		if (in0 != out0) {
			jive_value * value = jive_instruction_output(instr, 0);
			jive_value * out;
			jive_instruction * xfer = machine->transfer(machine, value, &out);
			jive_regalloc_add_instruction_between(xfer, instr, instr->next, machine);
			
			jive_value_set_cpureg(out, out0);
			jive_reg_candidate * cand = jive_reg_candidate_lookup(igraph, out);
			cand->reg = out0;
			
			jive_value_set_cpureg(value, in0);
			cand = jive_reg_candidate_lookup(igraph, value);
			cand->reg = in0;
			
			jive_output_edge_iterator i;
			
			i = jive_node_iterate_outputs((jive_node *) instr);
			while(i) {
				jive_edge * e = i;
				i = jive_output_edge_iterator_next(i);
				if (e->origin.port != value) continue;
				if (e->target.node == (jive_node *) xfer) continue;
				jive_edge_divert_origin(e, out);
			}
		}
	}
}

void
jive_regalloc_assign(
	jive_instruction_sequence * seq,
	const jive_machine * machine,
	jive_stackframe * frame)
{
	jive_interference_graph * igraph = build_interference_graph(seq->first->base.graph, seq, machine);
	
	jive_interference_graph_validate(igraph);
	
	while(igraph->cand.nitems) {
		jive_reg_candidate * cand = igraph->cand.items[0].value;
		size_t n;
		for(n=1; n<igraph->cand.nitems; n++) {
			if (igraph->cand.items[n].value->squeeze < cand->squeeze)
				cand = igraph->cand.items[n].value;
		}
		
		jive_interference_set_remove_all(&igraph->cand, cand);
		do_assign(igraph, cand, machine, frame);
	}
	
	jive_two_operand_fixup(seq, igraph, machine, frame);
	
	/* FIXME: the "correct" thing to do is to add dependency edges
	only for register reuses; this currently fully sequentializes
	the graph, which is a bit too strict */
	jive_instruction * instr = seq->first->next;
	while(instr) {
		jive_state_edge_create((jive_node *)instr->prev, (jive_node *)instr);
		instr = instr->next;
	}
}
