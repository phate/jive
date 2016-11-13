/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.h>

#include <stdio.h>
#include <string.h>

#include <jive/arch/addresstype.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

/* memberof */

namespace jive {
namespace address {

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
	return detail::strfmt("MEMBEROF", record_decl().get(), index());
}

const jive::base::type &
memberof_op::argument_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
}

const jive::base::type &
memberof_op::result_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
}

jive_unop_reduction_path_t
memberof_op::can_reduce_operand(
	const jive::oport * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::output*>(arg);
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

jive::oport *
memberof_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::oport * arg) const
{
	auto op = static_cast<jive::output*>(arg);
	if (path == jive_unop_reduction_inverse)
		return dynamic_cast<jive::output*>(op->node()->input(0)->origin());
	
	return nullptr;
}

std::unique_ptr<jive::operation>
memberof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new memberof_op(*this));
}

}
}


jive::output *
jive_memberof(
	jive::output * address,
	std::shared_ptr<const jive::rcd::declaration> & record_decl,
	size_t index)
{
	jive::address::memberof_op op(record_decl, index);
	return jive_node_create_normalized(address->node()->region(), op, {address})[0];
}

/* containerof */

namespace jive {
namespace address {

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

const jive::base::type &
containerof_op::argument_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
}

const jive::base::type &
containerof_op::result_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
}

jive_unop_reduction_path_t
containerof_op::can_reduce_operand(
	const jive::oport * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::output*>(arg);
	if (!tmp)
		return jive_unop_reduction_none;

	auto op = dynamic_cast<const memberof_op *>(&tmp->node()->operation());
	if (!op)
		return jive_unop_reduction_none;

	if (op->record_decl() == record_decl() && op->index() == index())
		return jive_unop_reduction_inverse;

	return jive_unop_reduction_none;
}

jive::oport *
containerof_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::oport * arg) const
{
	auto op = static_cast<const jive::output*>(arg);
	if (path == jive_unop_reduction_inverse)
		return dynamic_cast<jive::output*>(op->node()->input(0)->origin());

	return nullptr;
}

std::unique_ptr<jive::operation>
containerof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new containerof_op(*this));
}

}
}

jive::output *
jive_containerof(
	jive::output * address,
	std::shared_ptr<const jive::rcd::declaration> & record_decl,
	size_t index)
{
	jive::address::containerof_op op(record_decl, index);
	return jive_node_create_normalized(address->node()->region(), op, {address})[0];
}

/* arraysubscript */

namespace jive {
namespace address {

arraysubscript_op::~arraysubscript_op()
{
}

arraysubscript_op::arraysubscript_op(
	const arraysubscript_op & other)
	: element_type_(other.element_type_->copy())
	, index_type_(other.index_type())
{
}

arraysubscript_op::arraysubscript_op(
	arraysubscript_op && other) noexcept
	: element_type_(std::move(other.element_type_))
	, index_type_(other.index_type())
{
}

arraysubscript_op::arraysubscript_op(
	const jive::value::type& type,
	const jive::bits::type& index_type)
	: element_type_(type.copy())
	, index_type_(index_type)
{
}

bool
arraysubscript_op::operator==(const operation & other) const noexcept
{
	const arraysubscript_op * op =
		dynamic_cast<const arraysubscript_op *>(&other);
	return op && op->element_type() == element_type() && op->index_type() == index_type();
}

size_t
arraysubscript_op::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
arraysubscript_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return jive::addr::type::instance();
	} else {
		return index_type_;
	}
}

size_t
arraysubscript_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
arraysubscript_op::result_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
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
}


jive::output *
jive_arraysubscript(
	jive::output * address,
	const jive::value::type * element_type,
	jive::output * index)
{
	jive::address::arraysubscript_op op(
		*element_type,
		dynamic_cast<const jive::bits::type &>(index->type()));

	return jive_node_create_normalized(address->node()->region(), op, {address, index})[0];
}

/* arrayindex */

