/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/memlayout.h>

#include <jive/arch/address.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/union/unntype.h>

const jive_dataitem_memlayout *
jive_memlayout_mapper_map_value_type(jive_memlayout_mapper * self,
	const struct jive_value_type * type_)
{
	if (dynamic_cast<const jive::bits::type*>(type_)) {
		return jive_memlayout_mapper_map_bitstring(self,
			static_cast<const jive::bits::type*>(type_)->nbits());
	} else if (dynamic_cast<const jive_address_type*>(type_)) {
		return jive_memlayout_mapper_map_address(self);
	} else if (dynamic_cast<const jive_record_type*>(type_)) {
		const jive_record_type * type = (const jive_record_type *) type_;
		return &jive_memlayout_mapper_map_record(self, type->declaration())->base;
	} else if (dynamic_cast<const jive_union_type*>(type_)) {
		const jive_union_type * type = (const jive_union_type *) type_;
		return &jive_memlayout_mapper_map_union(self, type->declaration())->base;
	}
	
	return NULL;
}
