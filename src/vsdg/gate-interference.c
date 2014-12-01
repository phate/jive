/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph.h>

jive_gate_interference *
jive_gate_interference_create(jive::gate * first, jive::gate * second)
{
	jive_gate_interference * i = new jive_gate_interference;
	i->first.gate = first;
	i->first.whole = i;
	i->second.gate = second;
	i->second.whole = i;
	i->count = 0;
	
	first->interference.insert(&i->second);
	second->interference.insert(&i->first);

	return i;
}

void
jive_gate_interference_destroy(jive_gate_interference * self)
{
	self->first.gate->interference.erase(&self->second);
	self->second.gate->interference.erase(&self->first);
	delete self;
}
