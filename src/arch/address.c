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

static void
jive_memberof_node_get_label_(const jive_node * self_, struct jive_buffer * buffer);

static bool
jive_memberof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_memberof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[]);

static void
jive_memberof_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_unop_reduction_path_t
jive_memberof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand);

static jive_output *
jive_memberof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_);

const jive_unary_operation_class JIVE_MEMBEROF_NODE_ = {
	base : {
		parent : &JIVE_UNARY_OPERATION,
		name : "MEMBEROF",
		fini : jive_node_fini_, /* inherit */
		get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_memberof_node_get_label_, /* override */
		match_attrs : jive_memberof_node_match_attrs_, /* override */
		check_operands : jive_memberof_node_check_operands_, /* override */
		create : jive_memberof_node_create_, /* override */
	},
	
	single_apply_over : NULL,
	multi_apply_over : NULL,
	
	can_reduce_operand : jive_memberof_can_reduce_operand_, /* override */
	reduce_operand : jive_memberof_reduce_operand_ /* override */
};

static void
jive_memberof_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_memberof_node * self = (const jive_memberof_node *) self_;
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "MEMBEROF %p:%zd",
		self->operation().record_decl(), self->operation().index());
	jive_buffer_putstr(buffer, tmp);
}

static bool
jive_memberof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_memberof_node * self = (const jive_memberof_node *) self_;
	const jive::address::memberof_operation * attrs =
		(const jive::address::memberof_operation *) attrs_;
	return (self->operation().record_decl() == attrs->record_decl()) &&
		(self->operation().index() == attrs->index());
}

static void
jive_memberof_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	jive::addr::type addrtype;
	if(!dynamic_cast<const jive::addr::output*>(operands[0]))
		jive_raise_type_error(&addrtype, &operands[0]->type(), context);
}

static jive_node *
jive_memberof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive::address::memberof_operation * op = (const jive::address::memberof_operation *) attrs_;

	jive_memberof_node * node = new jive_memberof_node(*op);
	node->class_ = &JIVE_MEMBEROF_NODE;

	jive::addr::type address_type;
	const jive_type * typeptr = &address_type;

	jive_node_init_(node, region,
		1, &typeptr, operands,
		1, &typeptr);

	return node;
}

static jive_unop_reduction_path_t
jive_memberof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand)
{
	const jive::address::memberof_operation * attrs =
		(const jive::address::memberof_operation *) attrs_;
	
	const jive_containerof_node * node = jive_containerof_node_cast(operand->node());
	if (!node)
		return jive_unop_reduction_none;
	
	if (node->operation().record_decl() == attrs->record_decl() &&
		node->operation().index() == attrs->index())
		return jive_unop_reduction_inverse;
	
	return jive_unop_reduction_none;
}

static jive_output *
jive_memberof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node()->inputs[0]->origin();
	
	return NULL;
}

jive_output *
jive_memberof(jive_output * address,
	const jive::rcd::declaration * record_decl, size_t index)
{
	jive::address::memberof_operation op(record_decl, index);

	return jive_unary_operation_create_normalized(&JIVE_MEMBEROF_NODE_,
		address->node()->graph, &op, address);
}

/* containerof */

static void
jive_containerof_node_get_label_(const jive_node * self_, struct jive_buffer * buffer);

static bool
jive_containerof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static jive_node *
jive_containerof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[]);

static void
jive_containerof_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_unop_reduction_path_t
jive_containerof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand);

static jive_output *
jive_containerof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand_);

const jive_unary_operation_class JIVE_CONTAINEROF_NODE_ = {
	base : {
		parent : &JIVE_UNARY_OPERATION,
		name : "CONTAINEROF",
		fini : jive_node_fini_, /* inherit */
		get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_containerof_node_get_label_, /* override */
		match_attrs : jive_containerof_node_match_attrs_, /* override */
		check_operands : jive_containerof_node_check_operands_, /* override */
		create : jive_containerof_node_create_, /* override */
	},
	
	single_apply_over : NULL,
	multi_apply_over : NULL,
	
	can_reduce_operand : jive_containerof_can_reduce_operand_, /* override */
	reduce_operand : jive_containerof_reduce_operand_ /* override */
};

