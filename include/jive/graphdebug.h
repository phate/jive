#ifndef JIVE_GRAPHDEBUG_H
#define JIVE_GRAPHDEBUG_H

#include <jive/graph.h>

void
jive_graph_dump(jive_graph * graph, int fd);

void
jive_graph_view(jive_graph * graph);

struct jive_interference_graph;

void
jive_igraph_dump(struct jive_interference_graph * graph, int fd);

void
jive_igraph_view(struct jive_interference_graph * graph);

#endif
