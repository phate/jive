/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_SIMPLE_H
#define JIVE_ARCH_MEMLAYOUT_SIMPLE_H

#include <jive/arch/memlayout.h>

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

	virtual const record_memlayout *
	map_record(std::shared_ptr<const rcd::declaration> & decl) override;

	virtual const union_memlayout *
	map_union(const struct unn::declaration * decl) override;

	virtual const dataitem_memlayout *
	map_bitstring(size_t nbits) override;

	virtual const dataitem_memlayout *
	map_address()	override;

private:
	dataitem_memlayout address_layout_;
	std::unordered_map<size_t, dataitem_memlayout> bitstring_map_;
	std::unordered_map<const jive::unn::declaration*, union_memlayout> union_map_;
	std::unordered_map<
		std::shared_ptr<const rcd::declaration>,
		record_memlayout
	> record_map_;
};

}

#endif
