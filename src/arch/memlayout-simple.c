/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memlayout-simple.h>

#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>

namespace jive {

memlayout_mapper_simple::~memlayout_mapper_simple()
{}

const record_memlayout *
memlayout_mapper_simple::map_record(std::shared_ptr<const rcd::declaration> & decl)
{
	auto i = record_map_.find(decl);
	if (i != record_map_.end())
		return &i->second;

	size_t pos = 0, alignment = 1;
	std::vector<record_memlayout_element> elements;
	for (size_t n = 0; n < decl->nelements(); n++) {
		const dataitem_memlayout * ext = map_value_type(decl->element(n));
		
		alignment = std::max(alignment, ext->alignment());

		size_t mask = ext->alignment() - 1;
		pos = (pos + mask) & ~mask;
		elements.push_back(record_memlayout_element(ext->size(), pos));
		
		pos = pos + ext->size();
	}
	pos = (pos + alignment - 1) & ~(alignment - 1);

	record_map_.insert(std::make_pair(decl, record_memlayout(decl, elements, pos, alignment)));
	return &record_map_.find(decl)->second;
}

const union_memlayout *
memlayout_mapper_simple::map_union(const unn::declaration * decl)
{
	auto i = union_map_.find(decl);
	if (i != union_map_.end())
		return &i->second;
	
	size_t size = 0, alignment = 1;
	for (size_t n = 0; n < decl->nelements; n++) {
		const dataitem_memlayout * ext = map_value_type(*decl->elements[n]);
		alignment = std::max(alignment, ext->alignment());
		size = std::max(size, ext->size());
	}
	size = (size + alignment - 1) & ~(alignment - 1);

	union_map_.insert(std::make_pair(decl, union_memlayout(decl, size, alignment)));
	return &union_map_.find(decl)->second;
}

const dataitem_memlayout *
memlayout_mapper_simple::map_bitstring(size_t nbits)
{
	auto i = bitstring_map_.find(nbits);
	if (i != bitstring_map_.end())
		return &i->second;

	if (nbits > bits_per_word())
		nbits = (nbits + bits_per_word() - 1) & ~ (bits_per_word() - 1);
	else if (nbits <= 8)
		nbits = 8;
	else if (nbits <= 16)
		nbits = 16;
	else if (nbits <= 32)
		nbits = 32;
	else if (nbits <= 64)
		nbits = 64;
	else if (nbits <= 128)
		nbits = 128;
	else
		JIVE_DEBUG_ASSERT(0);

	size_t size = nbits / 8;
	size_t alignment = std::min(bytes_per_word(), size);
	bitstring_map_.insert(std::make_pair(nbits, dataitem_memlayout(size, alignment)));
	return &bitstring_map_.find(nbits)->second;
}

const dataitem_memlayout *
memlayout_mapper_simple::map_address()
{
	return &address_layout_;
}

}