static void
jive_containerof_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_containerof_node * self = (const jive_containerof_node *) self_;
	char tmp[128];
	snprintf(tmp, sizeof(tmp), "CONTAINEROF %p:%zd",
		self->operation().record_decl(), self->operation().index());
	jive_buffer_putstr(buffer, tmp);
}

static bool
jive_containerof_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_)
{
	const jive_containerof_node * self = (const jive_containerof_node *) self_;
	const jive::address::containerof_operation * attrs =
		(const jive::address::containerof_operation *) attrs_;
	return (self->operation().record_decl() == attrs->record_decl()) &&
		(self->operation().index() == attrs->index());
}

static void
jive_containerof_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	jive::addr::type addrtype;
	if(!dynamic_cast<const jive::addr::output*>(operands[0]))
		jive_raise_type_error(&addrtype, &operands[0]->type(), context);
}

static jive_node *
jive_containerof_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);
	const jive::address::containerof_operation * attrs =
		(const jive::address::containerof_operation *) attrs_;
	jive_node * node = new jive_containerof_node(*attrs);
	
	jive::addr::type address_type;
	const jive_type * typeptr = &address_type;

	node->class_ = &JIVE_CONTAINEROF_NODE;
	jive_node_init_(node, region,
		1, &typeptr, &operands[0],
		1, &typeptr);
	
	return node;
}

static jive_unop_reduction_path_t
jive_containerof_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand)
{
	const jive::address::containerof_operation * attrs =
		(const jive::address::containerof_operation *) attrs_;
	
	const jive_memberof_node * node = jive_memberof_node_cast(operand->node());
	if (!node)
		return jive_unop_reduction_none;
	
	if (node->operation().record_decl() == attrs->record_decl() &&
		node->operation().index() == attrs->index())
		return jive_unop_reduction_inverse;
	
	return jive_unop_reduction_none;
}

static jive_output *
jive_containerof_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node()->inputs[0]->origin();
	
	return NULL;
}

jive_node *
jive_containerof_node_create(jive_region * region,
	jive_output * address,
	const jive::rcd::declaration * record_decl, size_t index)
{
	jive::address::containerof_operation op(record_decl, index);
	jive_containerof_node * node = new jive_containerof_node(op);
	
	node->class_ = &JIVE_CONTAINEROF_NODE;
	
	jive::addr::type address_type;
	const jive_type * addrptr = &address_type;
	
	jive_node_init_(node, region,
		1, &addrptr, &address,
		1, &addrptr);
	
	return node;
}

jive_output *
jive_containerof(jive_output * address,
	const jive::rcd::declaration * record_decl, size_t index)
{
	jive::address::containerof_operation op(record_decl, index);

	return jive_unary_operation_create_normalized(&JIVE_CONTAINEROF_NODE_, address->node()->graph,
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
	size_t noperands, jive_output * const operands[]);

static bool
jive_arraysubscript_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static void
jive_arraysubscript_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_binop_reduction_path_t
jive_arraysubscript_can_reduce_operand_pair_(
	const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand1, const jive_output * operand2);

static jive_output *
jive_arraysubscript_reduce_operand_pair_(
	jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2);

const jive_binary_operation_class JIVE_ARRAYSUBSCRIPT_NODE_ = {
	base : {
		parent : &JIVE_BINARY_OPERATION,
		name : "ARRAYSUBSCRIPT",
		fini : jive_arraysubscript_node_fini_, /* override */
		get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_node_get_label_, /* inherit */
		match_attrs : jive_arraysubscript_node_match_attrs_, /* override */
		check_operands : jive_arraysubscript_node_check_operands_, /* override */
		create : jive_arraysubscript_node_create_, /* override */
	},
	
	flags : jive_binary_operation_none,
	single_apply_under : NULL,
	multi_apply_under : NULL,
	distributive_over : NULL,
	distributive_under : NULL,
	
	can_reduce_operand_pair : jive_arraysubscript_can_reduce_operand_pair_, /* override */
	reduce_operand_pair : jive_arraysubscript_reduce_operand_pair_ /* override */
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
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 2);

	jive::addr::type addrtype;
	if (!dynamic_cast<const jive::addr::output*>(operands[0]))
		jive_raise_type_error(&addrtype, &operands[0]->type(), context);
}

