/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GATE_INTERFERENCE_H
#define JIVE_VSDG_GATE_INTERFERENCE_H

#include <jive/util/hash.h>

typedef struct jive_gate_interference jive_gate_interference;
typedef struct jive_gate_interference_hash jive_gate_interference_hash;
typedef struct jive_gate_interference_part jive_gate_interference_part;

JIVE_DECLARE_HASH_TYPE(jive_gate_interference_hash, jive_gate_interference_part, jive::gate *, gate,
	chain);

#endif
