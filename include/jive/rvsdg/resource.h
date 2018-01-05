/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_RVSDG_RESOURCE_H
#define JIVE_RVSDG_RESOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <jive/common.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef enum {
	jive_resource_class_priority_invalid = 0,
	jive_resource_class_priority_control = 1,
	jive_resource_class_priority_reg_implicit = 2,
	jive_resource_class_priority_mem_unique = 3,
	jive_resource_class_priority_reg_high = 4,
	jive_resource_class_priority_reg_low = 5,
	jive_resource_class_priority_mem_generic = 6,
	jive_resource_class_priority_lowest = 7
} jive_resource_class_priority;

namespace jive {

class gate;
class resource;
class resource_class;
class type;

/* resource_type */

class resource_type {
public:
	virtual
	~resource_type();

	inline
	resource_type(
		bool is_abstract,
		const std::string & name,
		const resource_type * parent)
	: name_(name)
	, is_abstract_(is_abstract)
	, parent_(parent)
	{}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	inline const resource_type *
	parent() const noexcept
	{
		return parent_;
	}

	inline bool
	is_abstract() const noexcept
	{
		return is_abstract_;
	}

private:
	std::string name_;
	bool is_abstract_;
	const resource_type * parent_;
};

extern const jive::resource_type root_resource;

/* resource_class_demotion */

class resource_class_demotion final {
public:
	inline
	resource_class_demotion(
		const resource_class * target,
		const std::vector<const resource_class*> & path)
	: target_(target)
	, path_(path)
	{}

	inline const resource_class *
	target() const noexcept
	{
		return target_;
	}

	inline const std::vector<const resource_class*>
	path() const noexcept
	{
		return path_;
	}

private:
	const resource_class * target_;
	std::vector<const resource_class*> path_;
};

class resource_class {
public:
	virtual
	~resource_class();

	inline
	resource_class(
		const jive::resource_type * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource*> resources,
		const jive::resource_class * parent,
		jive_resource_class_priority pr,
		const std::vector<resource_class_demotion> & demotions,
		const jive::type * type)
	: class_(cls)
	, priority(pr)
	, depth_(parent ? parent->depth()+1 : 0)
	, name_(name)
	, type_(type)
	, parent_(parent)
	, resources_(resources)
	, demotions_(demotions)
	{}

	inline size_t
	depth() const noexcept
	{
		return depth_;
	}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	inline const jive::type &
	type() const noexcept
	{
		JIVE_DEBUG_ASSERT(type_ != nullptr);
		return *type_;
	}

	inline const jive::resource_class *
	parent() const noexcept
	{
		return parent_;
	}

	inline size_t
	nresources() const noexcept
	{
		return resources_.size();
	}

	inline const std::unordered_set<const jive::resource*> &
	resources() const noexcept
	{
		return resources_;
	}

	inline const std::vector<resource_class_demotion> &
	demotions() const noexcept
	{
		return demotions_;
	}

	const jive::resource_type * class_;
	
	/** \brief Priority for register allocator */
	jive_resource_class_priority priority;
	
private:
	/** \brief Number of steps from root resource class */
	size_t depth_;
	std::string name_;

	/** \brief Port and gate type corresponding to this resource */
	const jive::type * type_;

	/** \brief Parent resource class */
	const jive::resource_class * parent_;

	/** \brief Available resources */
	std::unordered_set<const jive::resource*> resources_;

	/** \brief Paths for "demoting" this resource to a different one */
	std::vector<resource_class_demotion> demotions_;
};

}

const jive::resource_class *
jive_resource_class_union(const jive::resource_class * self, const jive::resource_class * other);

const jive::resource_class *
jive_resource_class_intersection(const jive::resource_class * self,
	const jive::resource_class * other);

static inline bool
jive_resource_class_isinstance(
	const jive::resource_class * self,
	const jive::resource_type * cls)
{
	auto tmp = self->class_;
	while (tmp) {
		if (tmp == cls)
			return true;
		tmp = tmp->parent();
	}
	return false;
}

static inline bool
jive_resource_class_is_abstract(const jive::resource_class * self)
{
	return self->class_->is_abstract();
}

/** \brief Find largest resource class of same general type containing this class */
const jive::resource_class *
jive_resource_class_relax(const jive::resource_class * self);

extern const jive::resource_class jive_root_resource_class;

namespace jive {

class resource {
public:
	virtual
	~resource();

	inline
	resource(const std::string & name, const jive::resource_class * rescls)
	: resource_class(rescls)
	, name_(name)
	{}

	inline const std::string &
	name() const noexcept
	{
		return name_;
	}

	const jive::resource_class * resource_class;
private:
	std::string name_;
};

}

#endif
