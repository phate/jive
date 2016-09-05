/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <unordered_map>

#include <jive/vsdg/equivalence.h>

#include <jive/vsdg/node.h>

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

typedef struct jive_equiv_state jive_equiv_state;
struct jive_equiv_state {
	std::unordered_map<const jive_node *, jive_node_equiv_entry*> node_mapping;
	struct {
		jive_node_equiv_entry * first;
		jive_node_equiv_entry * last;
	} pending;
};

static void
jive_equiv_state_init(jive_equiv_state * self)
{
	self->pending.first = self->pending.last = 0;
}

static void
jive_equiv_state_fini(jive_equiv_state * self)
{
	while (self->node_mapping.size()) {
		jive_node_equiv_entry * entry = self->node_mapping.begin()->second;
		self->node_mapping.erase(self->node_mapping.begin());
		delete entry;
	}
}

static jive_node_equiv_entry *
jive_equiv_state_lookup(jive_equiv_state * self, const jive_node * node)
{
	jive_node_equiv_entry * entry;
	auto i = self->node_mapping.find(node);
	if (i == self->node_mapping.end()) {
		entry = new jive_node_equiv_entry;
		entry->first = node;
		entry->second = 0;
		entry->pending = true;
		self->node_mapping[node] = entry;
		JIVE_LIST_PUSH_BACK(self->pending, entry, pending_chain);
	} else
		entry = i->second;

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
	if (n1->noutputs != n2->noutputs) {
		return false;
	}
	if (n1->ninputs != n2->ninputs) {
		return false;
	}
	if (n1->noperands() != n2->noperands()) {
		return false;
	}
	if (n1->operation() != n2->operation()) {
		return false;
	}
	
	/* FIXME: verify gates */
	
	size_t n = 0;
	for (n = 0; n < n1->ninputs; ++n) {
		jive::output * o1 = n1->input(n)->origin();
		jive::output * o2 = n2->input(n)->origin();
		
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
	jive_equiv_state_init(&state);
	
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
