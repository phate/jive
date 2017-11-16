/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_REGISTERS_H
#define JIVE_ARCH_REGISTERS_H

#include <jive/rvsdg/resource.h>

namespace jive {
namespace base {
	class type;
}

class register_name : public resource_name {
public:
	virtual
	~register_name();

	inline
	register_name(
		const std::string & name,
		const jive::resource_class * rescls,
		size_t code)
	: resource_name(name, rescls)
	, code_(code)
	{}

	inline size_t
	code() const noexcept
	{
		return code_;
	}

private:
	size_t code_;
};

class register_class : public jive::resource_class {
public:
	virtual
	~register_class();

	inline
	register_class(
		const jive_resource_class_class * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource_name*> & resources,
		const jive::resource_class * parent,
		jive_resource_class_priority priority,
		const std::vector<jive::resource_class_demotion> & demotions,
		const jive::type * type,
		size_t nbits,
		size_t aw,
		size_t lw)
	: jive::resource_class(cls, name, resources, parent, priority, demotions, type)
	, int_arithmetic_width(aw)
	, loadstore_width(lw)
	, nbits_(nbits)
	{}

	inline size_t
	nbits() const noexcept
	{
		return nbits_;
	}

	size_t int_arithmetic_width;
	size_t loadstore_width;

private:
	size_t nbits_;
};

}

extern const jive::resource_class jive_root_register_class;
extern const jive_resource_class_class JIVE_REGISTER_RESOURCE;

#endif
