/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gate-interference-private.h>

#include <jive/vsdg/graph.h>
#include <jive/util/list.h>

jive_gate_interference *
jive_gate_interference_create(jive_gate * first, jive_gate * second)
{
	jive_gate_interference * i = jive_context_malloc(first->graph->context, sizeof(*i));
	i->first.gate = first;
	i->first.whole = i;
	i->second.gate = second;
	i->second.whole = i;
	i->count = 0;
	
	jive_gate_interference_hash_insert(&first->interference, &i->second);
	jive_gate_interference_hash_insert(&second->interference, &i->first);
	
	return i;
}

void
jive_gate_interference_destroy(jive_gate_interference * self)
{
	jive_gate_interference_hash_remove(&self->first.gate->interference, &self->second);
	jive_gate_interference_hash_remove(&self->second.gate->interference, &self->first);
	jive_context_free(self->first.gate->graph->context, self);
}
