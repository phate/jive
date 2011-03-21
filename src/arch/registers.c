#include <jive/arch/registers.h>
#include <jive/bitstring/type.h>

static const jive_bitstring_type
	bits8 = {{{.class_ = &JIVE_BITSTRING_TYPE}}, .nbits = 8};
static const jive_bitstring_type
	bits16 = {{{.class_ = &JIVE_BITSTRING_TYPE}}, .nbits = 16};
static const jive_bitstring_type
	bits32 = {{{.class_ = &JIVE_BITSTRING_TYPE}}, .nbits = 32};
static const jive_bitstring_type
	bits64 = {{{.class_ = &JIVE_BITSTRING_TYPE}}, .nbits = 64};
static const jive_bitstring_type
	bits128 = {{{.class_ = &JIVE_BITSTRING_TYPE}}, .nbits = 128};

const struct jive_type *
jive_register_class_get_type(const jive_register_class * self)
{
	switch(self->nbits) {
		case 8: return &bits8.base.base;
		case 16: return &bits16.base.base;
		case 32: return &bits32.base.base;
		case 64: return &bits64.base.base;
		case 128: return &bits128.base.base;
		default: return 0;
	}
}

struct jive_gate *
jive_register_class_create_gate(const jive_register_class * self, struct jive_graph * graph, const char * name)
{
	const jive_type * type = jive_register_class_get_type(self);
	jive_gate * gate = jive_type_create_gate(type, graph, name);
	gate->required_rescls = &self->base;
	return gate;
}

