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
{
	for (auto i : record_map_)
		delete[] i.second.element;
}

memlayout_mapper_simple::memlayout_mapper_simple(size_t bytes_per_word)
	:	memlayout_mapper(bytes_per_word)
{
	address_layout_.total_size = bytes_per_word;
	address_layout_.alignment = bytes_per_word;
}

jive_record_memlayout &
memlayout_mapper_simple::add_record(const rcd::declaration * decl)
{
	jive_record_memlayout layout;
	layout.decl = decl;
	layout.element = new jive_record_memlayout_element[decl->nelements()];
	for (size_t n = 0; n < decl->nelements(); n++) {
		layout.element[n].offset = 0;
		layout.element[n].size = 0;
	}
	layout.base.alignment = 1;
	layout.base.total_size = 0;

	record_map_[decl] = layout;
	return record_map_[decl];
}

jive_union_memlayout &
memlayout_mapper_simple::add_union(const unn::declaration * decl)
{
	jive_union_memlayout layout;
	layout.decl = decl;
	layout.base.alignment = 1;
	layout.base.total_size = 0;

	union_map_[decl] = layout;
	return union_map_[decl];
}

const jive_record_memlayout *
memlayout_mapper_simple::map_record(const rcd::declaration * decl)
{
	auto i = record_map_.find(decl);
	if (i != record_map_.end())
		return &i->second;

	size_t pos = 0, alignment = 1;
	jive_record_memlayout & layout = add_record(decl);
	for (size_t n = 0; n < decl->nelements(); n++) {
		const jive_dataitem_memlayout * ext = map_value_type(decl->element(n));
		
		if (alignment < ext->alignment)
			alignment = ext->alignment;

		size_t mask = ext->alignment - 1;
		pos = (pos + mask) & ~mask;
		layout.element[n].offset = pos;
		
		pos = pos + ext->total_size;
	}
	
	pos = (pos + alignment - 1) & ~(alignment - 1);
	layout.base.total_size = pos;
	layout.base.alignment = alignment;
	
	return &record_map_[decl];
}

const jive_union_memlayout *
memlayout_mapper_simple::map_union(const unn::declaration * decl)
{
	auto i = union_map_.find(decl);
	if (i != union_map_.end())
		return &i->second;
	
	size_t size = 0, alignment = 1;
	jive_union_memlayout & layout = add_union(decl);
	for (size_t n = 0; n < decl->nelements; n++) {
		const jive_dataitem_memlayout * ext = map_value_type(*decl->elements[n]);

		if (alignment < ext->alignment)
			alignment = ext->alignment;
		
		if (size < ext->total_size)
			size = ext->total_size;
	}
	
	size = (size + alignment - 1) & ~(alignment - 1);
	layout.base.total_size = size;
	layout.base.alignment = alignment;
	
	return &union_map_[decl];
}

const jive_dataitem_memlayout *
memlayout_mapper_simple::map_bitstring(size_t nbits)
{
	auto i = bitstring_map_.find(nbits);
	if (i != bitstring_map_.end())
		return &i->second;

	jive_dataitem_memlayout layout;
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

	size_t total_size = nbits / 8;
	layout.total_size = total_size;
	layout.alignment = std::min(bytes_per_word(), total_size);

	bitstring_map_[nbits] = layout;
	return &bitstring_map_[nbits];
}

const jive_dataitem_memlayout *
memlayout_mapper_simple::map_address()
{
	return &address_layout_;
}

}
