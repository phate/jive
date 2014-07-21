#ifndef JIVE_TESTS_TYPES_BITSTRING_COMPARISON_BITCMP_TEST_HELPERS_H
#define JIVE_TESTS_TYPES_BITSTRING_COMPARISON_BITCMP_TEST_HELPERS_H

#include <jive/vsdg/control.h>

static inline void
expect_static_true(jive::output * output)
{
	assert(dynamic_cast<const jive::ctl::constant_op &>(output->node()->operation()).value() == true);
}

static inline void
expect_static_false(jive::output * output)
{
	assert(dynamic_cast<const jive::ctl::constant_op &>(output->node()->operation()).value() == false);
}

#endif
