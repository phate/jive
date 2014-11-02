/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GATE_INTERFERENCE_PRIVATE_H
#define JIVE_VSDG_GATE_INTERFERENCE_PRIVATE_H

#include <stdlib.h>

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/gate-interference.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/notifiers.h>

jive_gate_interference *
jive_gate_interference_create(jive::gate * first, jive::gate * second);

void
jive_gate_interference_destroy(jive_gate_interference * self);

static inline void
jive_gate_interference_add(jive_graph * graph, jive::gate * first, jive::gate * second)
{
	auto iter = first->interference.find(second);
	if (iter != first->interference.end()) {
		iter->whole->count++;
	} else {
		jive_gate_interference * i = jive_gate_interference_create(first, second);
		i->count = 1;
		jive_gate_notifier_slot_call(&graph->on_gate_interference_add, first, second);
	}
}

static inline void
jive_gate_interference_remove(jive_graph * graph, jive::gate * first, jive::gate * second)
{
	jive_gate_interference * i = first->interference.find(second)->whole;
	size_t count = -- (i->count);
	if (!count) {
		jive_gate_interference_destroy(i);
		jive_gate_notifier_slot_call(&graph->on_gate_interference_remove, first, second);
	}
}

#endif
