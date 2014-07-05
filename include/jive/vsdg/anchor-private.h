/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_ANCHOR_PRIVATE_H
#define JIVE_VSDG_ANCHOR_PRIVATE_H

#include <jive/vsdg/anchor.h>

/* inheritable anchor node member functions */

struct jive_node_normal_form *
jive_anchor_node_get_default_normal_form_(const struct jive_node_class * cls,
	struct jive_node_normal_form * parent, struct jive_graph * graph);

/* inheritable anchor node normal form member functions */

void
jive_anchor_node_normal_form_init_(jive_anchor_node_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_, struct jive_graph * graph);

void
jive_anchor_node_normal_form_set_reducible_(jive_anchor_node_normal_form * self_, bool reducible);

#endif
