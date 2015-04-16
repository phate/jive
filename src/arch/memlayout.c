/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memlayout.h>

#include <jive/arch/address.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/union/unntype.h>

namespace jive {

memlayout_mapper::~memlayout_mapper()
{}

const jive_dataitem_memlayout *
memlayout_mapper::map_value_type(const value::type & type)
{
	if (auto t = dynamic_cast<const bits::type*>(&type))
		return map_bitstring(t->nbits());

	if (dynamic_cast<const addr::type*>(&type))
		return map_address();

	if (auto t = dynamic_cast<const jive::rcd::type*>(&type))
		return &map_record(t->declaration().get())->base;

	if (auto t = dynamic_cast<const jive::unn::type*>(&type))
		return &map_union(t->declaration())->base;

	return nullptr;
}

}
