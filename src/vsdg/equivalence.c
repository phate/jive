/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/equivalence.h>

#include <jive/context.h>
#include <jive/vsdg/node.h>
#include <jive/util/hash.h>

typedef struct jive_node_equiv_entry jive_node_equiv_entry;
struct jive_node_equiv_entry {
	const jive_node * first;
	const jive_node * second;
	bool pending;
	struct {
		jive_node_equiv_entry * prev;
		jive_node_equiv_entry * next;
	} hash_chain;
	struct {
		jive_node_equiv_entry * prev;
		jive_node_equiv_entry * next;
	} pending_chain;
};

typedef struct jive_node_equiv_mapping jive_node_equiv_mapping;

JIVE_DECLARE_HASH_TYPE(jive_node_equiv_mapping, jive_node_equiv_entry, const jive_node *, first, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_node_equiv_mapping, jive_node_equiv_entry, const jive_node *, first, hash_chain);

typedef struct jive_gate_equiv_entry jive_gate_equiv_entry;
struct jive_gate_equiv_entry {
	const jive_gate * first;
	const jive_gate * second;
	bool pending;
	struct {
		jive_gate_equiv_entry * prev;
		jive_gate_equiv_entry * next;
	} hash_chain;
	struct {
		jive_gate_equiv_entry * prev;
		jive_gate_equiv_entry * next;
	} pending_chain;
};

typedef struct jive_gate_equiv_mapping jive_gate_equiv_mapping;

JIVE_DECLARE_HASH_TYPE(jive_gate_equiv_mapping, jive_gate_equiv_entry, const jive_gate *, first, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_gate_equiv_mapping, jive_gate_equiv_entry, const jive_gate *, first, hash_chain);

typedef struct jive_equiv_state jive_equiv_state;
struct jive_equiv_state {
	jive_node_equiv_mapping node_mapping;
	struct {
		jive_node_equiv_entry * first;
		jive_node_equiv_entry * last;
	} pending;
};

static void
jive_equiv_state_init(jive_equiv_state * self, jive_context * context)
{
	jive_node_equiv_mapping_init(&self->node_mapping, context);
	self->pending.first = self->pending.last = 0;
}

static void
jive_equiv_state_fini(jive_equiv_state * self)
{
	jive_context * context = self->node_mapping.context;
	
	struct jive_node_equiv_mapping_iterator i;
	i = jive_node_equiv_mapping_begin(&self->node_mapping);
	while (i.entry) {
		jive_node_equiv_entry * entry = i.entry;
		jive_node_equiv_mapping_iterator_next(&i);
		jive_context_free(context, entry);
	}
	jive_node_equiv_mapping_fini(&self->node_mapping);
}

static jive_node_equiv_entry *
jive_equiv_state_lookup(jive_equiv_state * self, const jive_node * node)
{
	jive_node_equiv_entry * entry;
	entry = jive_node_equiv_mapping_lookup(&self->node_mapping, node);
	if (!entry) {
		jive_context * context = self->node_mapping.context;
		entry = jive_context_malloc(context, sizeof(*entry));
		entry->first = node;
		entry->second = 0;
		entry->pending = true;
		jive_node_equiv_mapping_insert(&self->node_mapping, entry);
		JIVE_LIST_PUSH_BACK(self->pending, entry, pending_chain);
	}
	
	return entry;
}

static void
jive_equiv_state_mark_verified(jive_equiv_state * self, jive_node_equiv_entry * entry)
{
	JIVE_DEBUG_ASSERT(entry->pending);
	entry->pending = false;
	JIVE_LIST_REMOVE(self->pending, entry, pending_chain);
}

static bool
jive_equiv_state_check_node(jive_equiv_state * self, const jive_node * n1, const jive_node * n2)
{
	if (n1->noutputs != n2->noutputs)
		return false;
	if (n1->ninputs != n2->ninputs)
		return false;
	if (n1->noperands != n2->noperands)
		return false;
	if (n1->class_ != n2->class_)
		return false;
	if (!jive_node_match_attrs(n1, jive_node_get_attrs(n2)))
		return false;
	
	/* FIXME: verify gates */
	
	size_t n = 0;
	for (n = 0; n < n1->ninputs; ++n) {
		jive::output * o1 = n1->inputs[n]->origin();
		jive::output * o2 = n2->inputs[n]->origin();
		
		jive_node_equiv_entry * entry = jive_equiv_state_lookup(self, o1->node());
		if (entry->second && entry->second != o2->node())
			return false;
		entry->second = o2->node();
	}
	
	return true;
}

bool
jive_graphs_equivalent(
	jive_graph * graph1, jive_graph * graph2,
	size_t ncheck, jive_node * const check1[], jive_node * const check2[],
	size_t nassumed, jive_node * const ass1[], jive_node * const ass2[])
{
	jive_equiv_state state;
	jive_equiv_state_init(&state, graph1->context);
	
	bool satisfied = true;
	size_t n;
	
	for (n = 0; n < nassumed; ++n) {
		jive_node_equiv_entry * entry = jive_equiv_state_lookup(&state, ass1[n]);
		if (entry->second != 0 && entry->second != ass2[n]) {
			satisfied = false;
			break;
		}
		entry->second = ass2[n];
		jive_equiv_state_mark_verified(&state, entry);
	}
	
	for (n = 0; n < ncheck; ++n) {
		jive_node_equiv_entry * entry = jive_equiv_state_lookup(&state, check1[n]);
		if (entry->second != 0 && entry->second != check2[n]) {
			satisfied = false;
			break;
		}
		entry->second = check2[n];
	}
	
	while (satisfied && state.pending.first) {
		jive_node_equiv_entry * entry = state.pending.first;
		
		satisfied = jive_equiv_state_check_node(&state, entry->first, entry->second);
		jive_equiv_state_mark_verified(&state, entry);
	}
	
	jive_equiv_state_fini(&state);
	
	return satisfied;
}
