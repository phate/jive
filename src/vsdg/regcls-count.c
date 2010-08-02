#include <jive/vsdg/regcls-count-private.h>
#include <jive/arch/registers.h>
#include <jive/util/list.h>
#include <jive/internal/compiler.h>

void
jive_regcls_count_clear(jive_regcls_count * self, jive_context * context)
{
	size_t n;
	for(n=0; n<self->nbuckets; n++) {
		while(self->buckets[n].first) {
			jive_regcls_count_item * item = self->buckets[n].first;
			JIVE_LIST_REMOVE(self->buckets[n], item, chain);
			jive_context_free(context, item);
		}
	}
	self->nitems = 0;
}

static jive_regcls_count_item *
jive_regcls_count_lookup_item(const jive_regcls_count * self, const jive_regcls * regcls)
{
	if (!self->nbuckets) return 0;
	size_t hash = ((size_t) regcls) % self->nbuckets;
	jive_regcls_count_item * item = self->buckets[hash].first;
	while(item && item->regcls != regcls) item = item->chain.next;
	
	return item;
}

static size_t
jive_regcls_count_lookup(const jive_regcls_count * self, const jive_regcls * regcls)
{
	jive_regcls_count_item * item = jive_regcls_count_lookup_item(self, regcls);
	if (item) return item->count;
	else return 0;
}

static void
rehash(jive_regcls_count * self, jive_context * context)
{
	size_t new_nbuckets = self->nitems * 2 + 1;
	jive_regcls_count_bucket * new_buckets = jive_context_malloc(context, sizeof(*new_buckets) * new_nbuckets);
	size_t n;
	for(n=0; n<new_nbuckets; n++)
		new_buckets[n].first = new_buckets[n].last = 0;
	
	for(n=0; n<self->nbuckets; n++) {
		while(self->buckets[n].first) {
			jive_regcls_count_item * item = self->buckets[n].first;
			JIVE_LIST_REMOVE(self->buckets[n], item, chain);
			size_t hash = ((size_t)item->regcls) % new_nbuckets;
			JIVE_LIST_PUSH_BACK(new_buckets[hash], item, chain);
		}
	}
	
	jive_context_free(context, self->buckets);
	self->buckets = new_buckets;
	self->nbuckets = new_nbuckets;
}

static size_t
jive_regcls_count_add_single(jive_regcls_count * self, jive_context * context, const jive_regcls * regcls, size_t count)
{
	jive_regcls_count_item * item = jive_regcls_count_lookup_item(self, regcls);
	if (likely(item != 0)) return item->count += count;
	self->nitems ++;
	if (self->nitems >= self->nbuckets) rehash(self, context);
	
	item = jive_context_malloc(context, sizeof(*item));
	size_t hash = ((size_t) regcls) % self->nbuckets;
	item->regcls = regcls;
	item->count = 0;
	JIVE_LIST_PUSH_BACK(self->buckets[hash], item, chain);
	
	item->count += count;
	return item->count;
}

static void
jive_regcls_count_sub_single(jive_regcls_count * self, jive_context * context, const jive_regcls * regcls)
{
	jive_regcls_count_item * item = jive_regcls_count_lookup_item(self, regcls);
	item->count --;
	if (item->count == 0) {
		size_t hash = ((size_t) regcls) % self->nbuckets;
		JIVE_LIST_REMOVE(self->buckets[hash], item, chain);
		jive_context_free(context, item);
	}
}


const jive_regcls *
jive_regcls_count_add(jive_regcls_count * self, jive_context * context, const jive_regcls * regcls)
{
	const jive_regcls * overflow = 0;
	while(regcls) {
		size_t count = jive_regcls_count_add_single(self, context, regcls, 1);
		if (count > regcls->nregs && ! overflow) overflow = regcls;
		
		regcls = regcls->parent;
	}
	return overflow;
}

void
jive_regcls_count_sub(jive_regcls_count * self, jive_context * context, const struct jive_regcls * regcls)
{
	while(regcls) {
		jive_regcls_count_sub_single(self, context, regcls);
		regcls = regcls->parent;
	}
}

const jive_regcls *
jive_regcls_count_change(jive_regcls_count * self, jive_context * context, const struct jive_regcls * old_regcls, const struct jive_regcls * new_regcls)
{
	jive_regcls_count_sub(self, context, old_regcls);
	return jive_regcls_count_add(self, context, new_regcls);
}

const jive_regcls *
jive_regcls_count_check_add(const jive_regcls_count * self, const jive_regcls * regcls)
{
	while(regcls) {
		size_t count = jive_regcls_count_lookup(self, regcls);
		if (count + 1 > regcls->nregs) return regcls;
		regcls = regcls->parent;
	}
	return 0;
}

const jive_regcls *
jive_regcls_count_check_change(const jive_regcls_count * self, const jive_regcls * old_regcls, const jive_regcls * new_regcls)
{
	if (!old_regcls) return jive_regcls_count_check_add(self, new_regcls);
	if (!new_regcls) return 0;
	
	const jive_regcls * common_regcls = jive_regcls_common_ancestor(old_regcls, new_regcls);
	
	while(new_regcls != common_regcls) {
		size_t count = jive_regcls_count_lookup(self, new_regcls);
		if (count + 1 > new_regcls->nregs) return new_regcls;
		new_regcls = new_regcls->parent;
	}
	return 0;
}

void
jive_regcls_count_copy(jive_regcls_count * self, jive_context * context, const jive_regcls_count * src)
{
	jive_regcls_count_clear(self, context);
	size_t n;
	for(n=0; n<src->nbuckets; n++) {
		jive_regcls_count_item * item;
		JIVE_LIST_ITERATE(src->buckets[n], item, chain)
			jive_regcls_count_add_single(self, context, item->regcls, item->count);
	}
}
