/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/stackslot.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>

#include <jive/arch/memorytype.h>
#include <jive/util/rangemap.h>

const jive_resource_class_class JIVE_STACK_RESOURCE = {
	parent : &JIVE_ABSTRACT_RESOURCE,
	name : "stack",
	is_abstract : false
};

const jive_resource_class_class JIVE_STACK_FRAMESLOT_RESOURCE = {
	parent : &JIVE_STACK_RESOURCE,
	name : "stack_frameslot",
	is_abstract : false
};

const jive_resource_class_class JIVE_STACK_CALLSLOT_RESOURCE = {
	parent : &JIVE_STACK_RESOURCE,
	name : "stack_callslot",
	is_abstract : false
};

static const jive_resource_class_demotion no_demotion[] = {{NULL, NULL}};
static const jive_memory_type stackvar_type;

const jive_resource_class jive_root_stackslot_class = {
	class_ : &JIVE_ABSTRACT_RESOURCE,
	name : "stackslot",
	limit : 0,
	names : NULL,
	parent : &jive_root_resource_class,
	depth : 1,
	priority : jive_resource_class_priority_lowest,
	demotions : no_demotion,
	type : NULL
};

#define MAKE_STACKSLOT_CLASS(SIZE, ALIGNMENT) \
const jive_stackslot_size_class jive_stackslot_class_##SIZE##_##ALIGNMENT = { \
	base : { \
		class_ : &JIVE_STACK_RESOURCE, \
		name : "stack_s" #SIZE "a" #ALIGNMENT, \
		limit : 0, names : NULL, \
		parent : &jive_root_stackslot_class, \
		depth : 2, \
		priority : jive_resource_class_priority_mem_generic,\
		demotions : no_demotion, \
		type : &stackvar_type \
	}, \
	size : SIZE, \
	alignment : ALIGNMENT \
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
	cls->base.priority = jive_resource_class_priority_mem_generic;
	cls->base.demotions = no_demotion;
	cls->base.type = &stackvar_type;
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
	cls->base.base.priority = jive_resource_class_priority_mem_unique;
	cls->base.base.demotions = no_demotion;
	cls->base.base.type = &stackvar_type;
	cls->base.size = parent->size;
	cls->base.alignment = parent->alignment;
	
	cls->slot = &slot->base;
	
	return cls;
}

static const jive_callslot_class *
jive_callslot_class_create(const jive_stackslot_size_class * parent, int offset)
{
	char tmpname[80];
	snprintf(tmpname, sizeof(tmpname), "call_s%zda%zd@%d", parent->size, parent->alignment, offset);
	
	char * name = strdup(tmpname);
	if (!name)
		return 0;
	
	jive_callslot_class * cls = malloc(sizeof(*cls));
	if (!cls) {
		free(name);
		return 0;
	}
	
	jive_callslot * slot = malloc(sizeof(*slot));
	if (!slot) {
		free(name);
		free(cls);
		return 0;
	}
	
	slot->base.name = name;
	slot->base.resource_class = &cls->base.base;
	slot->offset = offset;
	
	cls->base.base.class_ = &JIVE_STACK_CALLSLOT_RESOURCE;
	cls->base.base.name = name;
	cls->base.base.limit = 1;
	cls->base.base.names = &cls->slot;
	cls->base.base.parent = &parent->base;
	cls->base.base.depth = parent->base.depth + 1;
	cls->base.base.priority = jive_resource_class_priority_mem_unique;
	cls->base.base.demotions = no_demotion;
	cls->base.base.type = &stackvar_type;
	cls->base.size = parent->size;
	cls->base.alignment = parent->alignment;
	
	cls->offset = offset;
	cls->slot = &slot->base;
	
	return cls;
}

typedef struct jive_slot_offset_map jive_slot_offset_map;
typedef struct jive_callslot_offset_map jive_callslot_offset_map;
typedef struct jive_slot_alignment jive_slot_alignment;
typedef struct jive_slot_alignment_map jive_slot_alignment_map;
typedef struct jive_slot_size jive_slot_size;
typedef struct jive_slot_size_map jive_slot_size_map;

JIVE_DECLARE_RANGEMAP_TYPE(jive_slot_offset_map, const jive_fixed_stackslot_class *, NULL);
JIVE_DEFINE_RANGEMAP_TYPE(jive_slot_offset_map, const jive_fixed_stackslot_class *, NULL);
JIVE_DECLARE_RANGEMAP_TYPE(jive_callslot_offset_map, const jive_callslot_class *, NULL);
JIVE_DEFINE_RANGEMAP_TYPE(jive_callslot_offset_map, const jive_callslot_class *, NULL);

struct jive_slot_alignment {
	jive_slot_offset_map slot_by_offset;
	jive_callslot_offset_map callslot_by_offset;
	const jive_stackslot_size_class * cls;
};

#define JIVE_SLOT_ALIGNMENT_INITIALIZER \
	(jive_slot_alignment) { JIVE_RANGEMAP_INITIALIZER, JIVE_RANGEMAP_INITIALIZER, NULL }

JIVE_DECLARE_RANGEMAP_TYPE(jive_slot_alignment_map, jive_slot_alignment, JIVE_SLOT_ALIGNMENT_INITIALIZER);
JIVE_DEFINE_RANGEMAP_TYPE(jive_slot_alignment_map, jive_slot_alignment, JIVE_SLOT_ALIGNMENT_INITIALIZER);

struct jive_slot_size {
	jive_slot_alignment_map by_alignment;
};

#define JIVE_SLOT_SIZE_INITIALIZER \
	(jive_slot_size) { JIVE_RANGEMAP_INITIALIZER }

