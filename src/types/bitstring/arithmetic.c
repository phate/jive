/*
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/region.h>
#include <jive/util/strfmt.h>

namespace jive {
namespace bits {

#define DEFINE_BITUNARY_OPERATION(NAME, REDUCTION, DEBUG_STRING) \
NAME ## _op::~NAME ## _op() noexcept \
{} \
\
bool \
NAME ## _op::operator==(const operation & other) const noexcept \
{ \
	auto op = dynamic_cast<const NAME ## _op*>(&other); \
	return op && op->type() == type(); \
} \
\
value_repr \
NAME ## _op::reduce_constant(const value_repr & arg) const \
{ \
	return REDUCTION; \
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
std::unique_ptr<bits::unary_op> \
NAME ## _op::create(size_t nbits) const \
{ \
	return std::unique_ptr<bits::unary_op>(new NAME ## _op(nbits)); \
} \

#define DEFINE_BITBINARY_OPERATION(NAME, REDUCTION, DEBUG_STRING, FLAGS) \
NAME ## _op::~NAME ## _op() noexcept \
{} \
\
bool \
NAME ## _op::operator==(const operation & other) const noexcept \
{ \
	auto op = dynamic_cast<const NAME ## _op*>(&other); \
	return op && op->type() == type(); \
} \
\
value_repr \
NAME ## _op::reduce_constants( \
	const value_repr & arg1, \
	const value_repr & arg2) const \
{ \
	return REDUCTION; \
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
	return jive::detail::strfmt(#DEBUG_STRING, type().nbits()); \
} \
\
std::unique_ptr<jive::operation> \
NAME ## _op::copy() const \
{ \
	return std::unique_ptr<jive::operation>(new NAME ## _op(*this)); \
} \
\
std::unique_ptr<bits::binary_op> \
NAME ## _op::create(size_t nbits) const \
{ \
	return std::unique_ptr<bits::binary_op>(new NAME ## _op(nbits)); \
} \

DEFINE_BITUNARY_OPERATION(neg, arg.neg(), BITNEGATE);
DEFINE_BITUNARY_OPERATION(not, arg.lnot(), BITNOT);

DEFINE_BITBINARY_OPERATION(add, arg1.add(arg2), BITADD,
	jive_binary_operation_associative | jive_binary_operation_commutative);
DEFINE_BITBINARY_OPERATION(and, arg1.land(arg2), BITAND,
	jive_binary_operation_associative | jive_binary_operation_commutative);
DEFINE_BITBINARY_OPERATION(ashr, arg1.ashr(arg2.to_uint()), BITASHR, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(mul, arg1.mul(arg2), BITMUL,
	jive_binary_operation_associative | jive_binary_operation_commutative);
DEFINE_BITBINARY_OPERATION(or, arg1.lor(arg2), BITOR,
	jive_binary_operation_associative | jive_binary_operation_commutative);
DEFINE_BITBINARY_OPERATION(sdiv, arg1.sdiv(arg2), BITSDIV, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(shl, arg1.shl(arg2.to_uint()), BITSHL, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(shr, arg1.shr(arg2.to_uint()), BITSHR, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(smod, arg1.smod(arg2), BITSMOD, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(smulh, arg1.smulh(arg2), BITSMULH, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(sub, arg1.sub(arg2), BITSUB, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(udiv, arg1.udiv(arg2), BITUDIV, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(umod, arg1.umod(arg2), BITUMOD, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(umulh, arg1.umulh(arg2), BITUMULH, jive_binary_operation_none);
DEFINE_BITBINARY_OPERATION(xor, arg1.lxor(arg2), BITXOR,
	jive_binary_operation_associative | jive_binary_operation_commutative);

}
}
