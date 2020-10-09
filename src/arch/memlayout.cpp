/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memlayout.hpp>

#include <jive/arch/address.hpp>
#include <jive/types/bitstring/type.hpp>
#include <jive/types/union.hpp>

namespace jive {

dataitem_memlayout::~dataitem_memlayout()
{}

union_memlayout::~union_memlayout()
{}

record_memlayout::~record_memlayout()
{}

record_memlayout::record_memlayout(
	const rcddeclaration * dcl,
	const std::vector<record_memlayout_element> & elements,
	size_t size,
	size_t alignment) noexcept
	: dataitem_memlayout(size, alignment)
	, dcl_(dcl)
	, elements_(elements)
{
	JIVE_DEBUG_ASSERT(dcl->nelements() == elements.size());
}

memlayout_mapper::~memlayout_mapper()
{}

const dataitem_memlayout &
memlayout_mapper::map_value_type(const valuetype & type)
{
	if (auto t = dynamic_cast<const bittype*>(&type))
		return map_bitstring(t->nbits());

	if (dynamic_cast<const addrtype*>(&type))
		return map_address();

	if (auto t = dynamic_cast<const rcdtype*>(&type))
		return map_record(t->declaration());

	if (auto t = dynamic_cast<const jive::unntype*>(&type))
		return map_union(t->declaration());

	throw compiler_error("Type not supported.");
}

}
