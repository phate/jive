#include <jive/context.h>
#include <jive/internal/compiler.h>
#include <jive/util/list.h>
#include <jive/vsdg/resource.h>

const jive_resource_class *
jive_resource_class_union(const jive_resource_class * self, const jive_resource_class * other)
{
	for(;;) {
		if (self == other) return self;
		if (self->depth > other->depth)
			self = self->parent;
		else
			other = other->parent;
	}
}

const jive_resource_class *
jive_resource_class_intersection(const jive_resource_class * self, const jive_resource_class * other)
{
	const jive_resource_class * u = jive_resource_class_union(self, other);
	if (u == self) return other;
	else if (u == other) return self;
	else return 0;
}

const jive_resource_class *
jive_resource_class_relax(const jive_resource_class * self)
{
	/* hopefully this function is transitionary --
	currently everything that is needed is the
	class directly below the root */
	while (self->parent != &jive_root_resource_class)
		self = self->parent;
	return self;
}

const jive_resource_class jive_root_resource_class = {
	.name = "root",
	.limit = 0,
	.parent = 0,
	.depth = 0
};

void
jive_resource_class_count_clear(jive_resource_class_count * self, jive_context * context)
{
	size_t n;
	for(n=0; n<self->nbuckets; n++) {
		while(self->buckets[n].first) {
			jive_resource_class_count_item * item = self->buckets[n].first;
			JIVE_LIST_REMOVE(self->buckets[n], item, chain);
			jive_context_free(context, item);
		}
	}
	self->nitems = 0;
}

static jive_resource_class_count_item *
jive_resource_class_count_lookup_item(const jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	if (!self->nbuckets) return 0;
	size_t hash = ((size_t) resource_class) % self->nbuckets;
	jive_resource_class_count_item * item = self->buckets[hash].first;
	while(item && item->resource_class != resource_class) item = item->chain.next;
	
	return item;
}

static size_t
jive_resource_class_count_lookup(const jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	if (item) return item->count;
	else return 0;
}

static void
rehash(jive_resource_class_count * self, jive_context * context)
{
	size_t new_nbuckets = self->nitems * 2 + 1;
	jive_resource_class_count_bucket * new_buckets = jive_context_malloc(context, sizeof(*new_buckets) * new_nbuckets);
	size_t n;
	for(n=0; n<new_nbuckets; n++)
		new_buckets[n].first = new_buckets[n].last = 0;
	
	for(n=0; n<self->nbuckets; n++) {
		while(self->buckets[n].first) {
			jive_resource_class_count_item * item = self->buckets[n].first;
			JIVE_LIST_REMOVE(self->buckets[n], item, chain);
			size_t hash = ((size_t)item->resource_class) % new_nbuckets;
			JIVE_LIST_PUSH_BACK(new_buckets[hash], item, chain);
		}
	}
	
	jive_context_free(context, self->buckets);
	self->buckets = new_buckets;
	self->nbuckets = new_nbuckets;
}

static size_t
jive_resource_class_count_add_single(jive_resource_class_count * self, jive_context * context, const jive_resource_class * resource_class, size_t count)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	if (likely(item != 0)) return item->count += count;
	self->nitems ++;
	if (self->nitems >= self->nbuckets) rehash(self, context);
	
	item = jive_context_malloc(context, sizeof(*item));
	size_t hash = ((size_t) resource_class) % self->nbuckets;
	item->resource_class = resource_class;
	item->count = 0;
	JIVE_LIST_PUSH_BACK(self->buckets[hash], item, chain);
	
	item->count += count;
	return item->count;
}

void
jive_resource_class_count_max(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * resource_class, size_t count)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	if (likely(item != 0)) {
		if (item->count < count) item->count = count;
		return;
	}
	self->nitems ++;
	if (self->nitems >= self->nbuckets) rehash(self, context);
	
	item = jive_context_malloc(context, sizeof(*item));
	size_t hash = ((size_t) resource_class) % self->nbuckets;
	item->resource_class = resource_class;
	item->count = count;
	JIVE_LIST_PUSH_BACK(self->buckets[hash], item, chain);
}

static void
jive_resource_class_count_sub_single(jive_resource_class_count * self, jive_context * context, const jive_resource_class * resource_class)
{
	jive_resource_class_count_item * item = jive_resource_class_count_lookup_item(self, resource_class);
	item->count --;
	if (item->count == 0) {
		size_t hash = ((size_t) resource_class) % self->nbuckets;
		JIVE_LIST_REMOVE(self->buckets[hash], item, chain);
		jive_context_free(context, item);
	}
}


const jive_resource_class *
jive_resource_class_count_add(jive_resource_class_count * self, jive_context * context, const jive_resource_class * resource_class)
{
	const jive_resource_class * overflow = 0;
	while(resource_class) {
		size_t count = jive_resource_class_count_add_single(self, context, resource_class, 1);
		if (count > resource_class->limit && resource_class->limit && ! overflow) overflow = resource_class;
		
		resource_class = resource_class->parent;
	}
	return overflow;
}

void
jive_resource_class_count_sub(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * resource_class)
{
	while(resource_class) {
		jive_resource_class_count_sub_single(self, context, resource_class);
		resource_class = resource_class->parent;
	}
}

const jive_resource_class *
jive_resource_class_count_change(jive_resource_class_count * self, jive_context * context, const struct jive_resource_class * old_resource_class, const struct jive_resource_class * new_resource_class)
{
	jive_resource_class_count_sub(self, context, old_resource_class);
	return jive_resource_class_count_add(self, context, new_resource_class);
}

const jive_resource_class *
jive_resource_class_count_check_add(const jive_resource_class_count * self, const jive_resource_class * resource_class)
{
	while(resource_class) {
		size_t count = jive_resource_class_count_lookup(self, resource_class);
		if (count + 1 > resource_class->limit && resource_class->limit) return resource_class;
		resource_class = resource_class->parent;
	}
	return 0;
}

const jive_resource_class *
jive_resource_class_count_check_change(const jive_resource_class_count * self, const jive_resource_class * old_resource_class, const jive_resource_class * new_resource_class)
{
	if (!old_resource_class) return jive_resource_class_count_check_add(self, new_resource_class);
	if (!new_resource_class) return 0;
	
	const jive_resource_class * common_resource_class = jive_resource_class_union(old_resource_class, new_resource_class);
	
	while(new_resource_class != common_resource_class) {
		size_t count = jive_resource_class_count_lookup(self, new_resource_class);
		if (count + 1 > new_resource_class->limit && new_resource_class->limit) return new_resource_class;
		new_resource_class = new_resource_class->parent;
	}
	return 0;
}

void
jive_resource_class_count_copy(jive_resource_class_count * self, jive_context * context, const jive_resource_class_count * src)
{
	jive_resource_class_count_clear(self, context);
	size_t n;
	for(n=0; n<src->nbuckets; n++) {
		jive_resource_class_count_item * item;
		JIVE_LIST_ITERATE(src->buckets[n], item, chain)
			jive_resource_class_count_add_single(self, context, item->resource_class, item->count);
	}
}
