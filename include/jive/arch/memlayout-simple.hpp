/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_HPP
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_HPP

#include <jive/arch/memlayout.hpp>

#include <unordered_map>

namespace jive {

/* simplistic implementation, for simple use cases & testing */

/* FIXME: endianness */
class memlayout_mapper_simple final : public memlayout_mapper {
public:
	virtual
	~memlayout_mapper_simple();

	inline
	memlayout_mapper_simple(size_t bytes_per_word)
		: memlayout_mapper(bytes_per_word)
		, address_layout_(bytes_per_word, bytes_per_word)
	{}

	virtual const record_memlayout &
	map_record(const rcddeclaration * dcl) override;

	virtual const union_memlayout &
	map_union(const unndeclaration * decl) override;

	virtual const dataitem_memlayout &
	map_bitstring(size_t nbits) override;

	virtual const dataitem_memlayout &
	map_address()	override;

private:
	dataitem_memlayout address_layout_;
	std::unordered_map<size_t, dataitem_memlayout> bitstring_map_;
	std::unordered_map<const jive::unndeclaration*, union_memlayout> union_map_;
	std::unordered_map<const jive::rcddeclaration*, record_memlayout> record_map_;
};

}

#endif
