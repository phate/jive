/*
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
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

jive::oport *
jive_bitand(size_t nbits, const std::vector<jive::oport*> & operands)
{
	return jive_node_create_normalized(operands[0]->region(), jive::bits::and_op(nbits),
		operands)[0];
}

jive::oport *
jive_bitashr(size_t nbits, jive::oport * operand, jive::oport * shift)
{
	return jive_node_create_normalized(operand->region(), jive::bits::ashr_op(nbits),
		{operand, shift})[0];
}

jive::oport *
jive_bitdifference(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::sub_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bitnegate(size_t nbits, jive::oport * arg)
{
	return jive_node_create_normalized(arg->region(), jive::bits::neg_op(nbits), {arg})[0];
}

jive::oport *
jive_bitnot(size_t nbits, jive::oport * arg)
{
	return jive_node_create_normalized(arg->region(), jive::bits::not_op(nbits), {arg})[0];
}

jive::oport *
jive_bitor(size_t nbits, const std::vector<jive::oport*> & operands)
{
	return jive_node_create_normalized(operands[0]->region(), jive::bits::or_op(nbits), operands)[0];
}

jive::oport *
jive_bitmultiply(size_t nbits, const std::vector<jive::oport*> & operands)
{
	return jive_node_create_normalized(operands[0]->region(), jive::bits::mul_op(nbits),
		operands)[0];
}

jive::oport *
jive_bitshiproduct(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::smulh_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bitshl(size_t nbits, jive::oport * operand, jive::oport * shift)
{
	return jive_node_create_normalized(operand->region(), jive::bits::shl_op(nbits),
		{operand, shift})[0];
}

jive::oport *
jive_bitshr(size_t nbits, jive::oport * operand, jive::oport * shift)
{
	return jive_node_create_normalized(operand->region(), jive::bits::shr_op(nbits),
		{operand, shift})[0];
}

jive::oport *
jive_bitsmod(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::smod_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bitsquotient(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::sdiv_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bitsum(size_t nbits, const std::vector<jive::oport*> & operands)
{
	return jive_node_create_normalized(operands[0]->region(), jive::bits::add_op(nbits),
		operands)[0];
}

jive::oport *
jive_bituhiproduct(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::umulh_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bitumod(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::umod_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bituquotient(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	return jive_node_create_normalized(op1->region(), jive::bits::udiv_op(nbits), {op1, op2})[0];
}

jive::oport *
jive_bitxor(size_t nbits, const std::vector<jive::oport*> & operands)
{
	return jive_node_create_normalized(operands[0]->region(), jive::bits::xor_op(nbits),
		operands)[0];
}
