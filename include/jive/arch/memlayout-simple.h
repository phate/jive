/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_H
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_H

#include <jive/arch/memlayout.h>

#include <unordered_map>

namespace jive {

/* simplistic implementation, for simple use cases & testing */

/* FIXME: endianness */
class memlayout_mapper_simple : public memlayout_mapper {
public:
	virtual
	~memlayout_mapper_simple();

	memlayout_mapper_simple(size_t bytes_per_word);

	inline size_t
	bytes_per_word() const noexcept
	{
		return bytes_per_word_;
	}

	inline size_t
	bits_per_word() const noexcept
	{
		return bytes_per_word() * 8;
	}

	jive_dataitem_memlayout address_layout;
	std::unordered_map<const jive::rcd::declaration*, jive_record_memlayout> record_map;
	std::unordered_map<const jive::unn::declaration*, jive_union_memlayout> union_map;
	std::unordered_map<size_t, jive_dataitem_memlayout> bitstring_map;

private:
	size_t bytes_per_word_;
};

}

#endif
