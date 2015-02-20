/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H
#define JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H

#include <jive/common.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace jive {
namespace bits {

/**
 Value representation used for compile-time evaluation of bitstring
 expressions. A bit is either:
	- '0' : zero
	- '1' : one
	- 'D' : defined, but unknown
	- 'X' : undefined and unknown
*/

class value_repr {
public:
	inline
	value_repr(size_t nbits, int64_t value)
	{
		if (nbits == 0)
			throw compiler_error("Number of bits is zero.");

		if (nbits < 64 && (value >> nbits) != 0 && (value >> nbits != -1))
			throw compiler_error("Value cannot be represented with the given number of bits.");

		for (size_t n = 0; n < nbits; ++n) {
			data_.push_back('0' + (value & 1));
			value = value >> 1;
		}
	}

	inline
	value_repr(const char * s)
	{
		if (strlen(s) == 0)
			throw compiler_error("Number of bits is zero.");

		for (size_t n = 0; n < strlen(s); n++) {
			if (s[n] != '0' && s[n] != '1' && s[n] != 'X' && s[n] != 'D')
				throw compiler_error("Not a valid bit.");
			data_.push_back(s[n]);
		}
	}

	inline
	value_repr(const value_repr & other)
		: data_(other.data_)
	{}

	inline
	value_repr(value_repr && other) noexcept
		: data_(std::move(other.data_))
	{}

	inline static value_repr
	repeat(size_t nbits, char bit)
	{
		return value_repr(std::string(nbits, bit).c_str());
	}

	inline value_repr &
	operator=(const value_repr & other)
	{
		data_ = other.data_;
		return *this;
	}

	inline char &
	operator[](size_t n)
	{
		JIVE_DEBUG_ASSERT(n < nbits());
		return data_[n];
	}

	inline const char &
	operator[](size_t n) const
	{
		JIVE_DEBUG_ASSERT(n < nbits());
		return data_[n];
	}

	inline bool
	operator==(const jive::bits::value_repr & other) const noexcept
	{
		return data_ == other.data_;
	}

	inline bool
	operator!=(const jive::bits::value_repr & other) const noexcept
	{
		return !(*this == other);
	}

	void
	zext(size_t nbits)
	{
		data_.insert(data_.end(), nbits, '0');
	}

	void
	sext(size_t nbits)
	{
		data_.insert(data_.end(), nbits, data_[this->nbits()-1]);
	}

	inline size_t
	nbits() const noexcept
	{
		return data_.size();
	}

	inline std::string
	str() const
	{
		return std::string(data_.begin(), data_.end());
	}

	uint64_t
	to_uint() const;

	int64_t
	to_int() const;

private:
	/* [lsb ... msb] */
	std::vector<char> data_;
};

}
}

#endif
