#ifndef JIVE_VSDG_GRAPH_PRIVATE_H
#define JIVE_VSDG_GRAPH_PRIVATE_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>

static inline void
jive_graph_notify_node_create(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_create, node);
}

static inline void
jive_graph_notify_node_destroy(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_create, node);
}

static inline void
jive_graph_notify_input_create(jive_graph * graph, jive_input * input)
{
	jive_input_notifier_slot_call(&graph->on_input_create, input);
}

static inline void
jive_graph_notify_input_change(jive_graph * graph, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_input_change_notifier_slot_call(&graph->on_input_change, input, old_origin, new_origin);
}

static inline void
jive_graph_notify_input_destroy(jive_graph * graph, jive_input * input)
{
	jive_input_notifier_slot_call(&graph->on_input_destroy, input);
}

static inline void
jive_graph_notify_output_create(jive_graph * graph, jive_output * output)
{
	jive_output_notifier_slot_call(&graph->on_output_create, output);
}

static inline void
jive_graph_notify_output_destroy(jive_graph * graph, jive_output * output)
{
	jive_output_notifier_slot_call(&graph->on_output_destroy, output);
}

#endif
