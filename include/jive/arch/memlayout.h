/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_MEMLAYOUT_H
#define JIVE_ARCH_MEMLAYOUT_H

#include <jive/common.h>
#include <jive/vsdg/basetype.h>

#include <memory>
#include <vector>

namespace jive {

namespace rcd {
	struct declaration;
}
namespace unn {
	struct declaration;
}
namespace value {
	class type;
}

class dataitem_memlayout {
public:
	virtual
	~dataitem_memlayout();

	inline
	dataitem_memlayout(size_t size, size_t alignment) noexcept
		: size_(size)
		, alignment_(alignment)
	{}

	inline size_t
	size() const noexcept
	{
		return size_;
	}

	inline size_t
	alignment() const noexcept
	{
		return alignment_;
	}

private:
	size_t size_;
	size_t alignment_;
	/* FIXME: endianess? */
};

class union_memlayout final : public dataitem_memlayout {
public:
	virtual
	~union_memlayout();

	inline
	union_memlayout(const struct unn::declaration * decl, size_t size, size_t alignment) noexcept
		: dataitem_memlayout(size, alignment)
		, decl_(decl)
	{}

	inline const unn::declaration *
	declaration() const noexcept
	{
		return decl_;
	}

private:
	const struct unn::declaration * decl_;
};

class record_memlayout_element {
public:
	inline
	record_memlayout_element(size_t size, size_t offset) noexcept
		: size_(size)
		, offset_(offset)
	{}

	inline size_t
	size() const noexcept
	{
		return size_;
	}

	inline size_t
	offset() const noexcept
	{
		return offset_;
	}

private:
	size_t size_;
	size_t offset_;
};

class record_memlayout final : public dataitem_memlayout {
public:
	virtual
	~record_memlayout();

	record_memlayout(
	std::shared_ptr<const rcd::declaration> & decl,
	const std::vector<record_memlayout_element> & elements,
	size_t size,
	size_t alignment) noexcept;

	inline std::shared_ptr<const rcd::declaration>
	declaration() const noexcept
	{
		return decl_;
	}

	inline size_t
	nelements() const noexcept
	{
		return elements_.size();
	}

	inline const record_memlayout_element &
	element(size_t index) const noexcept
	{
		JIVE_DEBUG_ASSERT(index < nelements());
		return elements_[index];
	}

	inline const std::vector<record_memlayout_element> &
	elements() const noexcept
	{
		return elements_;
	}

private:
	std::shared_ptr<const rcd::declaration>  decl_;
	std::vector<record_memlayout_element> elements_;
};

class memlayout_mapper {
public:
	virtual
	~memlayout_mapper();

	inline constexpr
	memlayout_mapper(size_t bytes_per_word)
		: bytes_per_word_(bytes_per_word)
	{}

	inline size_t
	bytes_per_word() const noexcept
	{
		return bytes_per_word_;
	}

	inline size_t
	bits_per_word() const noexcept
	{
		return bytes_per_word() * 8;
	}

	virtual const record_memlayout &
	map_record(std::shared_ptr<const rcd::declaration> & decl) = 0;

	virtual const union_memlayout &
	map_union(const struct unn::declaration * decl) = 0;

	virtual const dataitem_memlayout &
	map_bitstring(size_t nbits) = 0;

	virtual const dataitem_memlayout &
	map_address() = 0;

	const dataitem_memlayout &
	map_value_type(const value::type & type);

private:
	size_t bytes_per_word_;
};

}

#endif
