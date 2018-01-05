/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_STACKSLOT_H
#define JIVE_ARCH_STACKSLOT_H

#include <jive/rvsdg/resource.h>

#include <unistd.h>

typedef struct jive_stackframe jive_stackframe;
typedef struct jive_stackframe_class jive_stackframe_class;

typedef struct jive_stackvar_gate jive_stackvar_gate;
typedef struct jive_stackvar_input jive_stackvar_input;
typedef struct jive_stackvar_output jive_stackvar_output;
typedef struct jive_stackvar_type jive_stackvar_type;

extern const jive::resource_class jive_root_stackslot_class;

const jive::resource_class *
jive_stackslot_size_class_get(size_t size, size_t alignment);

const jive::resource_class *
jive_fixed_stackslot_class_get(size_t size, size_t alignment, ssize_t offset);

const jive::resource_class *
jive_callslot_class_get(size_t size, size_t alignment, ssize_t offset);

const jive::resource *
jive_stackslot_name_get(size_t size, size_t alignment, ssize_t offset);

/* resource classes and names*/

class jive_stackslot_size_class : public jive::resource_class {
public:
	virtual
	~jive_stackslot_size_class();

	inline
	jive_stackslot_size_class(
		const jive_resource_class_class * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource*> & resources,
		const jive::resource_class * parent,
		jive_resource_class_priority priority,
		const std::vector<jive::resource_class_demotion> demotions,
		const jive::type * type,
		size_t s,
		size_t a)
	: jive::resource_class(cls, name, resources, parent, priority, demotions, type)
	, size(s)
	, alignment(a)
	{}

	size_t size, alignment;
};

class jive_fixed_stackslot_class : public jive_stackslot_size_class {
public:
	virtual
	~jive_fixed_stackslot_class();

	inline
	jive_fixed_stackslot_class(
		const jive_resource_class_class * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource*> & resources,
		const jive::resource_class * parent,
		jive_resource_class_priority priority,
		const std::vector<jive::resource_class_demotion> & demotions,
		const jive::type * type,
		size_t size,
		size_t alignment,
		const jive::resource * s)
	: jive_stackslot_size_class(cls, name, resources, parent, priority, demotions, type, size,
		alignment)
	, slot(s)
	{}
	
	const jive::resource * slot;
};

class jive_stackslot : public jive::resource {
public:
	virtual
	~jive_stackslot();

	inline
	jive_stackslot(
		const std::string & name,
		const jive::resource_class * rescls,
		int o)
	: jive::resource(name, rescls)
	, offset(o)
	{}

	int offset;
};

class jive_callslot_class : public jive_stackslot_size_class {
public:
	virtual
	~jive_callslot_class();

	inline
	jive_callslot_class(
		const jive_resource_class_class * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource*> & resources,
		const jive::resource_class * parent,
		jive_resource_class_priority priority,
		const std::vector<jive::resource_class_demotion> & demotions,
		const jive::type * type,
		size_t size,
		size_t alignment,
		size_t o,
		const jive::resource * s)
	: jive_stackslot_size_class(cls, name, resources, parent, priority, demotions, type, size,
		alignment)
	, offset(o)
	, slot(s)
	{}

	int offset;
	const jive::resource * slot;
};

class jive_callslot : public jive::resource {
public:
	virtual
	~jive_callslot();

	inline
	jive_callslot(
		const std::string & name,
		const jive::resource_class * rescls,
		int o)
	: jive::resource(name, rescls)
	, offset(o)
	{}

	int offset;
};

extern const jive_resource_class_class JIVE_STACK_RESOURCE;
extern const jive_resource_class_class JIVE_STACK_FRAMESLOT_RESOURCE;
extern const jive_resource_class_class JIVE_STACK_CALLSLOT_RESOURCE;

extern const jive_stackslot_size_class jive_stackslot_class_1_1;
extern const jive_stackslot_size_class jive_stackslot_class_2_2;
extern const jive_stackslot_size_class jive_stackslot_class_4_4;
extern const jive_stackslot_size_class jive_stackslot_class_8_8;
extern const jive_stackslot_size_class jive_stackslot_class_16_16;

#endif
