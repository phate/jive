/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TYPES_BITSTRING_COMPARISON_BITCMP_TEST_HELPERS_H
#define JIVE_TESTS_TYPES_BITSTRING_COMPARISON_BITCMP_TEST_HELPERS_H

#include <jive/types/bitstring/constant.h>

static inline void
expect_static_true(jive::output * output)
{
	const jive::bits::constant_op * op;
	op = dynamic_cast<const jive::bits::constant_op*>(&output->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "1");
}

static inline void
expect_static_false(jive::output * output)
{
	const jive::bits::constant_op * op;
	op = dynamic_cast<const jive::bits::constant_op*>(&output->node()->operation());
	assert(op && op->value().nbits() == 1 && op->value().str() == "0");
}

#endif
