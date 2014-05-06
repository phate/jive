/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTCONSTANT_H
#define JIVE_TYPES_FLOAT_FLTCONSTANT_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_node_class JIVE_FLTCONSTANT_NODE;

namespace jive {
namespace flt {

class constant_operation final : public jive::nullary_operation {
public:
	inline constexpr constant_operation(uint32_t value) noexcept
		: value_(value) {}
	inline constexpr constant_operation(const constant_operation& other) noexcept
		: value_(other.value_) {}
	
	uint32_t value() const noexcept { return value_; }
private:
	uint32_t value_;
};

}
}

typedef jive::operation_node<jive::flt::constant_operation> jive_fltconstant_node;

jive_output *
jive_fltconstant(struct jive_graph * graph, uint32_t value);

JIVE_EXPORTED_INLINE jive_output *
jive_fltconstant_float(struct jive_graph * graph, float value)
{
	union u {
		uint32_t i;
		float f;
	};

	union u c;
	c.f = value;

	return jive_fltconstant(graph, c.i);
}

JIVE_EXPORTED_INLINE jive_fltconstant_node *
jive_fltconstant_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_FLTCONSTANT_NODE))
		return (jive_fltconstant_node *) node;
	else
		return NULL;
}

#endif
