#include <jive/arch/memlayout.h>

#include <jive/arch/address.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/union/uniontype.h>

const jive_dataitem_memlayout *
jive_memlayout_mapper_map_value_type(jive_memlayout_mapper * self, const struct jive_value_type * type_)
{
	if (type_->base.class_ == &JIVE_BITSTRING_TYPE) {
		return jive_memlayout_mapper_map_bitstring(self, ((const jive_bitstring_type *) type_)->nbits);
	} else if (type_->base.class_ == &JIVE_ADDRESS_TYPE) {
		return jive_memlayout_mapper_map_address(self);
	} else if (type_->base.class_ == &JIVE_RECORD_TYPE) {
		const jive_record_type * type = (const jive_record_type *) type_;
		return &jive_memlayout_mapper_map_record(self, type->decl)->base;
	} else if (type_->base.class_ == &JIVE_UNION_TYPE) {
		const jive_union_type * type = (const jive_union_type *) type_;
		return &jive_memlayout_mapper_map_union(self, type->decl)->base;
	}
	
	return NULL;
}
