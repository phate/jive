/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address.h>

#include <stdio.h>
#include <string.h>

#include <jive/arch/addresstype.h>
#include <jive/context.h>
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

memberof_operation::~memberof_operation() noexcept
{
}

bool
memberof_operation::operator==(const operation & other) const noexcept
{
	const memberof_operation * op =
		dynamic_cast<const memberof_operation *>(&other);

	return op && op->record_decl() == record_decl() && op->index() == index();
}

jive_node *
memberof_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 1);

	jive_memberof_node * node = new jive_memberof_node(*this);
	node->class_ = &JIVE_MEMBEROF_NODE;

	const jive::base::type * typeptr = &jive::addr::type::singleton();

	jive_node_init_(node, region,
		1, &typeptr, arguments,
		1, &typeptr);

	return node;
}

std::string
memberof_operation::debug_string() const
{
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "MEMBEROF %p:%zd", record_decl(), index());
	return tmp;
}

const jive::base::type &
memberof_operation::argument_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

const jive::base::type &
memberof_operation::result_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

jive_unop_reduction_path_t
memberof_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	const containerof_operation * op =
		dynamic_cast<const containerof_operation *>(&arg->node()->operation());

	if (!op) {
		return jive_unop_reduction_none;
	}

	if (op->record_decl() == record_decl() && op->index() == index()) {
		return jive_unop_reduction_inverse;
	}

	return jive_unop_reduction_none;
}

jive::output *
memberof_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse)
		return arg->node()->inputs[0]->origin();
	
	return nullptr;
}

std::unique_ptr<jive::operation>
memberof_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new memberof_operation(*this));
}

}
}

const jive_node_class JIVE_MEMBEROF_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "MEMBEROF",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};


jive::output *
jive_memberof(jive::output * address,
	const jive::rcd::declaration * record_decl, size_t index)
{
	jive::address::memberof_operation op(record_decl, index);

	return jive_unary_operation_create_normalized(&JIVE_MEMBEROF_NODE,
		address->node()->graph, &op, address);
}

/* containerof */

namespace jive {
namespace address {

containerof_operation::~containerof_operation() noexcept
{
}

bool
containerof_operation::operator==(const operation & other) const noexcept
{
	const containerof_operation * op =
		dynamic_cast<const containerof_operation *>(&other);
	return op && op->record_decl() == record_decl() && op->index() == index();
}

jive_node *
containerof_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 1);

	jive_containerof_node * node = new jive_containerof_node(*this);
	node->class_ = &JIVE_CONTAINEROF_NODE;

	const jive::base::type * typeptr = &jive::addr::type::singleton();

	jive_node_init_(node, region,
		1, &typeptr, arguments,
		1, &typeptr);

	return node;
}

std::string
containerof_operation::debug_string() const
{
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "CONTAINEROF %p:%zd", record_decl(), index());
	return tmp;
}

const jive::base::type &
containerof_operation::argument_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

const jive::base::type &
containerof_operation::result_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

jive_unop_reduction_path_t
containerof_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	const memberof_operation * op =
		dynamic_cast<const memberof_operation *>(&arg->node()->operation());
	if (!op) {
		return jive_unop_reduction_none;
	}
	
	if (op->record_decl() == record_decl() && op->index() == index()) {
		return jive_unop_reduction_inverse;
	}
	
	return jive_unop_reduction_none;
}

jive::output *
containerof_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	if (path == jive_unop_reduction_inverse) {
		return arg->node()->inputs[0]->origin();
	} else {
		return nullptr;
	}
}

std::unique_ptr<jive::operation>
containerof_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new containerof_operation(*this));
}

}
}

const jive_node_class JIVE_CONTAINEROF_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "CONTAINEROF",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

jive::output *
jive_containerof(jive::output * address,
	const jive::rcd::declaration * record_decl, size_t index)
{
	jive::address::containerof_operation op(record_decl, index);

	return jive_unary_operation_create_normalized(&JIVE_CONTAINEROF_NODE, address->node()->graph,
		&op, address);
}

/* arraysubscript */

