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

}
}

const jive_node_class JIVE_MEMBEROF_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "MEMBEROF",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
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

}
}

const jive_node_class JIVE_CONTAINEROF_NODE = {
	parent : &JIVE_UNARY_OPERATION,
	name : "CONTAINEROF",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
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

arraysubscript_operation::arraysubscript_operation(
	const arraysubscript_operation & other)
	: element_type_(other.element_type_->copy())
{
}

arraysubscript_operation::arraysubscript_operation(
	arraysubscript_operation && other) noexcept
	: element_type_(std::move(other.element_type_))
{
}

arraysubscript_operation::arraysubscript_operation(
	const jive::value::type& type)
	: element_type_(type.copy())
{
}

}
}

static void
jive_arraysubscript_node_fini_(jive_node * self_);

static jive_node *
jive_arraysubscript_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[]);

static bool
jive_arraysubscript_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static void
jive_arraysubscript_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context);

const jive_node_class JIVE_ARRAYSUBSCRIPT_NODE = {
	parent : &JIVE_NODE,
	name : "ARRAYSUBSCRIPT",
	fini : jive_arraysubscript_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_arraysubscript_node_match_attrs_, /* override */
	check_operands : jive_arraysubscript_node_check_operands_, /* override */
	create : jive_arraysubscript_node_create_, /* override */
};

static void
jive_arraysubscript_node_fini_(jive_node * self_)
{
	jive_arraysubscript_node * self = (jive_arraysubscript_node *) self_;
	jive_node_fini_(self);
}

static bool
jive_arraysubscript_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_arraysubscript_node * self = (const jive_arraysubscript_node *) self_;
	const jive::address::arraysubscript_operation * attrs =
		(const jive::address::arraysubscript_operation *) attrs_;
	
	return self->operation().element_type() == attrs->element_type();
}

static void
jive_arraysubscript_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive::addr::type addrtype;
	if (!dynamic_cast<const jive::addr::output*>(operands[0]))
		jive_raise_type_error(&addrtype, &operands[0]->type(), context);
}

static jive_node *
jive_arraysubscript_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	jive::output * address = operands[0];
	jive::output * index = operands[1];

	const jive::address::arraysubscript_operation * attrs =
		(const jive::address::arraysubscript_operation *) attrs_;

	jive_node * node = new jive_arraysubscript_node(*attrs);
	node->class_ = &JIVE_ARRAYSUBSCRIPT_NODE;

	jive::addr::type address_type;
	const jive::base::type * typeptr = &address_type;

	const jive::base::type * index_type = &index->type();
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bits::type*>(index_type));

	const jive::base::type * operand_types[2] = {&address_type, index_type};
	
	jive_node_init_(node, region,
		2, operand_types, operands,
		1, &typeptr);
	
	return node;
}

jive::output *
jive_arraysubscript(jive::output * address, const jive::value::type * element_type,
	jive::output * index)
{
	jive::output * tmparray0[] = {address, index};
	jive_region * region = jive_region_innermost(2, tmparray0);

	jive::address::arraysubscript_operation attrs(*element_type);

	jive::output * operands[2] = {address, index};
	jive::output * result;
	jive_node_create_normalized(&JIVE_ARRAYSUBSCRIPT_NODE, region->graph,
		&attrs, 2, operands, &result);
	return result;
}

/* arrayindex */

namespace jive {
namespace address {

arrayindex_operation::arrayindex_operation(
	const arrayindex_operation & other)
	: element_type_(other.element_type().copy()),
	difference_type_(other.difference_type())
{
}

arrayindex_operation::arrayindex_operation(
	arrayindex_operation && other) noexcept
	: element_type_(std::move(other.element_type_)),
	difference_type_(std::move(other.difference_type_))
{
}

arrayindex_operation::arrayindex_operation(
	const jive::value::type & element_type,
	size_t nbits)
	: element_type_(element_type.copy()),
	difference_type_(nbits)
{
}

}
}

