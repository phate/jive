/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/stackslot.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jive/common.h>

#include <jive/arch/memorytype.h>

#include <unordered_map>

jive_stackslot_size_class::~jive_stackslot_size_class()
{}

jive_fixed_stackslot_class::~jive_fixed_stackslot_class()
{}

jive_stackslot::~jive_stackslot()
{}

jive_callslot_class::~jive_callslot_class()
{}

jive_callslot::~jive_callslot()
{}

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
static const jive::mem::type stackvar_type;

const jive_resource_class jive_root_stackslot_class(
	&JIVE_ABSTRACT_RESOURCE, "stackslot", 0,
	nullptr, &jive_root_resource_class, 1,
	jive_resource_class_priority_lowest,
	no_demotion, nullptr);

#define MAKE_STACKSLOT_CLASS(SIZE, ALIGNMENT) \
const jive_stackslot_size_class jive_stackslot_class_##SIZE##_##ALIGNMENT(\
	&JIVE_STACK_RESOURCE, "stack_s" #SIZE "a" #ALIGNMENT, \
		0, NULL, &jive_root_stackslot_class, \
		2, jive_resource_class_priority_mem_generic,\
		no_demotion, &stackvar_type, SIZE, ALIGNMENT \
);

MAKE_STACKSLOT_CLASS(1, 1);
MAKE_STACKSLOT_CLASS(2, 2);
MAKE_STACKSLOT_CLASS(4, 4);
MAKE_STACKSLOT_CLASS(8, 8);
MAKE_STACKSLOT_CLASS(16, 16);

static const jive_stackslot_size_class *
jive_stackslot_size_class_static(size_t size, size_t alignment)
{
	if (size == alignment) {
		switch(size) {
			case 1: return &jive_stackslot_class_1_1;
			case 2: return &jive_stackslot_class_2_2;
			case 4: return &jive_stackslot_class_4_4;
			case 8: return &jive_stackslot_class_8_8;
			case 16: return &jive_stackslot_class_16_16;
		}
	}

	return nullptr;
}

static jive_stackslot_size_class *
jive_stackslot_size_class_create(size_t size, size_t alignment)
{
	char tmpname[80];
	snprintf(tmpname, sizeof(tmpname), "stack_s%zda%zd", size, alignment);
	
	char * name = strdup(tmpname);
	if (!name)
		return 0;
	
	jive_stackslot_size_class * cls = new jive_stackslot_size_class(
		&JIVE_STACK_RESOURCE, name, 0, nullptr, &jive_root_stackslot_class,
		jive_root_stackslot_class.depth+1, jive_resource_class_priority_mem_generic,
		no_demotion, &stackvar_type, size, alignment);
	if (!cls) {
		free(name);
		return 0;
	}

	return cls;
}

jive_stackslot *
jive_stackslot_create(const jive_resource_class * parent, long offset);

static jive_fixed_stackslot_class *
jive_fixed_stackslot_class_create(const jive_stackslot_size_class * parent, int offset)
{
	char tmpname[80];
	snprintf(tmpname, sizeof(tmpname), "stack_s%zda%zd@%d", parent->size, parent->alignment, offset);
	
	char * name = strdup(tmpname);
	if (!name)
		return 0;
	
	jive_fixed_stackslot_class * cls = new jive_fixed_stackslot_class(&JIVE_STACK_FRAMESLOT_RESOURCE,
		name, 1, nullptr, parent, parent->depth+1, jive_resource_class_priority_mem_unique,
		no_demotion, &stackvar_type, parent->size, parent->alignment, nullptr);
	if (!cls) {
		free(name);
		return 0;
	}
	
	jive_stackslot * slot = new jive_stackslot(name, cls, offset);
	if (!slot) {
		free(name);
		delete cls;
		return 0;
	}

	cls->names = &cls->slot;
	cls->slot = slot;
	
	return cls;
}

static jive_callslot_class *
jive_callslot_class_create(const jive_stackslot_size_class * parent, int offset)
{
	char tmpname[80];
	snprintf(tmpname, sizeof(tmpname), "call_s%zda%zd@%d", parent->size, parent->alignment, offset);
	
	char * name = strdup(tmpname);
	if (!name)
		return 0;
	
	jive_callslot_class * cls = new jive_callslot_class(&JIVE_STACK_CALLSLOT_RESOURCE, name, 1,
		nullptr, parent, parent->depth+1, jive_resource_class_priority_mem_unique, no_demotion,
		&stackvar_type, parent->size, parent->alignment, offset, nullptr);
	if (!cls) {
		free(name);
		return 0;
	}
	
	jive_callslot * slot = new jive_callslot(name, cls, offset);
	if (!slot) {
		free(name);
		delete cls;
		return 0;
	}

	cls->names = &cls->slot;
	cls->slot = slot;
	
	return cls;
}

typedef struct jive_slot_alignment jive_slot_alignment;
struct jive_slot_alignment {
	jive_slot_alignment()
		: cls(nullptr)
	{}

	~jive_slot_alignment() noexcept
	{
		if (cls && !jive_stackslot_size_class_static(cls->size, cls->alignment)) {
			free((char *)cls->name);
			delete cls;
		}

		for (auto i = stackslot_offset_map.begin(); i != stackslot_offset_map.end(); i++) {
			free((char *)i->second->name);
			delete i->second->slot;
			delete i->second;
		}

		for (auto i = callslot_offset_map.begin(); i != callslot_offset_map.end(); i++) {
			free((char *)i->second->name);
			delete i->second->slot;
			delete i->second;
		}
	}

	jive_fixed_stackslot_class *
	lookup_stackslot(ssize_t offset)
	{
		auto i = stackslot_offset_map.find(offset);
		if (i != stackslot_offset_map.end())
			return i->second;
		else
			return nullptr;
	}

