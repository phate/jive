/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_OPERATORS_H
#define JIVE_VSDG_OPERATORS_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/operators/unary.h>
#include <jive/vsdg/operators/binary.h>

typedef struct {
	size_t count;
	const jive_node_class * const * classes;
} jive_node_class_vector;

#endif
