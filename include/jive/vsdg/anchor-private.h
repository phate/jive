/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHOR_PRIVATE_H
#define JIVE_VSDG_ANCHOR_PRIVATE_H

#include <jive/vsdg/anchor.h>

/* inheritable anchor node member functions */

struct jive::node_normal_form *
jive_anchor_node_get_default_normal_form_(const struct jive_node_class * cls,
	struct jive::node_normal_form * parent, struct jive_graph * graph);

#endif