namespace jive {
namespace address {

arrayindex_op::~arrayindex_op() noexcept
{
}

arrayindex_op::arrayindex_op(
	const arrayindex_op & other)
	: element_type_(other.element_type().copy()),
	index_type_(other.index_type())
{
}

arrayindex_op::arrayindex_op(
	arrayindex_op && other) noexcept
	: element_type_(std::move(other.element_type_)),
	index_type_(other.index_type())
{
}

arrayindex_op::arrayindex_op(
	const jive::value::type & element_type,
	const jive::bits::type & index_type)
	: element_type_(element_type.copy())
	, index_type_(index_type)
{
}

bool
arrayindex_op::operator==(const operation & other) const noexcept
{
	const arrayindex_op * op =
		dynamic_cast<const arrayindex_op *>(&other);
	return op && op->element_type() == element_type() && op->index_type() == index_type();
}

size_t
arrayindex_op::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
arrayindex_op::argument_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
}

size_t
arrayindex_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
arrayindex_op::result_type(size_t index) const noexcept
{
	return index_type_;
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
}

jive::output *
jive_arrayindex(
	jive::output * addr1, jive::output * addr2,
	const jive::value::type * element_type,
	const jive::bits::type * difference_type)
{
	jive::address::arrayindex_op op(*element_type, difference_type->nbits());
	return jive_node_create_normalized(addr1->node()->region(), op, {addr1, addr2})[0];
}

/* label_to_address node */

namespace jive {
namespace address {

label_to_address_op::~label_to_address_op() noexcept
{
}

bool
label_to_address_op::operator==(const operation & other) const noexcept
{
	const label_to_address_op * op =
		dynamic_cast<const label_to_address_op *>(&other);
	return op && op->label() == label();
}

size_t
label_to_address_op::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
label_to_address_op::argument_type(size_t index) const noexcept
{
	throw std::logic_error("no arguments");
}

size_t
label_to_address_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
label_to_address_op::result_type(size_t index) const noexcept
{
	return jive::addr::type::instance();
}
std::string
label_to_address_op::debug_string() const
{
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "addrof:label%p", label());
	return tmp;
}

std::unique_ptr<jive::operation>
label_to_address_op::copy() const
{
	return std::unique_ptr<jive::operation>(new label_to_address_op(*this));
}

}
}


jive::output *
jive_label_to_address_create(jive::region * region, const jive_label * label)
{
	jive::address::label_to_address_op op(label);
	return jive_node_create_normalized(region, op, {})[0];
}

/* label_to_bitstring_node */

namespace jive {
namespace address {

label_to_bitstring_op::~label_to_bitstring_op() noexcept
{
}

bool
label_to_bitstring_op::operator==(const operation & other) const noexcept
{
	const label_to_bitstring_op * op =
		dynamic_cast<const label_to_bitstring_op *>(&other);
	return op && op->label() == label() && op->nbits() == nbits();
}

size_t
label_to_bitstring_op::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
label_to_bitstring_op::argument_type(size_t index) const noexcept
{
	throw std::logic_error("no arguments");
}

size_t
label_to_bitstring_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
label_to_bitstring_op::result_type(size_t index) const noexcept
{
	return result_type_;
}
std::string
label_to_bitstring_op::debug_string() const
{
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "addrof:label%p", label());
	return tmp;
}

std::unique_ptr<jive::operation>
label_to_bitstring_op::copy() const
{
	return std::unique_ptr<jive::operation>(new label_to_bitstring_op(*this));
}

/* constant */

output *
constant(struct jive_graph * graph, const value_repr & vr)
{
	constant_op op(vr);
	return jive_node_create_normalized(graph->root(), op, {})[0];
}

}
}


jive::output *
jive_label_to_bitstring_create(jive::region * region, const jive_label * label, size_t nbits)
{
	jive::address::label_to_bitstring_op op(label, nbits);
	return jive_node_create_normalized(region, op, {})[0];
}
