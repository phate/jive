/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/gate.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/resource.h>
#include <jive/rvsdg/type.h>

namespace jive {

gate::gate(
	jive::graph * graph,
	const std::string & name,
	const jive::type & type)
: name_(name)
, graph_(graph)
, rescls_(&jive_root_resource_class)
, type_(type.copy())
{
	inputs.first = inputs.last = nullptr;
	outputs.first = outputs.last = nullptr;
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
	graph_gate_list.prev = graph_gate_list.next = nullptr;

	JIVE_LIST_PUSH_BACK(graph->gates, this, graph_gate_list);
}

gate::~gate() noexcept
{
	JIVE_DEBUG_ASSERT(inputs.first == nullptr && inputs.last == nullptr);
	JIVE_DEBUG_ASSERT(outputs.first == nullptr && outputs.last == nullptr);
	JIVE_DEBUG_ASSERT(interference_.empty());

	JIVE_LIST_REMOVE(graph()->gates, this, graph_gate_list);
}

void
gate::add_interference(jive::gate * other)
{
	JIVE_DEBUG_ASSERT(this != other);

	auto it = interference_.find(other);
	if (it != interference_.end()) {
		it->whole->count++;
		return;
	}

	auto i = new jive_gate_interference;
	i->first.gate = this;
	i->first.whole = i;
	i->second.gate = other;
	i->second.whole = i;
	i->count = 1;

	interference_.insert(&i->second);
	other->interference_.insert(&i->first);

	graph()->on_gate_interference_add(this, other);
}

void
gate::clear_interferences()
{
	while (!interference_.empty()) {
		auto i = interference_.begin()->whole;
		i->count = i->count-1;
		if (i->count == 0) {
			i->first.gate->interference_.erase(&i->second);
			i->second.gate->interference_.erase(&i->first);
			graph()->on_gate_interference_remove(this, i->second.gate);
			delete i;
		}
	}
}

}