static void
jive_arrayindex_node_fini_(jive_node * self_);

static jive_node *
jive_arrayindex_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[]);

static bool
jive_arrayindex_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static void
jive_arrayindex_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive::output * const operands[], jive_context * context);

const jive_node_class JIVE_ARRAYINDEX_NODE = {
	parent : &JIVE_NODE,
	name : "ARRAYINDEX",
	fini : jive_arrayindex_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_arrayindex_node_match_attrs_, /* override */
	check_operands : jive_arrayindex_node_check_operands_, /* inherit */
	create : jive_arrayindex_node_create_, /* override */
};

static void
jive_arrayindex_node_fini_(jive_node * self_)
{
	jive_arrayindex_node * self = (jive_arrayindex_node *) self_;
	jive_node_fini_(self);
}

static bool
jive_arrayindex_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_arrayindex_node * self = (const jive_arrayindex_node *) self_;
	const jive::address::arrayindex_operation * attrs =
		(const jive::address::arrayindex_operation *) attrs_;
	
	return self->operation().element_type() == attrs->element_type();
}

static void
jive_arrayindex_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	size_t n;
	jive::addr::type addrtype;
	for (n = 0; n < noperands; n++) {
		if(!dynamic_cast<const jive::addr::output*>(operands[n]))
			jive_raise_type_error(&addrtype, &operands[n]->type(), context);
	}
}

static jive_node *
jive_arrayindex_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	const jive::address::arrayindex_operation * attrs =
		(const jive::address::arrayindex_operation *) attrs_;
	jive_node * node = new jive_arrayindex_node(*attrs);
	node->class_ = &JIVE_ARRAYINDEX_NODE;

	jive::addr::type address_type;

	const jive::base::type * operand_types[2] = {&address_type, &address_type};
	const jive::base::type * output_types[1] = {&attrs->difference_type()};
	
	jive_node_init_(node, region,
		2, operand_types, operands,
		1, output_types);
	
	return node;
}


jive::output *
jive_arrayindex(jive::output * addr1, jive::output * addr2,
	const jive::value::type * element_type, const jive::bits::type * difference_type)
{
	jive::output * tmparray2[] = {addr1, addr2};
	jive_region * region = jive_region_innermost(2, tmparray2);
	
	jive::address::arrayindex_operation attrs(*element_type, difference_type->nbits());
	
	jive::output * operands[2] = {addr1, addr2};
	jive::output * result;
	jive_node_create_normalized(&JIVE_ARRAYINDEX_NODE, region->graph,
		&attrs, 2, operands, &result);
	return result;
}

/* label_to_address node */

static void
jive_label_to_address_node_fini_(jive_node * self);

static void
jive_label_to_address_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static bool
jive_label_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_label_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive::output * const operands[]);

const jive_node_class JIVE_LABEL_TO_ADDRESS_NODE = {
	parent : &JIVE_NODE,
	name : "LABEL_TO_ADDRESS_NODE",
	fini : jive_label_to_address_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_label_to_address_node_get_label_, /* override */
	match_attrs : jive_label_to_address_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_label_to_address_node_create_, /* override */
};

static void
jive_label_to_address_node_init_(
	jive_label_to_address_node * self,
	jive_graph * graph)
{
	jive::addr::type addrtype;
	const jive::base::type * addrptr = &addrtype;
	jive_node_init_(self, graph->root_region,
		0, NULL, NULL,
		1, &addrptr);
}

static void
jive_label_to_address_node_fini_(jive_node * self_)
{
	jive_label_to_address_node * self = (jive_label_to_address_node *) self_;
	
	jive_node_fini_(self);
}

static void
jive_label_to_address_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_label_to_address_node * self = (const jive_label_to_address_node *) self_;
	
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "addrof:label%p", self->operation().label());
	jive_buffer_putstr(buffer, tmp);
}

