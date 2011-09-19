#include <jive/arch/registers.h>
#include <jive/bitstring/type.h>

const struct jive_type *
jive_register_class_get_type(const jive_register_class * self)
{
	return jive_resource_class_get_type(&self->base);
}

jive_gate *
jive_register_class_create_gate(const jive_register_class * self, struct jive_graph * graph, const char * name)
{
	return jive_resource_class_create_gate(&self->base, graph, name);
}

static const jive_resource_class_demotion no_demotion[] = {{NULL, NULL}};

const jive_resource_class jive_root_register_class = {
	.name = "reg",
	.limit = 0,
	.parent = &jive_root_resource_class,
	.depth = 1,
	.is_abstract = true,
	.priority = jive_resource_class_priority_lowest,
	.demotions = no_demotion
};
