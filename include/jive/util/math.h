/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_MATH_H
#define JIVE_UTIL_MATH_H

#include <jive/common.h>

#include <stdint.h>

JIVE_EXPORTED_INLINE uint64_t
jive_max_unsigned(uint64_t x, uint64_t y)
{
	return x > y ? x : y;
}

JIVE_EXPORTED_INLINE int64_t
jive_max_signed(int64_t x, int64_t y)
{
	return x > y ? x : y;
}

JIVE_EXPORTED_INLINE uint64_t
jive_min_unsigned(uint64_t x, uint64_t y)
{
	return x < y ? x : y;
}

JIVE_EXPORTED_INLINE int64_t
jive_min_signed(int64_t x, int64_t y)
{
	return x < y ? x : y;
}

#endif