	jive_callslot_class *
	lookup_callslot(ssize_t offset)
	{
		auto i = callslot_offset_map.find(offset);
		if (i != callslot_offset_map.end())
			return i->second;
		else
			return nullptr;
	}

	const jive_stackslot_size_class * cls;
	std::unordered_map<ssize_t, jive_fixed_stackslot_class*> stackslot_offset_map;
	std::unordered_map<ssize_t, jive_callslot_class*> callslot_offset_map;
};

typedef struct jive_slot_size jive_slot_size;
struct jive_slot_size {
	jive_slot_alignment *
	create_or_lookup_slot_alignment(size_t alignment)
	{
		auto i = slot_alignment_map.find(alignment);
		if (i != slot_alignment_map.end())
			return &i->second;

		slot_alignment_map[alignment] = jive_slot_alignment();
		return &slot_alignment_map[alignment];
	}

	std::unordered_map<size_t, jive_slot_alignment> slot_alignment_map;
};

typedef struct jive_stackslot_class_map jive_stackslot_class_map;
struct jive_stackslot_class_map {
	jive_stackslot_class_map()
		: lock(PTHREAD_MUTEX_INITIALIZER)
	{}

	jive_slot_size *
	create_or_lookup_slot_size(size_t size)
	{
		auto i = slot_size_map.find(size);
		if (i != slot_size_map.end())
			return &i->second;

		slot_size_map[size] = jive_slot_size();
		return &slot_size_map[size];
	}

	pthread_mutex_t lock;
	std::unordered_map<size_t, jive_slot_size> slot_size_map;
};

static const jive_stackslot_size_class *
jive_stackslot_class_map_lookup_or_create_by_alignment(jive_stackslot_class_map * self, size_t size,
	size_t alignment)
{
	pthread_mutex_lock(&self->lock);
	
	jive_slot_size * slot_size = self->create_or_lookup_slot_size(size);
	jive_slot_alignment * slot_alignment = slot_size->create_or_lookup_slot_alignment(alignment);
	
	if (!slot_alignment->cls) {
		const jive_stackslot_size_class * cls =
			jive_stackslot_size_class_static(size, alignment);
		if (!cls)
			cls = jive_stackslot_size_class_create(size, alignment);

		if (!cls)
			return nullptr;
		slot_alignment->cls = cls;
	}

	pthread_mutex_unlock(&self->lock);

	return slot_alignment->cls;
}

static const jive_fixed_stackslot_class *
jive_stackslot_class_map_lookup_or_create_by_offset(jive_stackslot_class_map * self, size_t size,
	size_t alignment, ssize_t offset)
{
	JIVE_DEBUG_ASSERT((offset & (alignment - 1)) == 0);
	
	pthread_mutex_lock(&self->lock);
	
	jive_slot_size * slot_size = self->create_or_lookup_slot_size(size);
	jive_slot_alignment * slot_alignment = slot_size->create_or_lookup_slot_alignment(alignment);
	
	jive_fixed_stackslot_class * cls = slot_alignment->lookup_stackslot(offset);
	if (!cls) {
		const jive_stackslot_size_class * parent = slot_alignment->cls;
		if (!parent) {
			parent = jive_stackslot_size_class_static(size, alignment);
			if (!parent)
				parent = jive_stackslot_size_class_create(size, alignment);

			if (!parent)
				return nullptr;
			slot_alignment->cls = parent;
		}
		
		cls = jive_fixed_stackslot_class_create(parent, offset);
		if (!cls)
			return nullptr;

		slot_alignment->stackslot_offset_map[offset] = cls;
	}

	pthread_mutex_unlock(&self->lock);

	return cls;
}

static const jive_callslot_class *
jive_callslot_class_map_lookup_or_create_by_offset(jive_stackslot_class_map * self, size_t size,
	size_t alignment, ssize_t offset)
{
	JIVE_DEBUG_ASSERT((offset & (alignment - 1)) == 0);

	pthread_mutex_lock(&self->lock);

	jive_slot_size * slot_size = self->create_or_lookup_slot_size(size);
	jive_slot_alignment * slot_alignment = slot_size->create_or_lookup_slot_alignment(alignment);
	
	jive_callslot_class * cls = slot_alignment->lookup_callslot(offset);
	if (!cls) {
		const jive_stackslot_size_class * parent = slot_alignment->cls;
		if (!parent) {
			parent = jive_stackslot_size_class_static(size, alignment);
			if (!parent)
				parent = jive_stackslot_size_class_create(size, alignment);

			if (!parent)
				return nullptr;
			slot_alignment->cls = parent;
		}
		
		cls = jive_callslot_class_create(parent, offset);
		if (!cls)
			return nullptr;

		slot_alignment->callslot_offset_map[offset] = cls;
	}

	pthread_mutex_unlock(&self->lock);

	return cls;
}

static jive_stackslot_class_map class_map;

const jive_resource_class *
jive_stackslot_size_class_get(size_t size, size_t alignment)
{
	const jive_stackslot_size_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_alignment(&class_map, size, alignment);
	if (cls)
		return cls;
	else
		return 0;
}

const jive_resource_class *
jive_fixed_stackslot_class_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_fixed_stackslot_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_offset(&class_map, size, alignment, offset);
	if (cls)
		return cls;
	else
		return 0;
}

const jive_resource_class *
jive_callslot_class_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_callslot_class * cls;
	cls = jive_callslot_class_map_lookup_or_create_by_offset(&class_map, size, alignment, offset);
	if (cls)
		return cls;
	else
		return 0;
}

const jive_resource_name *
jive_stackslot_name_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_resource_class * rescls = jive_fixed_stackslot_class_get(size, alignment, offset);
	return rescls->names[0];
}
