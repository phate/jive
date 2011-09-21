#include <jive/arch/stackslot.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>

#include <jive/arch/memory.h>
#include <jive/vsdg/statetype-private.h>

const jive_resource_class_class JIVE_STACK_RESOURCE = {
	.parent = &JIVE_ABSTRACT_RESOURCE,
	.name = "stack",
	.is_abstract = false
};

const jive_resource_class_class JIVE_STACK_FRAMESLOT_RESOURCE = {
	.parent = &JIVE_STACK_RESOURCE,
	.name = "stack_frameslot",
	.is_abstract = false
};

static const jive_resource_class_demotion no_demotion[] = {{NULL, NULL}};
static const jive_memory_type stackvar_type = {{{&JIVE_MEMORY_TYPE}}};

const jive_resource_class jive_root_stackslot_class = {
	.class_ = &JIVE_ABSTRACT_RESOURCE,
	.name = "stackslot",
	.limit = 0,
	.parent = &jive_root_resource_class,
	.depth = 1,
	.priority = jive_resource_class_priority_lowest,
	.demotions = no_demotion
};

#define MAKE_STACKSLOT_CLASS(SIZE, ALIGNMENT) \
const jive_stackslot_size_class jive_stackslot_class_##SIZE##_##ALIGNMENT = { \
	.base = { \
		.class_ = &JIVE_STACK_RESOURCE, \
		.name = "stack_s" #SIZE "a" #ALIGNMENT, \
		.limit = 0, .names = NULL, \
		.parent = &jive_root_stackslot_class, \
		.depth = 2, \
		.priority = jive_resource_class_priority_mem_low,\
		.demotions = no_demotion, \
		.type = &stackvar_type.base.base \
	}, \
	.size = SIZE, \
	.alignment = ALIGNMENT \
}

MAKE_STACKSLOT_CLASS(1, 1);
MAKE_STACKSLOT_CLASS(2, 2);
MAKE_STACKSLOT_CLASS(4, 4);
MAKE_STACKSLOT_CLASS(8, 8);
MAKE_STACKSLOT_CLASS(16, 16);

static const jive_stackslot_size_class *
jive_stackslot_size_class_create(size_t size, size_t alignment)
{
	/* try statically allocated classes first */
	if (size == alignment) {
		switch(size) {
			case 1: return &jive_stackslot_class_1_1;
			case 2: return &jive_stackslot_class_2_2;
			case 4: return &jive_stackslot_class_4_4;
			case 8: return &jive_stackslot_class_8_8;
			case 16: return &jive_stackslot_class_16_16;
		}
	};
	
	/* if requisite class not statically allocated, create it
	dynamically */
	char tmpname[80];
	snprintf(tmpname, sizeof(tmpname), "stack_s%zda%zd", size, alignment);
	
	char * name = strdup(tmpname);
	if (!name)
		return 0;
	
	jive_stackslot_size_class * cls = malloc(sizeof(*cls));
	if (!cls) {
		free(name);
		return 0;
	}
	
	cls->base.class_ = &JIVE_STACK_RESOURCE;
	cls->base.name = name;
	cls->base.limit = 0;
	cls->base.names = NULL;
	cls->base.parent = &jive_root_stackslot_class;
	cls->base.depth = cls->base.parent->depth + 1;
	cls->base.priority = jive_resource_class_priority_mem_low;
	cls->base.demotions = no_demotion;
	cls->base.type = &stackvar_type.base.base;
	cls->size = size;
	cls->alignment = alignment;
	
	return cls;
}

jive_stackslot *
jive_stackslot_create(const jive_resource_class * parent, long offset);

static const jive_fixed_stackslot_class *
jive_fixed_stackslot_class_create(const jive_stackslot_size_class * parent, int offset)
{
	char tmpname[80];
	snprintf(tmpname, sizeof(tmpname), "stack_s%zda%zd@%d", parent->size, parent->alignment, offset);
	
	char * name = strdup(tmpname);
	if (!name)
		return 0;
	
	jive_fixed_stackslot_class * cls = malloc(sizeof(*cls));
	if (!cls) {
		free(name);
		return 0;
	}
	
	jive_stackslot * slot = malloc(sizeof(*slot));
	if (!slot) {
		free(name);
		free(cls);
		return 0;
	}
	
	slot->base.name = name;
	slot->base.resource_class = &cls->base.base;
	slot->offset = offset;
	
	cls->base.base.class_ = &JIVE_STACK_FRAMESLOT_RESOURCE;
	cls->base.base.name = name;
	cls->base.base.limit = 1;
	cls->base.base.names = &cls->slot;
	cls->base.base.parent = &parent->base;
	cls->base.base.depth = parent->base.depth + 1;
	cls->base.base.priority = jive_resource_class_priority_mem_high;
	cls->base.base.demotions = no_demotion;
	cls->base.base.type = &stackvar_type.base.base;
	cls->base.size = parent->size;
	cls->base.alignment = parent->alignment;
	
	cls->slot = &slot->base;
	
	return cls;
}

typedef struct jive_stackslot_class_map_by_size jive_stackslot_class_map_by_size;

struct jive_stackslot_class_map_by_size {
	struct {
		const jive_stackslot_size_class ** items;
		size_t nitems;
	} by_alignment;
	struct {
		const jive_fixed_stackslot_class ** items;
		int begin, end;
	} by_offset;
};