static jive_node *
jive_arraysubscript_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	jive_output * address = operands[0];
	jive_output * index = operands[1];

	const jive::address::arraysubscript_operation * attrs =
		(const jive::address::arraysubscript_operation *) attrs_;

	jive_node * node = new jive_arraysubscript_node(*attrs);
	node->class_ = &JIVE_ARRAYSUBSCRIPT_NODE;

	jive::addr::type address_type;
	const jive_type * typeptr = &address_type;

	const jive_type * index_type = &index->type();
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::bits::type*>(index_type));

	const jive_type * operand_types[2] = {&address_type, index_type};
	
	jive_node_init_(node, region,
		2, operand_types, operands,
		1, &typeptr);
	
	return node;
}

static jive_binop_reduction_path_t
jive_arraysubscript_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2)
{
	const jive::address::arraysubscript_operation * attrs =
		(const jive::address::arraysubscript_operation *) attrs_;
	
	jive_bitconstant_node * offset_constant = dynamic_cast<jive_bitconstant_node *>(operand2->node());
	if (offset_constant && jive_bitconstant_is_zero(offset_constant))
		return jive_binop_reduction_rneutral;
	
	const jive_arraysubscript_node * node = jive_arraysubscript_node_cast(operand1->node());
	if (node && attrs->element_type() == node->operation().element_type() &&
		operand2->type() == node->inputs[1]->type())
		return jive_binop_reduction_lfold;
	
	return jive_binop_reduction_none;
}

static jive_output *
jive_arraysubscript_reduce_operand_pair_(
	jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2)
{
	if (path == jive_binop_reduction_rneutral)
		return operand1;

	const jive_arraysubscript_node * node = jive_arraysubscript_node_cast(operand1->node());
	if (path == jive_binop_reduction_lfold) {
		jive_output * operands[2] = {node->inputs[1]->origin(), operand2};
		jive_output * sum = jive_bitsum(2, operands);
		
		operands[0] = node->inputs[0]->origin();
		operands[1] = sum;
		
		return jive_binary_operation_create_normalized(&JIVE_ARRAYSUBSCRIPT_NODE_,
			operand1->node()->graph, attrs_, 2, operands);
	}
	
	return NULL;
}

jive_output *
jive_arraysubscript(jive_output * address, const jive::value::type * element_type,
	jive_output * index)
{
	jive_output * tmparray0[] = {address, index};
	jive_region * region = jive_region_innermost(2, tmparray0);

	jive::address::arraysubscript_operation attrs(*element_type);

	jive_output * operands[2] = {address, index};
	return jive_binary_operation_create_normalized(&JIVE_ARRAYSUBSCRIPT_NODE_, region->graph,
		&attrs, 2, operands);
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
	size_t noperands, jive_output * const operands[]);

static bool
jive_arrayindex_node_match_attrs_(const jive_node * self_, const jive_node_attrs * attrs_);

static void
jive_arrayindex_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_binop_reduction_path_t
jive_arrayindex_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2);

static jive_output *
jive_arrayindex_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2);

const jive_binary_operation_class JIVE_ARRAYINDEX_NODE_ = {
	base : {
		parent : &JIVE_BINARY_OPERATION,
		name : "ARRAYINDEX",
		fini : jive_arrayindex_node_fini_, /* override */
		get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_node_get_label_, /* inherit */
		match_attrs : jive_arrayindex_node_match_attrs_, /* override */
		check_operands : jive_arrayindex_node_check_operands_, /* inherit */
		create : jive_arrayindex_node_create_, /* override */
	},
	
	flags : jive_binary_operation_none,
	single_apply_under : NULL,
	multi_apply_under : NULL,
	distributive_over : NULL,
	distributive_under : NULL,
	
	can_reduce_operand_pair : jive_arrayindex_can_reduce_operand_pair_, /* override */
	reduce_operand_pair : jive_arrayindex_reduce_operand_pair_ /* override */
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
	size_t noperands, jive_output * const operands[], jive_context * context)
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
	size_t noperands, jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 2);
	const jive::address::arrayindex_operation * attrs =
		(const jive::address::arrayindex_operation *) attrs_;
	jive_node * node = new jive_arrayindex_node(*attrs);
	node->class_ = &JIVE_ARRAYINDEX_NODE;

	jive::addr::type address_type;

	const jive_type * operand_types[2] = {&address_type, &address_type};
	const jive_type * output_types[1] = {&attrs->difference_type()};
	
	jive_node_init_(node, region,
		2, operand_types, operands,
		1, output_types);
	
	return node;
}

