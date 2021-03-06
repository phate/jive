/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_STACKSLOT_HPP
#define JIVE_ARCH_STACKSLOT_HPP

#include <jive/rvsdg/resource.hpp>

#include <unistd.h>

extern const jive::resource_type stack_resource;
extern const jive::resource_type callslot_resource;
extern const jive::resource_type frameslot_resource;

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
		const jive::resource_type * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource*> & resources,
		const jive::resource_class * parent,
		enum jive::resource_class::priority priority,
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
		const std::string & name,
		const std::unordered_set<const jive::resource*> & resources,
		const jive::resource_class * parent,
		enum jive::resource_class::priority priority,
		const std::vector<jive::resource_class_demotion> & demotions,
		const jive::type * type,
		size_t size,
		size_t alignment,
		const jive::resource * s)
	: jive_stackslot_size_class(&frameslot_resource, name, resources, parent, priority, demotions,
		type, size, alignment)
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
		const std::string & name,
		const std::unordered_set<const jive::resource*> & resources,
		const jive::resource_class * parent,
		enum jive::resource_class::priority priority,
		const std::vector<jive::resource_class_demotion> & demotions,
		const jive::type * type,
		size_t size,
		size_t alignment,
		size_t o,
		const jive::resource * s)
	: jive_stackslot_size_class(&callslot_resource, name, resources, parent, priority, demotions,
		type, size, alignment)
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

extern const jive_stackslot_size_class jive_stackslot_class_1_1;
extern const jive_stackslot_size_class jive_stackslot_class_2_2;
extern const jive_stackslot_size_class jive_stackslot_class_4_4;
extern const jive_stackslot_size_class jive_stackslot_class_8_8;
extern const jive_stackslot_size_class jive_stackslot_class_16_16;

#endif
