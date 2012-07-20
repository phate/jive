#include <jive/serialization/driver.h>

#include <jive/serialization/nodecls-registry.h>
#include <jive/serialization/rescls-registry.h>
#include <jive/serialization/typecls-registry.h>

void
jive_serialization_driver_init(
	jive_serialization_driver * self,
	jive_context * context)
{
	self->context = context;
	self->nodecls_registry = jive_serialization_nodecls_registry_get();
	self->typecls_registry = jive_serialization_typecls_registry_get();
	self->rescls_registry = jive_serialization_rescls_registry_get();
	jive_serialization_symtab_init(&self->symtab, context);
}

void
jive_serialization_driver_fini(
	jive_serialization_driver * self)
{
	jive_serialization_nodecls_registry_put(self->nodecls_registry);
	jive_serialization_typecls_registry_put(self->typecls_registry);
	jive_serialization_rescls_registry_put(self->rescls_registry);
	jive_serialization_symtab_fini(&self->symtab);
}
