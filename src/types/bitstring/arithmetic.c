/*
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitand.h>
#include <jive/types/bitstring/arithmetic/bitashr.h>
#include <jive/types/bitstring/arithmetic/bitdifference.h>
#include <jive/types/bitstring/arithmetic/bitnegate.h>
#include <jive/types/bitstring/arithmetic/bitnot.h>
#include <jive/types/bitstring/arithmetic/bitor.h>
#include <jive/types/bitstring/arithmetic/bitproduct.h>
#include <jive/types/bitstring/arithmetic/bitshiproduct.h>
#include <jive/types/bitstring/arithmetic/bitshl.h>
#include <jive/types/bitstring/arithmetic/bitshr.h>
#include <jive/types/bitstring/arithmetic/bitsmod.h>
#include <jive/types/bitstring/arithmetic/bitsquotient.h>
#include <jive/types/bitstring/arithmetic/bitsum.h>
#include <jive/types/bitstring/arithmetic/bituhiproduct.h>
#include <jive/types/bitstring/arithmetic/bitumod.h>
#include <jive/types/bitstring/arithmetic/bituquotient.h>
#include <jive/types/bitstring/arithmetic/bitxor.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

and_op::~and_op() noexcept {}

bool
and_op::operator==(const operation & other) const noexcept
{
	const and_op * o = dynamic_cast<const and_op *>(&other);
	return o && o->type() == type();
}
value_repr
and_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.land(arg2);
}

jive_binary_operation_flags
and_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
and_op::debug_string() const
{
	return "BITAND";
}

std::unique_ptr<jive::operation>
and_op::copy() const
{
	return std::unique_ptr<jive::operation>(new and_op(*this));
}

ashr_op::~ashr_op() noexcept {}

bool
ashr_op::operator==(const operation & other) const noexcept
{
	const ashr_op * o = dynamic_cast<const ashr_op *>(&other);
	return o && o->type() == type();
}
value_repr
ashr_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.ashr(arg2.to_uint());
}

jive_binary_operation_flags
ashr_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ashr_op::debug_string() const
{
	return "BITASHR";
}

std::unique_ptr<jive::operation>
ashr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ashr_op(*this));
}

sub_op::~sub_op() noexcept {}

bool
sub_op::operator==(const operation & other) const noexcept
{
	const sub_op * o = dynamic_cast<const sub_op *>(&other);
	return o && o->type() == type();
}
value_repr
sub_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.sub(arg2);
}

jive_binary_operation_flags
sub_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sub_op::debug_string() const
{
	return "BITDIFFERENCE";
}

std::unique_ptr<jive::operation>
sub_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sub_op(*this));
}

neg_op::~neg_op() noexcept {}

bool
neg_op::operator==(const operation & other) const noexcept
{
	const neg_op * o = dynamic_cast<const neg_op *>(&other);
	return o && o->type() == type();
}
value_repr
neg_op::reduce_constant(
	const value_repr & arg) const
{
	return arg.neg();
}

std::string
neg_op::debug_string() const
{
	return "BITNEGATE";
}

std::unique_ptr<jive::operation>
neg_op::copy() const
{
	return std::unique_ptr<jive::operation>(new neg_op(*this));
}

not_op::~not_op() noexcept {}

bool
not_op::operator==(const operation & other) const noexcept
{
	const not_op * o = dynamic_cast<const not_op *>(&other);
	return o && o->type() == type();
}
value_repr
not_op::reduce_constant(
	const value_repr & arg) const
{
	return arg.lnot();
}

std::string
not_op::debug_string() const
{
	return "BITNOT";
}

std::unique_ptr<jive::operation>
not_op::copy() const
{
	return std::unique_ptr<jive::operation>(new not_op(*this));
}

or_op::~or_op() noexcept {}

bool
or_op::operator==(const operation & other) const noexcept
{
	const or_op * o = dynamic_cast<const or_op *>(&other);
	return o && o->type() == type();
}
value_repr
or_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.lor(arg2);
}

jive_binary_operation_flags
or_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
or_op::debug_string() const
{
	return "BITOR";
}

std::unique_ptr<jive::operation>
or_op::copy() const
{
	return std::unique_ptr<jive::operation>(new or_op(*this));
}

mul_op::~mul_op() noexcept {}

bool
mul_op::operator==(const operation & other) const noexcept
{
	const mul_op * o = dynamic_cast<const mul_op *>(&other);
	return o && o->type() == type();
}
value_repr
mul_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.mul(arg2);
}

jive_binary_operation_flags
mul_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
mul_op::debug_string() const
{
	return "BITPRODUCT";
}

std::unique_ptr<jive::operation>
mul_op::copy() const
{
	return std::unique_ptr<jive::operation>(new mul_op(*this));
}

smulh_op::~smulh_op() noexcept {}

bool
smulh_op::operator==(const operation & other) const noexcept
{
	const smulh_op * o = dynamic_cast<const smulh_op *>(&other);
	return o && o->type() == type();
}
value_repr
smulh_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.smulh(arg2);
}

jive_binary_operation_flags
smulh_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
smulh_op::debug_string() const
{
	return "BITSHIPRODUCT";
}

std::unique_ptr<jive::operation>
smulh_op::copy() const
{
	return std::unique_ptr<jive::operation>(new smulh_op(*this));
}

shl_op::~shl_op() noexcept {}

bool
shl_op::operator==(const operation & other) const noexcept
{
	const shl_op * o = dynamic_cast<const shl_op *>(&other);
	return o && o->type() == type();
}
value_repr
shl_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.shl(arg2.to_uint());
}

jive_binary_operation_flags
shl_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shl_op::debug_string() const
{
	return "BITSHL";
}

std::unique_ptr<jive::operation>
shl_op::copy() const
{
	return std::unique_ptr<jive::operation>(new shl_op(*this));
}

shr_op::~shr_op() noexcept {}

bool
shr_op::operator==(const operation & other) const noexcept
{
	const shr_op * o = dynamic_cast<const shr_op *>(&other);
	return o && o->type() == type();
}
value_repr
shr_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.shr(arg2.to_uint());
}

jive_binary_operation_flags
shr_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shr_op::debug_string() const
{
	return "BITSHR";
}

std::unique_ptr<jive::operation>
shr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new shr_op(*this));
}

smod_op::~smod_op() noexcept {}

bool
smod_op::operator==(const operation & other) const noexcept
{
	const smod_op * o = dynamic_cast<const smod_op *>(&other);
	return o && o->type() == type();
}
value_repr
smod_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.smod(arg2);
}

jive_binary_operation_flags
smod_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
smod_op::debug_string() const
{
	return "BITSMOD";
}

std::unique_ptr<jive::operation>
smod_op::copy() const
{
	return std::unique_ptr<jive::operation>(new smod_op(*this));
}

sdiv_op::~sdiv_op() noexcept {}

bool
sdiv_op::operator==(const operation & other) const noexcept
{
	const sdiv_op * o = dynamic_cast<const sdiv_op *>(&other);
	return o && o->type() == type();
}
value_repr
sdiv_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.sdiv(arg2);
}

jive_binary_operation_flags
sdiv_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sdiv_op::debug_string() const
{
	return "BITSQUOTIENT";
}

std::unique_ptr<jive::operation>
sdiv_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sdiv_op(*this));
}

add_op::~add_op() noexcept {}

bool
add_op::operator==(const operation & other) const noexcept
{
	const add_op * o = dynamic_cast<const add_op *>(&other);
	return o && o->type() == type();
}
value_repr
add_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.add(arg2);
}

jive_binary_operation_flags
add_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
add_op::debug_string() const
{
	return "BITSUM";
}

std::unique_ptr<jive::operation>
add_op::copy() const
{
	return std::unique_ptr<jive::operation>(new add_op(*this));
}

umulh_op::~umulh_op() noexcept {}

bool
umulh_op::operator==(const operation & other) const noexcept
{
	const umulh_op * o = dynamic_cast<const umulh_op *>(&other);
	return o && o->type() == type();
}
value_repr
umulh_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.umulh(arg2);
}

jive_binary_operation_flags
umulh_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
umulh_op::debug_string() const
{
	return "BITUHIPRODUCT";
}

std::unique_ptr<jive::operation>
umulh_op::copy() const
{
	return std::unique_ptr<jive::operation>(new umulh_op(*this));
}

umod_op::~umod_op() noexcept {}

bool
umod_op::operator==(const operation & other) const noexcept
{
	const umod_op * o = dynamic_cast<const umod_op *>(&other);
	return o && o->type() == type();
}
value_repr
umod_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.umod(arg2);
}

jive_binary_operation_flags
umod_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
umod_op::debug_string() const
{
	return "BITUMOD";
}

std::unique_ptr<jive::operation>
umod_op::copy() const
{
	return std::unique_ptr<jive::operation>(new umod_op(*this));
}

udiv_op::~udiv_op() noexcept {}

bool
udiv_op::operator==(const operation & other) const noexcept
{
	const udiv_op * o = dynamic_cast<const udiv_op *>(&other);
	return o && o->type() == type();
}
value_repr
udiv_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.udiv(arg2);
}

jive_binary_operation_flags
udiv_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
udiv_op::debug_string() const
{
	return "BITUQUOTIENT";
}

std::unique_ptr<jive::operation>
udiv_op::copy() const
{
	return std::unique_ptr<jive::operation>(new udiv_op(*this));
}

xor_op::~xor_op() noexcept {}

bool
xor_op::operator==(const operation & other) const noexcept
{
	const xor_op * o = dynamic_cast<const xor_op *>(&other);
	return o && o->type() == type();
}
value_repr
xor_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.lxor(arg2);
}

jive_binary_operation_flags
xor_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
xor_op::debug_string() const
{
	return "BITXOR";
}

std::unique_ptr<jive::operation>
xor_op::copy() const
{
	return std::unique_ptr<jive::operation>(new xor_op(*this));
}

}
}

jive::output *
jive_bitand(size_t noperands, jive::output * const * operands)
{
	jive::region * region = operands[0]->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(region, jive::bits::and_op(type),
		std::vector<jive::oport*>(operands, operands + noperands))[0];
}

jive::output *
jive_bitashr(jive::output * operand, jive::output * shift)
{
	jive::region * region = operand->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operand->type());
	return jive_node_create_normalized(region, jive::bits::ashr_op(type), {operand, shift})[0];
}

jive::output *
jive_bitdifference(jive::output * op1, jive::output * op2)
{
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::sub_op(type), {op1, op2})[0];
}

jive::output *
jive_bitnegate(jive::output * arg)
{
	const auto & type = dynamic_cast<const jive::bits::type &>(arg->type());
	return jive_node_create_normalized(arg->node()->region(), jive::bits::neg_op(type), {arg})[0];
}

jive::output *
jive_bitnot(jive::output * arg)
{
	const auto & type = dynamic_cast<const jive::bits::type &>(arg->type());
	return jive_node_create_normalized(arg->node()->region(), jive::bits::not_op(type), {arg})[0];
}

jive::output *
jive_bitor(size_t noperands, jive::output * const * operands)
{
	jive::region * region = operands[0]->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(region, jive::bits::or_op(type),
		std::vector<jive::oport*>(operands, operands + noperands))[0];
}

jive::output *
jive_bitmultiply(size_t noperands, jive::output * const * operands)
{
	jive::region * region = operands[0]->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(region, jive::bits::mul_op(type),
		std::vector<jive::oport*>(operands, operands + noperands))[0];
}

jive::output *
jive_bitshiproduct(jive::output * op1, jive::output * op2)
{
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::smulh_op(type), {op1, op2})[0];
}

jive::output *
jive_bitshl(jive::output * operand, jive::output * shift)
{
	jive::region * region = operand->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operand->type());
	return jive_node_create_normalized(region, jive::bits::shl_op(type), {operand, shift})[0];
}

jive::output *
jive_bitshr(jive::output * operand, jive::output * shift)
{
	jive::region * region = operand->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operand->type());
	return jive_node_create_normalized(region, jive::bits::shr_op(type), {operand, shift})[0];
}

jive::output *
jive_bitsmod(jive::output * op1, jive::output * op2)
{
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::smod_op(type), {op1, op2})[0];
}

jive::output *
jive_bitsquotient(jive::output * op1, jive::output * op2)
{
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::sdiv_op(type), {op1, op2})[0];
}

jive::output *
jive_bitsum(size_t noperands, jive::output * const * operands)
{
	jive::region * region = operands[0]->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(region, jive::bits::add_op(type),
		std::vector<jive::oport*>(operands, operands + noperands))[0];
}

jive::output *
jive_bituhiproduct(jive::output * op1, jive::output * op2)
{
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::umulh_op(type), {op1, op2})[0];
}

jive::output *
jive_bitumod(jive::output * op1, jive::output * op2)
{
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::umod_op(type), {op1, op2})[0];
}

jive::output *
jive_bituquotient(jive::output * op1, jive::output * op2)
{
	std::vector<jive::output*> operands = {op1, op2};
	jive::region * region = op1->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::udiv_op(type), {op1, op2})[0];
}

jive::output *
jive_bitxor(size_t noperands, jive::output * const * operands)
{
	jive::region * region = operands[0]->node()->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(region, jive::bits::xor_op(type),
		std::vector<jive::oport*>(operands, operands + noperands))[0];
}
