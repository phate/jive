/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/basetype.h>
#include <jive/vsdg/gate.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/resource.h>

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