JIVE_DECLARE_RANGEMAP_TYPE(jive_slot_size_map, jive_slot_size, JIVE_SLOT_SIZE_INITIALIZER)
JIVE_DEFINE_RANGEMAP_TYPE(jive_slot_size_map, jive_slot_size, JIVE_SLOT_SIZE_INITIALIZER)

static const jive_stackslot_size_class *
lookup_or_create_size_class(jive_slot_size_map * self, size_t size, size_t alignment)
{
	jive_slot_size * by_size = jive_slot_size_map_lookup(self, size);
	if (!by_size)
		return NULL;
	
	jive_slot_alignment * by_alignment = jive_slot_alignment_map_lookup(&by_size->by_alignment, alignment);
	if (!by_alignment)
		return NULL;
	
	if (!by_alignment->cls) {
		const jive_stackslot_size_class * cls = jive_stackslot_size_class_create(size, alignment);
		if (!cls)
			return 0;
		by_alignment->cls = cls;
	}
	
	return by_alignment->cls;
}

static const jive_fixed_stackslot_class *
lookup_or_create_stackslot_class(jive_slot_size_map * self, size_t size, size_t alignment, ssize_t offset)
{
	JIVE_DEBUG_ASSERT((offset & (alignment - 1)) == 0);
	jive_slot_size * by_size = jive_slot_size_map_lookup(self, size);
	if (!by_size)
		return NULL;
	
	jive_slot_alignment * by_alignment = jive_slot_alignment_map_lookup(&by_size->by_alignment, alignment);
	if (!by_alignment)
		return NULL;
	
	const jive_fixed_stackslot_class ** pcls = jive_slot_offset_map_lookup(&by_alignment->slot_by_offset, offset);
	if (!*pcls) {
		const jive_stackslot_size_class * parent = by_alignment->cls;
		if (!parent) {
			parent = jive_stackslot_size_class_create(size, alignment);
			if (!parent)
				return NULL;
			by_alignment->cls = parent;
		}
		
		const jive_fixed_stackslot_class * cls = jive_fixed_stackslot_class_create(parent, offset);
		if (!cls)
			return NULL;
		*pcls = cls;
	}
	
	return *pcls;
}

static const jive_callslot_class *
lookup_or_create_callslot_class(jive_slot_size_map * self, size_t size, size_t alignment, ssize_t offset)
{
	JIVE_DEBUG_ASSERT((offset & (alignment - 1)) == 0);
	jive_slot_size * by_size = jive_slot_size_map_lookup(self, size);
	if (!by_size)
		return NULL;
	
	jive_slot_alignment * by_alignment = jive_slot_alignment_map_lookup(&by_size->by_alignment, alignment);
	if (!by_alignment)
		return NULL;
	
	const jive_callslot_class ** pcls = jive_callslot_offset_map_lookup(&by_alignment->callslot_by_offset, offset);
	if (!*pcls) {
		const jive_stackslot_size_class * parent = by_alignment->cls;
		if (!parent) {
			parent = jive_stackslot_size_class_create(size, alignment);
			if (!parent)
				return NULL;
			by_alignment->cls = parent;
		}
		
		const jive_callslot_class * cls = jive_callslot_class_create(parent, offset);
		if (!cls)
			return NULL;
		*pcls = cls;
	}
	
	return *pcls;
}

typedef struct jive_stackslot_class_map jive_stackslot_class_map;

struct jive_stackslot_class_map {
	pthread_mutex_t lock;
	jive_slot_size_map map;
};

static const jive_stackslot_size_class *
jive_stackslot_class_map_lookup_or_create_by_alignment(jive_stackslot_class_map * self, size_t size, size_t alignment)
{
	pthread_mutex_lock(&self->lock);
	const jive_stackslot_size_class * cls = lookup_or_create_size_class(&self->map, size, alignment);
	pthread_mutex_unlock(&self->lock);
	
	return cls;
}

static const jive_fixed_stackslot_class *
jive_stackslot_class_map_lookup_or_create_by_offset(jive_stackslot_class_map * self, size_t size, size_t alignment, ssize_t offset)
{
	pthread_mutex_lock(&self->lock);
	const jive_fixed_stackslot_class * cls = lookup_or_create_stackslot_class(&self->map, size, alignment, offset);
	pthread_mutex_unlock(&self->lock);
	return cls;
}

static const jive_callslot_class *
jive_callslot_class_map_lookup_or_create_by_offset(jive_stackslot_class_map * self, size_t size, size_t alignment, ssize_t offset)
{
	pthread_mutex_lock(&self->lock);
	const jive_callslot_class * cls = lookup_or_create_callslot_class(&self->map, size, alignment, offset);
	pthread_mutex_unlock(&self->lock);
	return cls;
}

static jive_stackslot_class_map class_map = {
	lock : PTHREAD_MUTEX_INITIALIZER,
	map : JIVE_RANGEMAP_INITIALIZER,
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
jive_fixed_stackslot_class_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_fixed_stackslot_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_offset(&class_map, size, alignment, offset);
	if (cls)
		return &cls->base.base;
	else
		return 0;
}

const jive_resource_class *
jive_callslot_class_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_callslot_class * cls;
	cls = jive_callslot_class_map_lookup_or_create_by_offset(&class_map, size, alignment, offset);
	if (cls)
		return &cls->base.base;
	else
		return 0;
}

const jive_resource_name *
jive_stackslot_name_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_resource_class * rescls = jive_fixed_stackslot_class_get(size, alignment, offset);
	return rescls->names[0];
}
