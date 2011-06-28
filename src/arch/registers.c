#include <jive/arch/registers.h>
#include <jive/bitstring/type.h>

const struct jive_type *
jive_register_class_get_type(const jive_register_class * self)
{
	return jive_resource_class_get_type(&self->base);
}

struct jive_gate *
jive_register_class_create_gate(const jive_register_class * self, struct jive_graph * graph, const char * name)
{
	const jive_type * type = jive_register_class_get_type(self);
	jive_gate * gate = jive_type_create_gate(type, graph, name);
	gate->required_rescls = &self->base;
	return gate;
}

