/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_CONTROL_H
#define JIVE_VSDG_CONTROL_H

#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/node.h>

struct jive_graph;
struct jive_region;

struct jive_node *
jive_control_false_create(struct jive_region * region);

struct jive_output *
jive_control_false(struct jive_graph * graph);

struct jive_node *
jive_control_true_create(struct jive_region * region);

struct jive_output *
jive_control_true(struct jive_graph * graph);

extern const jive_node_class JIVE_CONTROL_FALSE_NODE;
extern const jive_node_class JIVE_CONTROL_TRUE_NODE;

#endif
