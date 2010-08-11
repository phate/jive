#include <jive/vsdg/resource-interference-private.h>

#include <jive/vsdg/graph.h>

jive_resource_interference *
jive_resource_interference_create(jive_resource * first, jive_resource * second)
{
	jive_resource_interference * i = jive_context_malloc(first->graph->context, sizeof(*i));
	i->first.resource = first;
	i->first.whole = i;
	i->second.resource = second;
	i->second.whole = i;
	i->count = 0;
	
	jive_resource_interference_hash_insert(&first->interference, &i->second);
	jive_resource_interference_hash_insert(&second->interference, &i->first);
	
	return i;
}

void
jive_resource_interference_destroy(jive_resource_interference * self)
{
	jive_resource_interference_hash_remove(&self->first.resource->interference, &self->second);
	jive_resource_interference_hash_remove(&self->second.resource->interference, &self->first);
	jive_context_free(self->first.resource->graph->context, self);
}
