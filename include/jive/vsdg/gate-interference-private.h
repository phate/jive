/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_GATE_INTERFERENCE_PRIVATE_H
#define JIVE_GATE_INTERFERENCE_PRIVATE_H

#include <stdlib.h>

#include <jive/context.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/gate-interference.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/notifiers.h>

struct jive_gate_interference_part {
	jive_gate * gate;
	struct {
		jive_gate_interference_part * prev;
		jive_gate_interference_part * next;
	} chain;
	jive_gate_interference * whole;
};

struct jive_gate_interference {
	jive_gate_interference_part first;
	jive_gate_interference_part second;
	size_t count;
};

JIVE_DEFINE_HASH_TYPE(jive_gate_interference_hash, jive_gate_interference_part, struct jive_gate *, gate, chain);

jive_gate_interference *
jive_gate_interference_create(struct jive_gate * first, struct jive_gate * second);

void
jive_gate_interference_destroy(jive_gate_interference * self);

static inline void
jive_gate_interference_add(jive_graph * graph, jive_gate * first, jive_gate * second)
{
	jive_gate_interference * i;
	jive_gate_interference_part * part = jive_gate_interference_hash_lookup(&first->interference, second);
	if (part) {
		i = part->whole;
		i->count ++;
	} else {
		i = jive_gate_interference_create(first, second);
		i->count = 1;
		jive_gate_notifier_slot_call(&graph->on_gate_interference_add, first, second);
	}
}

static inline void
jive_gate_interference_remove(jive_graph * graph, jive_gate * first, jive_gate * second)
{
	jive_gate_interference * i;
	jive_gate_interference_part * part = jive_gate_interference_hash_lookup(&first->interference, second);
	i = part->whole;
	size_t count = -- (i->count);
	if (!count) {
		jive_gate_interference_destroy(i);
		jive_gate_notifier_slot_call(&graph->on_gate_interference_remove, first, second);
	}
}

#endif
