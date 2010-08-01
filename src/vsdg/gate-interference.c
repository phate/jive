#include <jive/vsdg/gate-interference-private.h>

#include <jive/vsdg/graph.h>
#include <jive/util/list.h>

static void
rehash(jive_gate_interference_hash * self, jive_context * context)
{
	size_t new_nbuckets = self->nitems * 2 + 1;
	jive_gate_interference_hash_bucket * new_buckets = jive_context_malloc(context, sizeof(*new_buckets) * new_nbuckets);
	
	size_t n;
	for(n=0; n<new_nbuckets; n++) new_buckets[n].first = new_buckets[n].last = 0;
	
	for(n=0; n<self->nbuckets; n++) {
		while(self->buckets[n].first) {
			jive_gate_interference_part * part = self->buckets[n].first;
			JIVE_LIST_REMOVE(self->buckets[n], part, chain);
			size_t hash = ((size_t) part->gate) % new_nbuckets;
			JIVE_LIST_PUSH_BACK(new_buckets[hash], part, chain);
		}
	}
	
	jive_context_free(context, self->buckets);
	self->buckets = new_buckets;
	self->nbuckets = new_nbuckets;
}

static void
jive_gate_interference_hash_insert(jive_gate_interference_hash * self, jive_gate_interference_part * part)
{
	self->nitems ++;
	if (self->nitems > self->nbuckets) rehash(self, part->gate->graph->context);
	
	size_t hash = ((size_t) part->gate) % self->nbuckets;
	JIVE_LIST_PUSH_BACK(self->buckets[hash], part, chain);
}

static void
jive_gate_interference_hash_remove(jive_gate_interference_hash * self, jive_gate_interference_part * part)
{
	self->nitems --;
	
	size_t hash = ((size_t) part->gate) % self->nbuckets;
	JIVE_LIST_REMOVE(self->buckets[hash], part, chain);
}

jive_gate_interference *
jive_gate_interference_create(jive_gate * first, jive_gate * second)
{
	jive_gate_interference * i = jive_context_malloc(first->graph->context, sizeof(*i));
	i->first.gate = first;
	i->first.whole = i;
	i->second.gate = second;
	i->second.whole = i;
	i->count = 0;
	
	jive_gate_interference_hash_insert(&first->interference, &i->second);
	jive_gate_interference_hash_insert(&second->interference, &i->first);
	
	return i;
}

void
jive_gate_interference_destroy(jive_gate_interference * self)
{
	jive_gate_interference_hash_remove(&self->first.gate->interference, &self->second);
	jive_gate_interference_hash_remove(&self->second.gate->interference, &self->first);
	jive_context_free(self->first.gate->graph->context, self);
}
