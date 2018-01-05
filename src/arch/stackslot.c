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

#include <jive/arch/addresstype.h>
#include <jive/common.h>
#include <jive/util/strfmt.h>

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

const jive::resource_class_class root_stack_resource_class_class(
	false, "stack", &root_resource_class_class);

const jive::resource_class_class stack_frameslot_resource(
	false, "stack_frameslot", &root_stack_resource_class_class);

const jive::resource_class_class stack_callslot_resource(
	false, "stack_callslot", &root_stack_resource_class_class);

static const jive::memtype stackvar_type;

const jive::resource_class jive_root_stackslot_class(
	&root_resource_class_class, "stackslot", {},
	&jive_root_resource_class, jive_resource_class_priority_lowest,
	{}, nullptr);

#define MAKE_STACKSLOT_CLASS(SIZE, ALIGNMENT) \
const jive_stackslot_size_class jive_stackslot_class_##SIZE##_##ALIGNMENT(\
	&root_stack_resource_class_class, "stack_s" #SIZE "a" #ALIGNMENT, {}, \
	&jive_root_stackslot_class, jive_resource_class_priority_mem_generic,\
	{}, &stackvar_type, SIZE, ALIGNMENT \
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
	auto name = jive::detail::strfmt("stack_s", size, "a", alignment);
	return new jive_stackslot_size_class(&root_stack_resource_class_class, name, {},
		&jive_root_stackslot_class, jive_resource_class_priority_mem_generic,
		{}, &stackvar_type, size, alignment);
}

jive_stackslot *
jive_stackslot_create(const jive::resource_class * parent, long offset);

static jive_fixed_stackslot_class *
jive_fixed_stackslot_class_create(const jive_stackslot_size_class * parent, int offset)
{
	auto name = jive::detail::strfmt("stack_s", parent->size, "a", parent->alignment, "@", offset);
	
	auto slot = new jive_stackslot(name, nullptr, offset);
	if (!slot)
		return nullptr;

	auto cls = new jive_fixed_stackslot_class(&stack_frameslot_resource, name, {slot},
		parent, jive_resource_class_priority_mem_unique, {}, &stackvar_type, parent->size,
		parent->alignment, slot);
	if (!cls) {
		delete slot;
		return nullptr;
	}

	slot->resource_class = cls;

	return cls;
}

static jive_callslot_class *
jive_callslot_class_create(const jive_stackslot_size_class * parent, int offset)
{
	auto name = jive::detail::strfmt("call_s", parent->size, "a", parent->alignment, "@", offset);

	auto slot = new jive_callslot(name, nullptr, offset);
	if (!slot)
		return nullptr;

	auto cls = new jive_callslot_class(&stack_callslot_resource, name, {slot}, parent,
		jive_resource_class_priority_mem_unique, {}, &stackvar_type, parent->size,
		parent->alignment, offset, slot);
	if (!cls) {
		delete slot;
		return nullptr;
	}

	slot->resource_class = cls;
	
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
			delete cls;
		}

		for (auto i = stackslot_offset_map.begin(); i != stackslot_offset_map.end(); i++) {
			delete i->second->slot;
			delete i->second;
		}

		for (auto i = callslot_offset_map.begin(); i != callslot_offset_map.end(); i++) {
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

const jive::resource_class *
jive_stackslot_size_class_get(size_t size, size_t alignment)
{
	const jive_stackslot_size_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_alignment(&class_map, size, alignment);
	if (cls)
		return cls;
	else
		return 0;
}

const jive::resource_class *
jive_fixed_stackslot_class_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_fixed_stackslot_class * cls;
	cls = jive_stackslot_class_map_lookup_or_create_by_offset(&class_map, size, alignment, offset);
	if (cls)
		return cls;
	else
		return 0;
}

const jive::resource_class *
jive_callslot_class_get(size_t size, size_t alignment, ssize_t offset)
{
	const jive_callslot_class * cls;
	cls = jive_callslot_class_map_lookup_or_create_by_offset(&class_map, size, alignment, offset);
	if (cls)
		return cls;
	else
		return 0;
}

const jive::resource *
jive_stackslot_name_get(size_t size, size_t alignment, ssize_t offset)
{
	auto rescls = jive_fixed_stackslot_class_get(size, alignment, offset);
	return *rescls->resources().begin();
}
