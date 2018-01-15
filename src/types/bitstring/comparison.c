/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison.h>

namespace jive {
namespace bits {

#define DEFINE_BITCOMPARISON_OPERATION(NAME, FLAGS, DEBUG_STRING) \
NAME ## _op::~NAME ## _op() noexcept \
{} \
\
bool \
NAME ## _op::operator==(const operation & other) const noexcept \
{ \
	auto op = dynamic_cast<const NAME ## _op *>(&other); \
	return op && op->type() == type(); \
} \
\
compare_result \
NAME ## _op::reduce_constants( \
	const value_repr & arg1, \
	const value_repr & arg2) const \
{ \
	switch (arg1.NAME(arg2)) { \
		case '0': return compare_result::static_false; \
		case '1': return compare_result::static_true; \
		default: return compare_result::undecidable; \
	} \
} \
\
enum jive::binary_op::flags \
NAME ## _op::flags() const noexcept \
{ \
	return FLAGS; \
} \
\
std::string \
NAME ## _op::debug_string() const \
{ \
	return jive::detail::strfmt(#DEBUG_STRING, type().nbits()); \
} \
\
std::unique_ptr<jive::operation> \
NAME ## _op::copy() const \
{ \
	return std::unique_ptr<jive::operation>(new NAME ## _op(*this)); \
} \
\
std::unique_ptr<bits::compare_op> \
NAME ## _op::create(size_t nbits) const \
{ \
	return std::unique_ptr<bits::compare_op>(new NAME ## _op(nbits)); \
} \

DEFINE_BITCOMPARISON_OPERATION(eq, jive::binary_op::flags::commutative, BITEQ)
DEFINE_BITCOMPARISON_OPERATION(ne, jive::binary_op::flags::commutative, BITNE)
DEFINE_BITCOMPARISON_OPERATION(sge, jive::binary_op::flags::none, BITSGE)
DEFINE_BITCOMPARISON_OPERATION(sgt, jive::binary_op::flags::none, BITSGT)
DEFINE_BITCOMPARISON_OPERATION(sle, jive::binary_op::flags::none, BITSLE)
DEFINE_BITCOMPARISON_OPERATION(slt, jive::binary_op::flags::none, BITSLT)
DEFINE_BITCOMPARISON_OPERATION(uge, jive::binary_op::flags::none, BITUGE)
DEFINE_BITCOMPARISON_OPERATION(ugt, jive::binary_op::flags::none, BITUGT)
DEFINE_BITCOMPARISON_OPERATION(ule, jive::binary_op::flags::none, BITULE)
DEFINE_BITCOMPARISON_OPERATION(ult, jive::binary_op::flags::none, BITULT)

}
}