namespace jive {
namespace address {

arraysubscript_operation::~arraysubscript_operation()
{
}

arraysubscript_operation::arraysubscript_operation(
	const arraysubscript_operation & other)
	: element_type_(other.element_type_->copy())
	, index_type_(other.index_type())
{
}

arraysubscript_operation::arraysubscript_operation(
	arraysubscript_operation && other) noexcept
	: element_type_(std::move(other.element_type_))
	, index_type_(other.index_type())
{
}

arraysubscript_operation::arraysubscript_operation(
	const jive::value::type& type,
	const jive::bits::type& index_type)
	: element_type_(type.copy())
	, index_type_(index_type)
{
}

bool
arraysubscript_operation::operator==(const operation & other) const noexcept
{
	const arraysubscript_operation * op =
		dynamic_cast<const arraysubscript_operation *>(&other);
	return op && op->element_type() == element_type() && op->index_type() == index_type();
}

size_t
arraysubscript_operation::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
arraysubscript_operation::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return jive::addr::type::singleton();
	} else {
		return index_type_;
	}
}

size_t
arraysubscript_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
arraysubscript_operation::result_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

jive_node *
arraysubscript_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 2);

	jive_node * node = new jive_arraysubscript_node(*this);
	node->class_ = &JIVE_ARRAYSUBSCRIPT_NODE;

	const jive::base::type * argument_types[2] = {
		&argument_type(0),
		&argument_type(1)
	};
	const jive::base::type * result_types[1] = {
		&result_type(0)
	};
	
	jive_node_init_(node, region,
		2, argument_types, arguments,
		1, result_types);
	
	return node;
}

std::string
arraysubscript_operation::debug_string() const
{
	return "ARRAYSUBSCRIPT";
}

std::unique_ptr<jive::operation>
arraysubscript_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new arraysubscript_operation(*this));
}

}
}

const jive_node_class JIVE_ARRAYSUBSCRIPT_NODE = {
	parent : &JIVE_NODE,
	name : "ARRAYSUBSCRIPT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};


jive::output *
jive_arraysubscript(
	jive::output * address,
	const jive::value::type * element_type,
	jive::output * index)
{
	jive::address::arraysubscript_operation op(
		*element_type,
		dynamic_cast<const jive::bits::type &>(index->type()));


	jive::output * arguments[2] = {address, index};
	jive_region * region = jive_region_innermost(2, arguments);

	jive::output * result;

	jive_node_create_normalized(&JIVE_ARRAYSUBSCRIPT_NODE, region->graph,
		&op, 2, arguments, &result);
	return result;
}

/* arrayindex */

namespace jive {
namespace address {

arrayindex_operation::~arrayindex_operation() noexcept
{
}

arrayindex_operation::arrayindex_operation(
	const arrayindex_operation & other)
	: element_type_(other.element_type().copy()),
	index_type_(other.index_type())
{
}

arrayindex_operation::arrayindex_operation(
	arrayindex_operation && other) noexcept
	: element_type_(std::move(other.element_type_)),
	index_type_(other.index_type())
{
}

arrayindex_operation::arrayindex_operation(
	const jive::value::type & element_type,
	const jive::bits::type & index_type)
	: element_type_(element_type.copy())
	, index_type_(index_type)
{
}

bool
arrayindex_operation::operator==(const operation & other) const noexcept
{
	const arrayindex_operation * op =
		dynamic_cast<const arrayindex_operation *>(&other);
	return op && op->element_type() == element_type() && op->index_type() == index_type();
}

size_t
arrayindex_operation::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
arrayindex_operation::argument_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

size_t
arrayindex_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
arrayindex_operation::result_type(size_t index) const noexcept
{
	return index_type_;
}

jive_node *
arrayindex_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 2);

	jive_node * node = new jive_arrayindex_node(*this);
	node->class_ = &JIVE_ARRAYINDEX_NODE;

	const jive::base::type * argument_types[2] = {
		&argument_type(0),
		&argument_type(1)
	};
	const jive::base::type * result_types[1] = {
		&result_type(0)
	};
	
	jive_node_init_(node, region,
		2, argument_types, arguments,
		1, result_types);
	
	return node;
}

std::string
arrayindex_operation::debug_string() const
{
	return "ARRAYINDEX";
}

std::unique_ptr<jive::operation>
arrayindex_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new arrayindex_operation(*this));
}

}
}

