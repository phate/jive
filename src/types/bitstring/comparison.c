/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace bits {

/* eq_op */

eq_op::~eq_op() noexcept {}

bool
eq_op::operator==(const operation & other) const noexcept
{
	const eq_op * o = dynamic_cast<const eq_op *>(&other);
	return o && o->type() == type();
}
compare_result
eq_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.eq(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
eq_op::flags() const noexcept
{
	return jive_binary_operation_commutative;
}

std::string
eq_op::debug_string() const
{
	return "BITEQUAL";
}

std::unique_ptr<jive::operation>
eq_op::copy() const
{
	return std::unique_ptr<jive::operation>(new eq_op(*this));
}

/* ne_op */

ne_op::~ne_op() noexcept {}

bool
ne_op::operator==(const operation & other) const noexcept
{
	const ne_op * o = dynamic_cast<const ne_op *>(&other);
	return o && o->type() == type();
}
compare_result
ne_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.ne(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ne_op::flags() const noexcept
{
	return jive_binary_operation_commutative;
}

std::string
ne_op::debug_string() const
{
	return "BITNOTEQUAL";
}

std::unique_ptr<jive::operation>
ne_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ne_op(*this));
}

/* sgt_op */

sgt_op::~sgt_op() noexcept {}

bool
sgt_op::operator==(const operation & other) const noexcept
{
	const sgt_op * o = dynamic_cast<const sgt_op *>(&other);
	return o && o->type() == type();
}
compare_result
sgt_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.sgt(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
sgt_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sgt_op::debug_string() const
{
	return "BITSGREATER";
}

std::unique_ptr<jive::operation>
sgt_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sgt_op(*this));
}

/* sge_op */

sge_op::~sge_op() noexcept {}

bool
sge_op::operator==(const operation & other) const noexcept
{
	const sge_op * o = dynamic_cast<const sge_op *>(&other);
	return o && o->type() == type();
}

compare_result
sge_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.sge(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
sge_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sge_op::debug_string() const
{
	return "BITSGREATEREQ";
}

std::unique_ptr<jive::operation>
sge_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sge_op(*this));
}

/* slt_op */

slt_op::~slt_op() noexcept {}

bool
slt_op::operator==(const operation & other) const noexcept
{
	const slt_op * o = dynamic_cast<const slt_op *>(&other);
	return o && o->type() == type();
}
compare_result
slt_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.slt(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
slt_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
slt_op::debug_string() const
{
	return "BITSLESS";
}

std::unique_ptr<jive::operation>
slt_op::copy() const
{
	return std::unique_ptr<jive::operation>(new slt_op(*this));
}

/* sle_op */

sle_op::~sle_op() noexcept {}

bool
sle_op::operator==(const operation & other) const noexcept
{
	const sle_op * o = dynamic_cast<const sle_op *>(&other);
	return o && o->type() == type();
}
compare_result
sle_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.sle(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
sle_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sle_op::debug_string() const
{
	return "BITSLESSEQ";
}

std::unique_ptr<jive::operation>
sle_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sle_op(*this));
}

/* ugt_op */

ugt_op::~ugt_op() noexcept {}

bool
ugt_op::operator==(const operation & other) const noexcept
{
	const ugt_op * o = dynamic_cast<const ugt_op *>(&other);
	return o && o->type() == type();
}
compare_result
ugt_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.ugt(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ugt_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ugt_op::debug_string() const
{
	return "BITUGREATER";
}

std::unique_ptr<jive::operation>
ugt_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ugt_op(*this));
}

/* uge_op */

uge_op::~uge_op() noexcept {}

bool
uge_op::operator==(const operation & other) const noexcept
{
	const uge_op * o = dynamic_cast<const uge_op *>(&other);
	return o && o->type() == type();
}
compare_result
uge_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.uge(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
uge_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
uge_op::debug_string() const
{
	return "BITUGREATEREQ";
}

std::unique_ptr<jive::operation>
uge_op::copy() const
{
	return std::unique_ptr<jive::operation>(new uge_op(*this));
}

/* ult_op */

ult_op::~ult_op() noexcept {}

bool
ult_op::operator==(const operation & other) const noexcept
{
	const ult_op * o = dynamic_cast<const ult_op *>(&other);
	return o && o->type() == type();
}
compare_result
ult_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.ult(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ult_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ult_op::debug_string() const
{
	return "BITULESS";
}

std::unique_ptr<jive::operation>
ult_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ult_op(*this));
}

/* ule_op */

ule_op::~ule_op() noexcept {}

bool
ule_op::operator==(const operation & other) const noexcept
{
	const ule_op * o = dynamic_cast<const ule_op *>(&other);
	return o && o->type() == type();
}
compare_result
ule_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.ule(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ule_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ule_op::debug_string() const
{
	return "BITULESSEQ";
}

std::unique_ptr<jive::operation>
ule_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ule_op(*this));
}

}
}

jive::oport *
jive_biteq(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::eq_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitne(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::ne_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitsgt(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::sgt_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitsge(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::sge_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitslt(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::slt_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitsle(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::sle_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitugt(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::ugt_op(type), {op1, op2})[0];
}

jive::oport *
jive_bituge(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::uge_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitult(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::ult_op(type), {op1, op2})[0];
}

jive::oport *
jive_bitule(size_t nbits, jive::oport * op1, jive::oport * op2)
{
	jive::bits::type type(nbits);
	return jive::create_normalized(op1->region(), jive::bits::ule_op(type), {op1, op2})[0];
}
