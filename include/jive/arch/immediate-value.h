/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_IMMEDIATE_VALUE_H
#define JIVE_ARCH_IMMEDIATE_VALUE_H

#include <jive/vsdg/label.h>

typedef uint64_t jive_immediate_int;

namespace jive {

/* immediates, as represented in the graph */

class immediate final {
public:
	inline constexpr
	immediate(
		jive_immediate_int offset = 0,
		const struct jive_label * add_label = nullptr,
		const struct jive_label * sub_label = nullptr,
		const void * modifier = nullptr) noexcept
	: offset_(offset)
	, add_label_(add_label)
	, sub_label_(sub_label)
	, modifier_(modifier)
	{}

	inline immediate &
	operator=(const immediate & other) noexcept
	{
		offset_ = other.offset_;
		add_label_ = other.add_label_;
		sub_label_ = other.sub_label_;
		modifier_ = other.modifier_;
		return *this;
	}

	inline immediate &
	operator+(const immediate & other)
	{
		const struct jive_label * add1 = add_label_;
		const struct jive_label * add2 = other.add_label_;
		const struct jive_label * sub1 = sub_label_;
		const struct jive_label * sub2 = other.sub_label_;

		if (add1 == sub2) {
			add1 = nullptr;
			sub2 = nullptr;
		}

		if (add2 && sub1) {
			add2 = nullptr;
			sub1 = nullptr;
		}

		if ((add1 && add2) || (sub1 && sub2) || (modifier_ || other.modifier_))
			throw compiler_error("Cannot add immediates");

		offset_ += other.offset_;
		add_label_ = add1 ? add1 : add2;
		sub_label_ = sub1 ? sub1 : sub2;
		modifier_ = nullptr;
	
		return *this;
	}

	inline immediate &
	operator+(jive_immediate_int offset) noexcept
	{
		offset_ += offset;
		return *this;
	}

	inline immediate &
	operator+=(const immediate & other)
	{
		*this = *this + other;
		return *this;
	}

	inline immediate &
	operator+=(uint64_t offset)
	{
		*this = *this + offset;
		return *this;
	}

	inline immediate &
	operator-(const immediate & other)
	{
		const struct jive_label * add1 = add_label_;
		const struct jive_label * add2 = other.sub_label_;
		const struct jive_label * sub1 = sub_label_;
		const struct jive_label * sub2 = other.add_label_;

		if (add1 == sub2) {
			add1 = nullptr;
			sub2 = nullptr;
		}

		if (add2 && sub1) {
			add2 = nullptr;
			sub1 = nullptr;
		}

		if ((add1 && add2) || (sub1 && sub2) || (modifier_ || other.modifier_))
			throw compiler_error("Cannot subtract immediates");

		offset_ -= other.offset_;
		add_label_ = add1 ? add1 : add2;
		sub_label_ = sub1 ? sub1 : sub2;
		modifier_ = nullptr;

		return *this;
	}

	inline bool
	operator==(const immediate & other) const noexcept
	{
		return
			offset_ == other.offset_ &&
			add_label_ == other.add_label_ &&
			sub_label_ == other.sub_label_ &&
			modifier_ == other.modifier_;
	}

	inline bool
	operator==(jive_immediate_int offset) const noexcept
	{
		return
			offset_ == offset &&
			add_label_ == nullptr &&
			sub_label_ == nullptr &&
			modifier_ == nullptr;
	}

	inline bool
	operator!=(const immediate & other) const noexcept
	{
		return !(*this == other);
	}

	inline bool
	operator!=(jive_immediate_int offset) const noexcept
	{
		return !(*this == offset);
	}

	bool
	has_add_label() const noexcept
	{
		return add_label_ != nullptr;
	}

	bool
	has_sub_label() const noexcept
	{
		return sub_label_ != nullptr;
	}

	bool
	has_modifier() const noexcept
	{
		return modifier_ != nullptr;
	}

	bool
	has_symbols() const noexcept
	{
		return has_add_label() || has_sub_label() || has_modifier();
	}

	inline jive_immediate_int
	offset() const noexcept
	{
		return offset_;
	}

	void
	set_offset(uint64_t offset) noexcept
	{
		offset_ = offset;
	}

	const struct jive_label *
	add_label() const noexcept
	{
		return add_label_;
	}

	void
	set_add_label(const struct jive_label * add_label) noexcept
	{
		add_label_ = add_label;
	}

	const struct jive_label *
	sub_label() const noexcept
	{
		return sub_label_;
	}

	void
	set_sub_label(const struct jive_label * sub_label) noexcept
	{
		sub_label_ = sub_label;
	}

	const void *
	modifier() const noexcept
	{
		return modifier_;
	}

	void
	set_modifier(const void * modifier) noexcept
	{
		modifier_ = modifier;
	}

private:
	jive_immediate_int offset_;
	const struct jive_label * add_label_;
	const struct jive_label * sub_label_;
	const void * modifier_;
};

}

#endif