static const jive_output *
get_array_base(const jive_output * addr, const jive::value::type * element_type)
{
	jive_arraysubscript_node * node = jive_arraysubscript_node_cast(addr->node());
	if (node && *element_type == node->operation().element_type())
		return addr->node()->inputs[0]->origin();
	else return addr;
}

static jive_output *
get_array_index(jive_output * addr, const jive_output * base, const jive::value::type * element_type,
	const jive::bits::type * index_type)
{
	jive_output * index = NULL;
	jive_arraysubscript_node * node = jive_arraysubscript_node_cast(addr->node());
	if (node && *element_type == node->operation().element_type()) {
		/* FIXME: correct type! */
		index = addr->node()->inputs[1]->origin();
	} else {
		char bits[index_type->nbits()];
		memset(bits, '0', index_type->nbits());
		index = jive_bitconstant(addr->node()->graph, index_type->nbits(), bits);
	}
	
	return index;
}

static jive_binop_reduction_path_t
jive_arrayindex_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand1, const jive_output * operand2)
{
	const jive::address::arrayindex_operation * attrs =
		(const jive::address::arrayindex_operation *) attrs_;
	const jive_output * base1 = get_array_base(operand1, &attrs->element_type());
	const jive_output * base2 = get_array_base(operand2, &attrs->element_type());

	if (base1 == base2)
		return jive_binop_reduction_factor;
	
	return jive_binop_reduction_none;
}

static jive_output *
jive_arrayindex_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand1, jive_output * operand2)
{
	const jive::address::arrayindex_operation * attrs =
		(const jive::address::arrayindex_operation *) attrs_;

	if (path == jive_binop_reduction_factor) {
		const jive_output * base1 = get_array_base(operand1, &attrs->element_type());
		const jive_output * base2 = get_array_base(operand2, &attrs->element_type());
	
		jive_output * index1 = get_array_index(operand1, base1, &attrs->element_type(),
			&attrs->difference_type());
		jive_output * index2 = get_array_index(operand2, base2, &attrs->element_type(),
			&attrs->difference_type());
	
		index2 = jive_bitnegate(index2);
		jive_output * tmparray1[] = {index1, index2};
		return jive_bitsum(2, tmparray1);
	}

	return NULL;
}

jive_output *
jive_arrayindex(jive_output * addr1, jive_output * addr2,
	const jive::value::type * element_type, const jive::bits::type * difference_type)
{
	jive_output * tmparray2[] = {addr1, addr2};
	jive_region * region = jive_region_innermost(2, tmparray2);
	
	jive::address::arrayindex_operation attrs(*element_type, difference_type->nbits());
	
	jive_output * operands[2] = {addr1, addr2};
	return jive_binary_operation_create_normalized(&JIVE_ARRAYINDEX_NODE_, region->graph,
		&attrs, 2, operands);
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
	size_t noperands, struct jive_output * const operands[]);

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
	const jive_type * addrptr = &addrtype;
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
	size_t noperands, struct jive_output * const operands[])
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

jive_output *
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
	size_t noperands, struct jive_output * const operands[]);

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
	const jive_type * typeptr = &btype;
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
	size_t noperands, struct jive_output * const operands[])
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

jive_output *
jive_label_to_bitstring_create(struct jive_graph * graph, const jive_label * label, size_t nbits)
{
	jive::address::label_to_bitstring_operation op(label, nbits);
	return jive_nullary_operation_create_normalized(&JIVE_LABEL_TO_BITSTRING_NODE, graph, &op);
}
