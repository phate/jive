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
jive_binary_operation_flags \
NAME ## _op::flags() const noexcept \
{ \
	return FLAGS; \
} \
\
std::string \
NAME ## _op::debug_string() const \
{ \
	return #DEBUG_STRING; \
} \
\
std::unique_ptr<jive::operation> \
NAME ## _op::copy() const \
{ \
	return std::unique_ptr<jive::operation>(new NAME ## _op(*this)); \
} \

DEFINE_BITCOMPARISON_OPERATION(eq, jive_binary_operation_commutative, BITEQ);
DEFINE_BITCOMPARISON_OPERATION(ne, jive_binary_operation_commutative, BITNE);
DEFINE_BITCOMPARISON_OPERATION(sge, jive_binary_operation_none, BITSGE);
DEFINE_BITCOMPARISON_OPERATION(sgt, jive_binary_operation_none, BITSGT);
DEFINE_BITCOMPARISON_OPERATION(sle, jive_binary_operation_none, BITSLE);
DEFINE_BITCOMPARISON_OPERATION(slt, jive_binary_operation_none, BITSLT);
DEFINE_BITCOMPARISON_OPERATION(uge, jive_binary_operation_none, BITUGE);
DEFINE_BITCOMPARISON_OPERATION(ugt, jive_binary_operation_none, BITUGT);
DEFINE_BITCOMPARISON_OPERATION(ule, jive_binary_operation_none, BITULE);
DEFINE_BITCOMPARISON_OPERATION(ult, jive_binary_operation_none, BITULT);

}
}
