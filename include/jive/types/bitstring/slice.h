/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_SLICE_H
#define JIVE_TYPES_BITSTRING_SLICE_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_unary_operation_class JIVE_BITSLICE_NODE_;
#define JIVE_BITSLICE_NODE (JIVE_BITSLICE_NODE_.base)

namespace jive {
namespace bitstring {

class slice_operation : public jive_node_attrs {
public:
	inline constexpr
	slice_operation(size_t low, size_t high) noexcept
		: low_(low), high_(high)
	{
	}
	
	inline size_t
	low() const noexcept { return low_; }
	
	inline size_t
	high() const noexcept { return high_; }
private:
	size_t low_;
	size_t high_;
};

}
}

typedef jive::operation_node<jive::bitstring::slice_operation> jive_bitslice_node;

/**
	\brief Create bitslice
	\param operand Input value
	\param low Low bit
	\param high High bit
	\returns Bitstring value representing slice
	
	Convenience function that either creates a new slice or
	returns the output handle of an existing slice.
*/
jive_output *
jive_bitslice(jive_output * operand, size_t low, size_t high);

#endif
