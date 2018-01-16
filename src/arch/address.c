/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.h>

#include <jive/arch/addresstype.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/region.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>

/* memberof */

namespace jive {

memberof_op::~memberof_op() noexcept
{
}

bool
memberof_op::operator==(const operation & other) const noexcept
{
	const memberof_op * op =
		dynamic_cast<const memberof_op *>(&other);

	return op && op->record_decl() == record_decl() && op->index() == index();
}
std::string
memberof_op::debug_string() const
{
	return detail::strfmt("MEMBEROF", record_decl(), index());
}

const jive::port &
memberof_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	static const jive::port p(addrtype::instance());
	return p;
}

const jive::port &
memberof_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	static const jive::port p(addrtype::instance());
	return p;
}

jive_unop_reduction_path_t
memberof_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::simple_output*>(arg);
	if (!tmp)
		return jive_unop_reduction_none;

	auto op = dynamic_cast<const containerof_op *>(&tmp->node()->operation());
	if (!op)
		return jive_unop_reduction_none;

	if (op->record_decl() == record_decl() && op->index() == index()) {
		return jive_unop_reduction_inverse;
	}

	return jive_unop_reduction_none;
}

jive::output *
memberof_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(0)->origin();
	
	return nullptr;
}

std::unique_ptr<jive::operation>
memberof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new memberof_op(*this));
}

}


jive::output *
jive_memberof(
	jive::output * address,
	const jive::rcddeclaration * dcl,
	size_t index)
{
	jive::memberof_op op(dcl, index);
	return jive::create_normalized(address->region(), op, {address})[0];
}

/* containerof */

namespace jive {

containerof_op::~containerof_op() noexcept
{
}

bool
containerof_op::operator==(const operation & other) const noexcept
{
	const containerof_op * op =
		dynamic_cast<const containerof_op *>(&other);
	return op && op->record_decl() == record_decl() && op->index() == index();
}
std::string
containerof_op::debug_string() const
{
	return detail::strfmt("CONTAINEROF", record_decl(), index());
}

const jive::port &
containerof_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	static const jive::port p(addrtype::instance());
	return p;
}

const jive::port &
containerof_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const jive::port p(addrtype::instance());
	return p;
}

jive_unop_reduction_path_t
containerof_op::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::simple_output*>(arg);
	if (!tmp)
		return jive_unop_reduction_none;

	auto op = dynamic_cast<const memberof_op *>(&tmp->node()->operation());
	if (!op)
		return jive_unop_reduction_none;

	if (op->record_decl() == record_decl() && op->index() == index())
		return jive_unop_reduction_inverse;

	return jive_unop_reduction_none;
}

jive::output *
containerof_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->input(0)->origin();

	return nullptr;
}

std::unique_ptr<jive::operation>
containerof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new containerof_op(*this));
}

}

jive::output *
jive_containerof(
	jive::output * address,
	const jive::rcddeclaration * dcl,
	size_t index)
{
	jive::containerof_op op(dcl, index);
	return jive::create_normalized(address->region(), op, {address})[0];
}

/* arraysubscript */

namespace jive {

arraysubscript_op::~arraysubscript_op()
{
}

arraysubscript_op::arraysubscript_op(const arraysubscript_op & other)
: index_(other.index_)
, element_type_(other.element_type_->copy())
{}

arraysubscript_op::arraysubscript_op(arraysubscript_op && other) noexcept
: index_(std::move(other.index_))
, element_type_(std::move(other.element_type_))
{}

arraysubscript_op::arraysubscript_op(
	const jive::valuetype & type,
	const jive::bittype & index_type)
: index_(std::move(index_type.copy()))
, element_type_(type.copy())
{}

bool
arraysubscript_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const arraysubscript_op*>(&other);
	return op && op->element_type() == element_type() && op->index_ == index_;
}

size_t
arraysubscript_op::narguments() const noexcept
{
	return 2;
}

const jive::port &
arraysubscript_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	if (index == 0) {
		static const jive::port p(addrtype::instance());
		return p;
	}

	return index_;
}

size_t
arraysubscript_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
arraysubscript_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const jive::port p(addrtype::instance());
	return p;
}

std::string
arraysubscript_op::debug_string() const
{
	return "ARRAYSUBSCRIPT";
}

std::unique_ptr<jive::operation>
arraysubscript_op::copy() const
{
	return std::unique_ptr<jive::operation>(new arraysubscript_op(*this));
}

}


jive::output *
jive_arraysubscript(
	jive::output * address,
	const jive::valuetype * element_type,
	jive::output * index)
{
	jive::arraysubscript_op op(*element_type, dynamic_cast<const jive::bittype &>(index->type()));
	return jive::create_normalized(address->region(), op, {address, index})[0];
}

/* arrayindex */

namespace jive {

arrayindex_op::~arrayindex_op() noexcept
{
}

arrayindex_op::arrayindex_op(const arrayindex_op & other)
: simple_op(other)
, index_(other.index_)
, element_type_(other.element_type().copy())
{}

arrayindex_op::arrayindex_op(arrayindex_op && other) noexcept
: simple_op(other)
, index_(std::move(other.index_))
, element_type_(std::move(other.element_type_))
{}

arrayindex_op::arrayindex_op(
	const jive::valuetype & element_type,
	const jive::bittype & index_type)
: simple_op()
, index_(std::move(index_type.copy()))
, element_type_(element_type.copy())
{}

bool
arrayindex_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const arrayindex_op*>(&other);
	return op && op->element_type() == element_type() && op->index_ == index_;
}

size_t
arrayindex_op::narguments() const noexcept
{
	return 2;
}

const jive::port &
arrayindex_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	static const jive::port p(addrtype::instance());
	return p;
}

size_t
arrayindex_op::nresults() const noexcept
{
	return 1;
}

const jive::port &
arrayindex_op::result(size_t index) const noexcept
{
	return index_;
}

std::string
arrayindex_op::debug_string() const
{
	return "ARRAYINDEX";
}

std::unique_ptr<jive::operation>
arrayindex_op::copy() const
{
	return std::unique_ptr<jive::operation>(new arrayindex_op(*this));
}

}

jive::output *
jive_arrayindex(
	jive::output * addr1, jive::output * addr2,
	const jive::valuetype * element_type,
	const jive::bittype * difference_type)
{
	jive::arrayindex_op op(*element_type, difference_type->nbits());
	return jive::create_normalized(addr1->region(), op, {addr1, addr2})[0];
}

/* lbl2addr operation */

namespace jive {

lbl2addr_op::~lbl2addr_op() noexcept
{}

bool
lbl2addr_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const lbl2addr_op*>(&other);
	return op && op->label() == label();
}

const jive::port &
lbl2addr_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	static const jive::port p(addrtype::instance());
	return p;
}

std::string
lbl2addr_op::debug_string() const
{
	return detail::strfmt("LBL2ADDR: ", label());
}

std::unique_ptr<jive::operation>
lbl2addr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lbl2addr_op(*this));
}

/* lbl2bit operation */

lbl2bit_op::~lbl2bit_op() noexcept
{}

bool
lbl2bit_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const lbl2bit_op*>(&other);
	return op && op->label() == label() && op->result_ == result_;
}

const jive::port &
lbl2bit_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

std::string
lbl2bit_op::debug_string() const
{
	return detail::strfmt("LBL2BIT: ", label());
}

std::unique_ptr<jive::operation>
lbl2bit_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lbl2bit_op(*this));
}

/* constant */

jive::output *
constant(jive::graph * graph, const value_repr & vr)
{
	addrconstant_op op(vr);
	return jive::create_normalized(graph->root(), op, {})[0];
}

}
