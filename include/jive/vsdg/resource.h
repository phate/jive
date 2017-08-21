/*
 * Copyright 2010 2011 2012 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_RESOURCE_H
#define JIVE_VSDG_RESOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <jive/common.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jive {
namespace base {
	class type;
}
	class gate;
	class resource_name;
}

typedef struct jive_rescls_prio_array jive_rescls_prio_array;
typedef struct jive_resource_class_class jive_resource_class_class;
typedef struct jive_resource_class_count jive_resource_class_count;
typedef struct jive_resource_class_count_bucket jive_resource_class_count_bucket;
typedef struct jive_resource_class_count_item jive_resource_class_count_item;
typedef struct jive_resource_class_demotion jive_resource_class_demotion;

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

class resource_class {
public:
	virtual
	~resource_class();

	inline
	resource_class(
		const jive_resource_class_class * cls,
		const std::string & name,
		const std::unordered_set<const jive::resource_name*> resources,
		const jive::resource_class * parent,
		jive_resource_class_priority pr,
		const jive_resource_class_demotion * dm,
		const jive::base::type * type)
	: class_(cls)
	, priority(pr)
	, demotions(dm)
	, depth_(parent ? parent->depth()+1 : 0)
	, name_(name)
	, type_(type)
	, parent_(parent)
	, resources_(resources)
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

	inline const jive::base::type &
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

	inline const std::unordered_set<const jive::resource_name*> &
	resources() const noexcept
	{
		return resources_;
	}

	const jive_resource_class_class * class_;
	
	/** \brief Priority for register allocator */
	jive_resource_class_priority priority;
	
	/** \brief Paths for "demoting" this resource to a different one */
	const jive_resource_class_demotion * demotions;
	
private:
	/** \brief Number of steps from root resource class */
	size_t depth_;
	std::string name_;

	/** \brief Port and gate type corresponding to this resource */
	const jive::base::type * type_;

	/** \brief Parent resource class */
	const jive::resource_class * parent_;

	/** \brief Available resources */
	std::unordered_set<const jive::resource_name*> resources_;
};

}

struct jive_resource_class_class {
	const jive_resource_class_class * parent;
	const char * name;
	bool is_abstract;
};

struct jive_resource_class_demotion {
	const jive::resource_class * target;
	const jive::resource_class * const * path;
};

const jive::resource_class *
jive_resource_class_union(const jive::resource_class * self, const jive::resource_class * other);

const jive::resource_class *
jive_resource_class_intersection(const jive::resource_class * self,
	const jive::resource_class * other);

static inline bool
jive_resource_class_isinstance(const jive::resource_class * self,
	const jive_resource_class_class * cls)
{
	const jive_resource_class_class * tmp = self->class_;
	while (tmp) {
		if (tmp == cls)
			return true;
		tmp = tmp->parent;
	}
	return false;
}

static inline bool
jive_resource_class_is_abstract(const jive::resource_class * self)
{
	return self->class_->is_abstract;
}

/** \brief Find largest resource class of same general type containing this class */
const jive::resource_class *
jive_resource_class_relax(const jive::resource_class * self);

extern const jive_resource_class_class JIVE_ABSTRACT_RESOURCE;
extern const jive::resource_class jive_root_resource_class;

namespace jive {

class resource_name {
public:
	virtual
	~resource_name();

	inline
	resource_name(const std::string & name, const jive::resource_class * rescls)
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

class jive_resource_class_count {
public:
	typedef std::unordered_map<const jive::resource_class *, size_t> counts_repr;

	inline jive_resource_class_count()
		: counts_(0)
	{
	}

	inline void swap(jive_resource_class_count & other) noexcept
	{
		counts_.swap(other.counts_);
	}

	/* check use counts for identity */
	inline bool
	operator==(const jive_resource_class_count & other) const noexcept
	{
		return counts_ == other.counts_;
	}

	inline bool
	operator!=(const jive_resource_class_count & other) const noexcept
	{
		return ! (*this == other);
	}

	/* clear use counts */
	inline void
	clear()
	{
		counts_.clear();
	}

	/* get use count for given class */
	inline size_t get(const jive::resource_class * cls) const noexcept
	{
		auto i = counts_.find(cls);
		return (i != counts_.end()) ? i->second : 0;
	}

