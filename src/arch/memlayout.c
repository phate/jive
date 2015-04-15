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

}

const jive_dataitem_memlayout *
jive_memlayout_mapper_map_value_type(jive::memlayout_mapper * self, const jive::value::type * type_)
{
	if (dynamic_cast<const jive::bits::type*>(type_)) {
		return jive_memlayout_mapper_map_bitstring(self,
			static_cast<const jive::bits::type*>(type_)->nbits());
	} else if (dynamic_cast<const jive::addr::type*>(type_)) {
		return jive_memlayout_mapper_map_address(self);
	} else if (dynamic_cast<const jive::rcd::type*>(type_)) {
		const jive::rcd::type * type = static_cast<const jive::rcd::type*>(type_);
		return &jive_memlayout_mapper_map_record(self, type->declaration().get())->base;
	} else if (dynamic_cast<const jive::unn::type*>(type_)) {
		const jive::unn::type * type = static_cast<const jive::unn::type*>(type_);
		return &jive_memlayout_mapper_map_union(self, type->declaration())->base;
	}
	
	return NULL;
}
