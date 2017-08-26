/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/gate.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/resource.h>

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

void
jive_gate_interference_add(jive::graph * graph, jive::gate * first, jive::gate * second)
{
	auto iter = first->interference.find(second);
	if (iter != first->interference.end()) {
		iter->whole->count++;
	} else {
		jive_gate_interference * i = jive_gate_interference_create(first, second);
		i->count = 1;
		graph->on_gate_interference_add(first, second);
	}
}

void
jive_gate_interference_remove(jive::graph * graph, jive::gate * first, jive::gate * second)
{
	jive_gate_interference * i = first->interference.find(second)->whole;
	size_t count = -- (i->count);
	if (!count) {
		jive_gate_interference_destroy(i);
		graph->on_gate_interference_remove(first, second);
	}
}

namespace jive {

gate::gate(
	jive::graph * graph,
	const std::string & name,
	const jive::base::type & type)
: name_(name)
, graph_(graph)
, rescls_(&jive_root_resource_class)
, type_(type.copy())
{
	inputs.first = inputs.last = nullptr;
	outputs.first = outputs.last = nullptr;
	may_spill = true;
	graph_gate_list.prev = graph_gate_list.next = nullptr;

	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

gate::gate(
	jive::graph * graph,
	const std::string & name,
	const resource_class * rescls)
: name_(name)
, graph_(graph)
, rescls_(rescls)
, type_(rescls->type().copy())
{
	inputs.first = inputs.last = nullptr;
	outputs.first = outputs.last = nullptr;
	may_spill = true;
	graph_gate_list.prev = graph_gate_list.next = nullptr;

	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

gate::~gate() noexcept
{
	JIVE_DEBUG_ASSERT(inputs.first == nullptr && inputs.last == nullptr);
	JIVE_DEBUG_ASSERT(outputs.first == nullptr && outputs.last == nullptr);

	JIVE_LIST_REMOVE(graph()->gates, this, graph_gate_list);
}

}