static const jive_stackslot_size_class *
lookup_or_create_by_alignment(jive_stackslot_class_map_by_size * self, size_t size, size_t alignment)
{
	if (alignment >= self->by_alignment.nitems) {
		const jive_stackslot_size_class ** tmp;
		tmp = realloc(self->by_alignment.items, (alignment + 1) * sizeof(tmp[0]));
		if (!tmp)
			return 0;
		
		size_t n;
		for (n = self->by_alignment.nitems; n <= alignment; n++)
			tmp[n] = 0;
		self->by_alignment.items = tmp;
		self->by_alignment.nitems = alignment + 1;
	}
	
	if (!self->by_alignment.items[alignment]) {
		const jive_stackslot_size_class * cls = jive_stackslot_size_class_create(size, alignment);
		if (!cls)
			return 0;
		self->by_alignment.items[alignment] = cls;
	}
	
	return self->by_alignment.items[alignment];
}

static const jive_fixed_stackslot_class *
lookup_or_create_by_offset(jive_stackslot_class_map_by_size * self, size_t size, int offset)
{
	if (offset < self->by_offset.begin) {
		const jive_fixed_stackslot_class ** tmp;
		tmp = malloc( (self->by_offset.end - offset) * sizeof(tmp[0]) );
		if (!tmp)
			return 0;
		int n;
		for (n = offset; n < self->by_offset.begin; n++)
			tmp[n - offset] = 0;
		for (n = self->by_offset.begin; n < self->by_offset.end; n++)
			tmp[n - offset] = self->by_offset.items[n - self->by_offset.begin];
		free(self->by_offset.items);
		self->by_offset.items = tmp;
		self->by_offset.begin = offset;
	}
	
	if (offset >= self->by_offset.end) {
		const jive_fixed_stackslot_class ** tmp;
		tmp = realloc( self->by_offset.items, (offset - self->by_offset.begin + 1) * sizeof(tmp[0]) );
		if (!tmp)
			return 0;
		int n;
		for (n = self->by_offset.end; n<= offset; n++)
			tmp[n - self->by_offset.begin] = 0;
		self->by_offset.items = tmp;
		self->by_offset.end = offset + 1;
	}
	
	if (!self->by_offset.items[offset - self->by_offset.begin]) {
		size_t max_alignment = size | offset;
		size_t alignment = 1;
		while ((max_alignment & 1) == 0) {
			max_alignment >>= 1;
			alignment <<= 1;
		}
		
		const jive_stackslot_size_class * parent = lookup_or_create_by_alignment(self, size, alignment);
		if (!parent)
			return 0;
		
		const jive_fixed_stackslot_class * cls = jive_fixed_stackslot_class_create(parent, offset);
		if (!cls)
			return 0;
		self->by_offset.items[offset - self->by_offset.begin] = cls;
	}
	
	return self->by_offset.items[offset - self->by_offset.begin];
}

typedef struct jive_stackslot_class_map jive_stackslot_class_map;

struct jive_stackslot_class_map {
	pthread_mutex_t lock;
	jive_stackslot_class_map_by_size * map;
	size_t nitems;
};

static jive_stackslot_class_map_by_size *
lookup_or_create_size_map(jive_stackslot_class_map * self, size_t size)
{
	if (size >= self->nitems) {
		jive_stackslot_class_map_by_size * map;
		map = realloc(self->map, (size + 1) * sizeof(map[0]));
		if (!map)
			return 0;
		
		size_t n;
		for (n = self->nitems; n <= size; n++) {
			map[n].by_alignment.nitems = 0;
			map[n].by_alignment.items = NULL;
			map[n].by_offset.items = NULL;
			map[n].by_offset.begin = 0;
			map[n].by_offset.end = 0;
		}
		
		self->map = map;
		self->nitems = size + 1;
	}
	
	return &self->map[size];
}

static const jive_stackslot_size_class *
jive_stackslot_class_map_lookup_or_create_by_alignment(jive_stackslot_class_map * self, size_t size, size_t alignment)
{
	pthread_mutex_lock(&self->lock);
	
	size = size;
	alignment = alignment;
	
	jive_stackslot_class_map_by_size * map = lookup_or_create_size_map(self, size);
	if (!map) {
		pthread_mutex_unlock(&self->lock);
		return 0;
	}
	
	const jive_stackslot_size_class * cls = lookup_or_create_by_alignment(map, size, alignment);
	
	pthread_mutex_unlock(&self->lock);
	
	return cls;
}

static const jive_fixed_stackslot_class *
jive_stackslot_class_map_lookup_or_create_by_offset(jive_stackslot_class_map * self, size_t size, int offset)
{
	pthread_mutex_lock(&self->lock);
	
	size = size;
	
	jive_stackslot_class_map_by_size * map = lookup_or_create_size_map(self, size);
	if (!map) {
		pthread_mutex_unlock(&self->lock);
		return 0;
	}
	
	const jive_fixed_stackslot_class * cls = lookup_or_create_by_offset(map, size, offset);
	
	pthread_mutex_unlock(&self->lock);
	
	return cls;
}

static jive_stackslot_class_map class_map = {
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.map = 0,
	.nitems = 0
};

const jive_resource_class *
jive_stackslot_size_class_get(size_t size, size_t alignment)
{
	const jive_stackslot_size_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_alignment(&class_map, size, alignment);
	if (cls)
		return &cls->base;
	else
		return 0;
}

const jive_resource_class *
jive_fixed_stackslot_class_get(size_t size, int offset)
{
	const jive_fixed_stackslot_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_offset(&class_map, size, offset);
	if (cls)
		return &cls->base.base;
	else
		return 0;
}

const jive_resource_name *
jive_stackslot_name_get(size_t size, int offset)
{
	const jive_resource_class * rescls = jive_fixed_stackslot_class_get(size, offset);
	return rescls->names[0];
}