static jive_node *
jive_label_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[])
{
	const jive::address::label_to_address_operation * attrs =
		(const jive::address::label_to_address_operation *)attrs_;
	
	jive_label_to_address_node * node = new jive_label_to_address_node(*attrs);
	node->class_ = &JIVE_LABEL_TO_ADDRESS_NODE;
	jive_label_to_address_node_init_(node, region->graph);

	return node;
}

static bool
jive_label_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::address::label_to_address_operation * first =
		&((const jive_label_to_address_node *)self)->operation();
	const jive::address::label_to_address_operation * second =
		(const jive::address::label_to_address_operation *) attrs;
	
	return first->label() == second->label();
}

jive_node *
jive_label_to_address_node_create(struct jive_graph * graph, const jive_label * label)
{
	jive::address::label_to_address_operation op(label);
	jive_label_to_address_node * node = new jive_label_to_address_node(op);
	node->class_ = &JIVE_LABEL_TO_ADDRESS_NODE;
	jive_label_to_address_node_init_(node, graph);

	return node;
}

jive::output *
jive_label_to_address_create(struct jive_graph * graph, const jive_label * label)
{
	jive::address::label_to_address_operation op(label);
	return jive_nullary_operation_create_normalized(&JIVE_LABEL_TO_ADDRESS_NODE, graph, &op);
}

/* label_to_bitstring_node */

static void
jive_label_to_bitstring_node_fini_(jive_node * self);

static void
jive_label_to_bitstring_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static bool
jive_label_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_label_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive::output * const operands[]);

const jive_node_class JIVE_LABEL_TO_BITSTRING_NODE = {
	parent : &JIVE_NODE,
	name : "LABEL_TO_BITSTRING_NODE",
	fini : jive_label_to_bitstring_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_label_to_bitstring_node_get_label_, /* override */
	match_attrs : jive_label_to_bitstring_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_label_to_bitstring_node_create_, /* override */
};

static void
jive_label_to_bitstring_node_init_(
	jive_label_to_bitstring_node * self,
	jive_graph * graph, size_t nbits)
{
	jive::bits::type btype(nbits);
	const jive::base::type * typeptr = &btype;
	jive_node_init_(self, graph->root_region,
		0, NULL, NULL,
		1, &typeptr);
}

static void
jive_label_to_bitstring_node_fini_(jive_node * self_)
{
	jive_label_to_bitstring_node * self = (jive_label_to_bitstring_node *) self_;

	jive_node_fini_(self);
}

static void
jive_label_to_bitstring_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_label_to_bitstring_node * self = (const jive_label_to_bitstring_node *) self_;
	
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "addrof:label%p", self->operation().label());
	jive_buffer_putstr(buffer, tmp);
}

static jive_node *
jive_label_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[])
{
	const jive::address::label_to_bitstring_operation * attrs =
		(const jive::address::label_to_bitstring_operation *) attrs_;

	jive_label_to_bitstring_node * node = new jive_label_to_bitstring_node(*attrs);
	node->class_ = &JIVE_LABEL_TO_BITSTRING_NODE;
	jive_label_to_bitstring_node_init_(node, region->graph, attrs->nbits());

	return node;
}

static bool
jive_label_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::address::label_to_bitstring_operation * first =
		&((const jive_label_to_bitstring_node *)self)->operation();
	const jive::address::label_to_bitstring_operation * second =
		(const jive::address::label_to_bitstring_operation *) attrs;

	return (first->label() == second->label()) && (first->nbits() == second->nbits());
}

jive::output *
jive_label_to_bitstring_create(struct jive_graph * graph, const jive_label * label, size_t nbits)
{
	jive::address::label_to_bitstring_operation op(label, nbits);
	return jive_nullary_operation_create_normalized(&JIVE_LABEL_TO_BITSTRING_NODE, graph, &op);
}

namespace jive {
namespace address {

arraysubscript_operation::~arraysubscript_operation() noexcept
{
}

arrayindex_operation::~arrayindex_operation() noexcept
{
}

}
}