	/* add to use count for given class, report narrowest overflowing class
	 * (or nullptr if no overflow) */
	inline const jive::resource_class *
	add(const jive::resource_class * cls, size_t amount = 1)
	{
		const jive::resource_class * overflow = 0;
		while (cls) {
			size_t count = add_single(cls, amount);
			if (count > cls->nresources() && cls->nresources() && ! overflow) {
				overflow = cls;
			}
			
			cls = cls->parent();
		}
		return overflow;
	}

	/* check if adding use count for class would lead to overflow, report
	 * narrowest class (or nullptr if no overflow) */
	inline const jive::resource_class *
	check_add(const jive::resource_class * cls, size_t amount = 1) const noexcept
	{
		while (cls) {
			size_t count = get(cls);
			if (count + amount > cls->nresources() && cls->nresources()) {
				break;
			}
			
			cls = cls->parent();
		}
		return cls;
	}

	/* subtract use count for class */
	inline void
	sub(const jive::resource_class * cls, size_t amount = 1)
	{
		while (cls) {
			sub_single(cls, amount);
			cls = cls->parent();
		}
	}

	/* same as sub(old_cls) followed by add(new_cls) */
	inline const jive::resource_class *
	change(const jive::resource_class * old_cls, const jive::resource_class * new_cls)
	{
		sub(old_cls);
		return add(new_cls);
	}

	/* check whether a "change" operation would result in overflow (without
	 * actually performing the change) */
	inline const jive::resource_class *
	check_change(
		const jive::resource_class * old_resource_class,
		const jive::resource_class * new_resource_class) const noexcept
	{
		if (!old_resource_class) {
			return check_add(new_resource_class);
		}
		if (!new_resource_class) {
			return nullptr;
		}
	
		const jive::resource_class * common_resource_class =
			jive_resource_class_union(
				old_resource_class,
				new_resource_class);
	
		while (new_resource_class != common_resource_class) {
			size_t count = get(new_resource_class);
			if (count + 1 > new_resource_class->nresources() && new_resource_class->nresources()) {
				return new_resource_class;
			}
			new_resource_class = new_resource_class->parent();
		}
		return nullptr;
	}

	inline void
	update_union(const jive_resource_class_count & other)
	{
		for (const auto & item : other.counts()) {
			counts_repr::iterator i;
			bool was_inserted;
			std::tie(i, was_inserted) = counts_.insert(item);
			if (!was_inserted) {
				i->second = std::max(item.second, i->second);
			}
		}
	}

	inline void
	update_add(const jive_resource_class_count & other)
	{
		for (const auto & item : other.counts()) {
			counts_repr::iterator i;
			bool was_inserted;
			std::tie(i, was_inserted) = counts_.insert(item);
			if (!was_inserted) {
				i->second += item.second;
			}
		}
	}

	inline void
	update_intersection(const jive_resource_class_count & other)
	{
		auto iter = counts_.begin();
		while (iter != counts_.end()) {
			size_t count = std::min(iter->second, other.get(iter->first));
			if (!count) {
				iter = counts_.erase(iter);
			} else {
				iter->second = count;
				++iter;
			}
		}
	}

	/* access to raw counts per class */
	inline const counts_repr & counts() const noexcept { return counts_; }

	/* generate human-readable representation */
	std::string
	debug_string() const;

private:
	inline size_t
	add_single(const jive::resource_class * cls, size_t amount)
	{
		counts_repr::iterator i;
		bool was_inserted;
		std::tie(i, was_inserted) = counts_.insert(std::make_pair(cls, amount));
		if (!was_inserted) {
			i->second += amount;
		}
		return i->second;
	}

	inline void
	sub_single(const jive::resource_class * cls, size_t amount)
	{
		auto i = counts_.find(cls);
		i->second -= amount;
		if (i->second == 0) {
			counts_.erase(i);
		}
	}

	counts_repr counts_;
};

struct jive_rescls_prio_array {
	uint16_t count[8];
};

void
jive_rescls_prio_array_compute(jive_rescls_prio_array * self,
	const jive_resource_class_count * count);

int
jive_rescls_prio_array_compare(const jive_rescls_prio_array * self,
	const jive_rescls_prio_array * other);

static inline void
jive_rescls_prio_array_init(jive_rescls_prio_array * self)
{
	size_t n;
	for (n = 0; n < 8; n++)
		self->count[n] = 0;
}

#endif
