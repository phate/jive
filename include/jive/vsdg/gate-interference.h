/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GATE_INTERFERENCE_H
#define JIVE_VSDG_GATE_INTERFERENCE_H

#include <jive/util/intrusive-hash.h>

namespace jive{
	class gate;
}

struct jive_gate_interference;

typedef struct jive_gate_interference_part jive_gate_interference_part;
struct jive_gate_interference_part {
	jive::gate * gate;
	jive_gate_interference * whole;
private:
	jive::detail::intrusive_hash_anchor<jive_gate_interference_part> hash_chain;
public:
	typedef jive::detail::intrusive_hash_accessor<
		jive::gate*,
		jive_gate_interference_part,
		&jive_gate_interference_part::gate,
		&jive_gate_interference_part::hash_chain
	> hash_chain_accessor;
};

typedef struct jive_gate_interference jive_gate_interference;
struct jive_gate_interference {
	jive_gate_interference_part first;
	jive_gate_interference_part second;
	size_t count;
};

typedef jive::detail::intrusive_hash<
	const jive::gate *,
	jive_gate_interference_part,
	jive_gate_interference_part::hash_chain_accessor
> jive_gate_interference_hash;

#endif
