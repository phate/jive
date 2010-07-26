#ifndef JIVE_VSDG_GRAPH_PRIVATE_H
#define JIVE_VSDG_GRAPH_PRIVATE_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/types.h>

void
jive_graph_notify_input_change(jive_graph * graph, jive_input * input, jive_output * old_origin, jive_output * new_origin);

void
jive_graph_notify_node_create(jive_graph * graph, jive_node * node);

void
jive_graph_notify_input_create(jive_graph * graph, jive_input * input);

void
jive_graph_notify_output_create(jive_graph * graph, jive_output * output);

#endif