const jive_node_class JIVE_ARRAYINDEX_NODE = {
	parent : &JIVE_NODE,
	name : "ARRAYINDEX",
	fini : jive_node_fini_, /* override */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_arrayindex(
	jive::output * addr1, jive::output * addr2,
	const jive::value::type * element_type,
	const jive::bits::type * difference_type)
{
	jive::output * arguments[] = {addr1, addr2};
	jive_region * region = jive_region_innermost(2, arguments);
	
	jive::address::arrayindex_operation attrs(*element_type, difference_type->nbits());
	
	jive::output * result;
	jive_node_create_normalized(&JIVE_ARRAYINDEX_NODE, region->graph,
		&attrs, 2, arguments, &result);
	return result;
}

/* label_to_address node */

namespace jive {
namespace address {

label_to_address_operation::~label_to_address_operation() noexcept
{
}

bool
label_to_address_operation::operator==(const operation & other) const noexcept
{
	const label_to_address_operation * op =
		dynamic_cast<const label_to_address_operation *>(&other);
	return op && op->label() == label();
}

size_t
label_to_address_operation::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
label_to_address_operation::argument_type(size_t index) const noexcept
{
	throw std::logic_error("no arguments");
}

size_t
label_to_address_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
label_to_address_operation::result_type(size_t index) const noexcept
{
	return jive::addr::type::singleton();
}

jive_node *
label_to_address_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_label_to_address_node * node = new jive_label_to_address_node(*this);
	node->class_ = &JIVE_LABEL_TO_ADDRESS_NODE;

	const jive::base::type * result_types[1] = {&result_type(0)};
	jive_node_init_(node, region,
		0, nullptr, nullptr,
		1, result_types);

	return node;
}

std::string
label_to_address_operation::debug_string() const
{
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "addrof:label%p", label());
	return tmp;
}

std::unique_ptr<jive::operation>
label_to_address_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new label_to_address_operation(*this));
}

}
}

const jive_node_class JIVE_LABEL_TO_ADDRESS_NODE = {
	parent : &JIVE_NODE,
	name : "LABEL_TO_ADDRESS_NODE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};


jive::output *
jive_label_to_address_create(jive_graph * graph, const jive_label * label)
{
	jive::address::label_to_address_operation op(label);
	return jive_nullary_operation_create_normalized(&JIVE_LABEL_TO_ADDRESS_NODE, graph, &op);
}

/* label_to_bitstring_node */

namespace jive {
namespace address {

label_to_bitstring_operation::~label_to_bitstring_operation() noexcept
{
}

bool
label_to_bitstring_operation::operator==(const operation & other) const noexcept
{
	const label_to_bitstring_operation * op =
		dynamic_cast<const label_to_bitstring_operation *>(&other);
	return op && op->label() == label() && op->nbits() == nbits();
}

size_t
label_to_bitstring_operation::narguments() const noexcept
{
	return 0;
}

const jive::base::type &
label_to_bitstring_operation::argument_type(size_t index) const noexcept
{
	throw std::logic_error("no arguments");
}

size_t
label_to_bitstring_operation::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
label_to_bitstring_operation::result_type(size_t index) const noexcept
{
	return result_type_;
}

jive_node *
label_to_bitstring_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_label_to_bitstring_node * node = new jive_label_to_bitstring_node(*this);
	node->class_ = &JIVE_LABEL_TO_BITSTRING_NODE;

	const jive::base::type * result_types[1] = {&result_type(0)};
	jive_node_init_(node, region,
		0, nullptr, nullptr,
		1, result_types);

	return node;
}

std::string
label_to_bitstring_operation::debug_string() const
{
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "addrof:label%p", label());
	return tmp;
}

std::unique_ptr<jive::operation>
label_to_bitstring_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new label_to_bitstring_operation(*this));
}

}
}

const jive_node_class JIVE_LABEL_TO_BITSTRING_NODE = {
	parent : &JIVE_NODE,
	name : "LABEL_TO_BITSTRING_NODE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};


jive::output *
jive_label_to_bitstring_create(jive_graph * graph, const jive_label * label, size_t nbits)
{
	jive::address::label_to_bitstring_operation op(label, nbits);
	return jive_nullary_operation_create_normalized(&JIVE_LABEL_TO_BITSTRING_NODE, graph, &op);
}
