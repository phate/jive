/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTCONSTANT_H
#define JIVE_TYPES_FLOAT_FLTCONSTANT_H

#include <jive/types/float/value-representation.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_node_class JIVE_FLTCONSTANT_NODE;

namespace jive {
namespace flt {

class constant_operation final : public jive::nullary_operation {
public:
	inline constexpr constant_operation(value_repr value) noexcept
		: value_(value) {}
	inline constexpr constant_operation(const constant_operation& other) noexcept
		: value_(other.value_) {}
	
	value_repr value() const noexcept { return value_; }
private:
	value_repr value_;
};

}
}

typedef jive::operation_node<jive::flt::constant_operation> jive_fltconstant_node;

jive::output *
jive_fltconstant(jive_graph * graph, jive::flt::value_repr value);

JIVE_EXPORTED_INLINE jive::output *
jive_fltconstant_float(jive_graph * graph, float value)
{
	return jive_fltconstant(graph, value);
}

#endif
