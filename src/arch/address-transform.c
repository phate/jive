/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address-transform.h>

#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/load.h>
#include <jive/arch/memlayout.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fcttype.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

static bool
type_contains_address(const jive_type * type)
{
	if (dynamic_cast<const jive::addr::type*>(type))
		return true;

	const jive_function_type * fcttype = dynamic_cast<const jive_function_type*>(type);
	if (fcttype != NULL) {
		size_t n;
		for (n = 0; n < fcttype->narguments(); n++)
			if (type_contains_address(fcttype->argument_type(n)))
				return true;
		for (n = 0; n < fcttype->nreturns(); n++)
			if (type_contains_address(fcttype->return_type(n)))
				return true;
	}

	return false;
}

static jive_type *
convert_address_to_bitstring_type(const jive_type * type, size_t nbits, jive_context * context)
{
	if (dynamic_cast<const jive::addr::type*>(type)) {
		jive::bits::type bittype(nbits);
		return bittype.copy();
	}

	const jive_function_type * fcttype = dynamic_cast<const jive_function_type*>(type);
	if (fcttype != NULL) {

		size_t n;
		size_t narguments = fcttype->narguments();
		const jive_type * argument_types[narguments];
		for (n = 0; n < narguments; n++)
			argument_types[n] = convert_address_to_bitstring_type(fcttype->argument_type(n), nbits,
				context);

		size_t nresults = fcttype->nreturns();
		const jive_type * result_types[nresults];
		for (n = 0; n < nresults; n++)
			result_types[n] = convert_address_to_bitstring_type(fcttype->return_type(n), nbits, context);

		jive_function_type return_type(narguments, argument_types, nresults, result_types);

		jive_type * new_fcttype = return_type.copy();

		for (n = 0; n < narguments; n++)
			delete argument_types[n];
		for (n = 0; n < nresults; n++)
			delete result_types[n];

		return new_fcttype;
	}

	JIVE_DEBUG_ASSERT(0);
}

/* address_to_bitstring node */

static void
jive_address_to_bitstring_node_init_(jive_address_to_bitstring_node * self, jive_region * region,
	jive_output * address, size_t nbits, jive_type * original_type);

static void
jive_address_to_bitstring_node_fini_(jive_node * self);

static bool
jive_address_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static void
jive_address_to_bitstring_node_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

static jive_node *
jive_address_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_unop_reduction_path_t
jive_address_to_bitstring_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * operand);

static jive_output *
jive_address_to_bitstring_node_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * operand);

const jive_unary_operation_class JIVE_ADDRESS_TO_BITSTRING_NODE_ = {
	base : { /* jive_node_class */
		parent : &JIVE_UNARY_OPERATION,
		name : "ADDRESS_TO_BITSTRING",
		fini : jive_address_to_bitstring_node_fini_, /* override */
		get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_node_get_label_, /* inherit */
		match_attrs : jive_address_to_bitstring_node_match_attrs_, /* override */
		check_operands : jive_address_to_bitstring_node_check_operands_, /* override */
		create : jive_address_to_bitstring_node_create_, /* override */
	},

	single_apply_over : NULL,
	multi_apply_over : NULL,
	
	can_reduce_operand : jive_address_to_bitstring_node_can_reduce_operand_, /* override */
	reduce_operand : jive_address_to_bitstring_node_reduce_operand_ /* override */
};

static void
jive_address_to_bitstring_node_init_(
	jive_address_to_bitstring_node * self,
	jive_region * region,
	jive_output * address,
	size_t nbits,	jive_type * original_type)
{
	jive_context * context = region->graph->context;

	const jive_type * addrtype = &address->type();
	if (!dynamic_cast<const jive_value_type*>(addrtype))
		jive_context_fatal_error(context, "Type mismatch: expected a value type.");

	JIVE_DEBUG_ASSERT(*addrtype == *original_type);
	jive_type * return_type = convert_address_to_bitstring_type(addrtype, nbits, context);

	jive_node_init_(self, region,
		1, &addrtype, &address,
		1, (const jive_type **)&return_type);

	delete return_type;
}

static void
jive_address_to_bitstring_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static bool
jive_address_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::address_to_bitstring_operation * first = &((const jive_address_to_bitstring_node *)
		self)->operation();
	const jive::address_to_bitstring_operation * second =
		(const jive::address_to_bitstring_operation *) attrs;
	
	if (first->nbits() != second->nbits())
		return false;

	return first->original_type() == second->original_type();
}

static void
jive_address_to_bitstring_node_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	if (!dynamic_cast<jive_value_output*>(operands[0]))
		jive_context_fatal_error(context, "Type mismatch: expected a value type.");
}

static jive_node *
jive_address_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive::address_to_bitstring_operation * attrs =
		(const jive::address_to_bitstring_operation *) attrs_;

	jive_address_to_bitstring_node * node = new jive_address_to_bitstring_node(*attrs);
	node->class_ = &JIVE_ADDRESS_TO_BITSTRING_NODE;
	jive_address_to_bitstring_node_init_(node, region, operands[0], attrs->nbits(),
		&attrs->original_type());

	return node;
}

static jive_unop_reduction_path_t
jive_address_to_bitstring_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand)
{
	const jive::address_to_bitstring_operation * attrs;
	attrs = (const jive::address_to_bitstring_operation *) attrs_;

	if (jive_node_isinstance(operand->node(), &JIVE_BITSTRING_TO_ADDRESS_NODE)) {
		const jive_bitstring_to_address_node * node;
		node = (const jive_bitstring_to_address_node *) operand->node();

		if (node->operation().nbits() != attrs->nbits())
			return jive_unop_reduction_none;

		if (node->operation().original_type() != attrs->original_type())
			return jive_unop_reduction_none;

		return jive_unop_reduction_inverse;
	}

	if (!type_contains_address(&operand->type()))
		return jive_unop_reduction_idempotent;

	return jive_unop_reduction_none;
}

static jive_output *
jive_address_to_bitstring_node_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node()->inputs[0]->origin();

	if (path == jive_unop_reduction_idempotent)
		return operand;

	return NULL;
}

jive_node *
jive_address_to_bitstring_node_create(struct jive_region * region,
	jive_output * address, size_t nbits, const jive_type * original_type)
{
	jive::address_to_bitstring_operation op(nbits, original_type);

	return jive_unary_operation_create_normalized(&JIVE_ADDRESS_TO_BITSTRING_NODE_, region->graph,
		&op, address)->node();
}

jive_output *
jive_address_to_bitstring_create(jive_output * address, size_t nbits,
	const jive_type * original_type)
{
	jive::address_to_bitstring_operation op(nbits, original_type);

	return jive_unary_operation_create_normalized(&JIVE_ADDRESS_TO_BITSTRING_NODE_,
		address->node()->graph, &op, address);
}


/* bitstring_to_address node */

static void
jive_bitstring_to_address_node_init_(jive_bitstring_to_address_node * self, jive_region * region,
	jive_output * bitstring, size_t nbits, jive_type * original_type);

static void
jive_bitstring_to_address_node_fini_(jive_node * self);

static bool
jive_bitstring_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static void
jive_bitstring_to_address_node_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context);

static jive_node *
jive_bitstring_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_unop_reduction_path_t
jive_bitstring_to_address_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * operand);

static jive_output *
jive_bitstring_to_address_node_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs, jive_output * operand);

const jive_unary_operation_class JIVE_BITSTRING_TO_ADDRESS_NODE_ = {
	base : { /* jive_node_class */
		parent : &JIVE_UNARY_OPERATION,
		name : "BITSTRING_TO_ADDRESS",
		fini : jive_bitstring_to_address_node_fini_, /* override */
		get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_node_get_label_, /* inherit */
		match_attrs : jive_bitstring_to_address_node_match_attrs_, /* override */
		check_operands : jive_bitstring_to_address_node_check_operands_, /* override */
		create : jive_bitstring_to_address_node_create_, /* override */
	},

	single_apply_over : NULL,
	multi_apply_over : NULL,

	can_reduce_operand : jive_bitstring_to_address_node_can_reduce_operand_, /* override */
	reduce_operand : jive_bitstring_to_address_node_reduce_operand_ /* override */
};

static void
jive_bitstring_to_address_node_init_(
	jive_bitstring_to_address_node * self,
	jive_region * region,
	jive_output * bitstring,
	size_t nbits,
	const jive_type * original_type)
{
	jive_context * context = region->graph->context;

	const jive_type * bittype = &bitstring->type();
	if (!dynamic_cast<const jive_value_type*>(bittype))
		jive_context_fatal_error(context, "Type mismatch: expected a value type.");

	jive_node_init_(self, region,
		1, &bittype, &bitstring,
		1, (const jive_type **)&original_type);
}

static void
jive_bitstring_to_address_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static bool
jive_bitstring_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::bitstring_to_address_operation * first = &((const jive_bitstring_to_address_node *)
		self)->operation();
	const jive::bitstring_to_address_operation * second =
		(const jive::bitstring_to_address_operation *) attrs;

	if (first->nbits() != second->nbits())
		return false;

	return first->original_type() == second->original_type();
}

static void
jive_bitstring_to_address_node_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	if (!dynamic_cast<jive_value_output*>(operands[0]))
		jive_context_fatal_error(context, "Type mismatch: expected a value type.");
}

static jive_node *
jive_bitstring_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive::bitstring_to_address_operation * attrs =
		(const jive::bitstring_to_address_operation *) attrs_;

	jive_bitstring_to_address_node * node = new jive_bitstring_to_address_node(*attrs);
	node->class_ = &JIVE_BITSTRING_TO_ADDRESS_NODE;
	jive_bitstring_to_address_node_init_(node, region, operands[0], attrs->nbits(),
		&attrs->original_type());

	return node;
}

static jive_unop_reduction_path_t
jive_bitstring_to_address_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand)
{
	const jive::bitstring_to_address_operation * attrs;
	attrs = (const jive::bitstring_to_address_operation *) attrs_;

	if (jive_node_isinstance(operand->node(), &JIVE_ADDRESS_TO_BITSTRING_NODE)) {
		const jive_address_to_bitstring_node * node;
		node = (const jive_address_to_bitstring_node *)operand->node();

		if (node->operation().nbits() != attrs->nbits())
			return jive_unop_reduction_none;

		if (node->operation().original_type() != attrs->original_type())
			return jive_unop_reduction_none;

		return jive_unop_reduction_inverse;
	}

	if (!type_contains_address(&attrs->original_type()))
		return jive_unop_reduction_idempotent;

	return jive_unop_reduction_none;
}

static jive_output *
jive_bitstring_to_address_node_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand)
{
	if (path == jive_unop_reduction_inverse)
		return operand->node()->inputs[0]->origin();

	if (path == jive_unop_reduction_idempotent)
		return operand;

	return NULL;
}

jive_node *
jive_bitstring_to_address_node_create(struct jive_region * region,
	jive_output * bitstring, size_t nbits, const jive_type * original_type)
{
	jive::bitstring_to_address_operation op(nbits, original_type);

	return jive_unary_operation_create_normalized(&JIVE_BITSTRING_TO_ADDRESS_NODE_, region->graph,
		&op, bitstring)->node();
}

jive_output *
jive_bitstring_to_address_create(jive_output * bitstring, size_t nbits,
	const jive_type * original_type)
{
	jive::bitstring_to_address_operation op(nbits, original_type);

	return jive_unary_operation_create_normalized(&JIVE_BITSTRING_TO_ADDRESS_NODE_,
		bitstring->node()->graph, &op, bitstring);
}

/* reductions */

void
jive_load_node_address_transform(jive_load_node * node, size_t nbits)
{
	const jive_node * node_ = node;

	bool input_is_address = dynamic_cast<jive::addr::input*>(node_->inputs[0]);
	bool output_is_address = dynamic_cast<jive::addr::output*>(node_->outputs[0]);

	if (!input_is_address && !output_is_address)
		return;

	jive_output * address = node_->inputs[0]->origin();
	if (input_is_address)
		address = jive_address_to_bitstring_create(address, nbits, &address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::output*>(address)->nbits() == nbits);

	jive::bits::type bits(nbits);
	const jive_value_type * datatype = &node->operation().datatype();
	if (output_is_address)
		datatype = &bits;

	size_t i;
	size_t nstates = node_->ninputs - 1;
	jive_output * states[nstates];
	for (i = 0; i < nstates; i++){
		states[i] = node_->inputs[i+1]->origin();
	}

	jive_output * load = jive_load_by_bitstring_create(address, nbits, datatype, nstates, states);
	
	if (output_is_address)
		load = jive_bitstring_to_address_create(load, nbits, &node->outputs[0]->type());
	
	jive_output_replace(node_->outputs[0], load);
}

void
jive_store_node_address_transform(jive_store_node * node, size_t nbits)
{
	const jive_node * node_ = node;

	bool input0_is_address = dynamic_cast<jive::addr::input*>(node_->inputs[0]);
	bool input1_is_address = dynamic_cast<jive::addr::input*>(node_->inputs[1]);

	if (!input0_is_address && !input1_is_address)
		return;

	jive_output * address = node_->inputs[0]->origin();
	if (input0_is_address)
		address = jive_address_to_bitstring_create(address, nbits, &address->type());

	JIVE_DEBUG_ASSERT(static_cast<const jive::bits::output*>(address)->nbits() == nbits);

	jive::bits::type bits(nbits);
	const jive_value_type * datatype = &node->operation().datatype();
	jive_output * value = node_->inputs[1]->origin();
	if(input1_is_address){
		datatype = &bits;
		value = jive_address_to_bitstring_create(node_->inputs[1]->origin(), nbits, &value->type());
	}

	size_t i;
	size_t nstates = node_->ninputs - 2;
	jive_output * states[nstates];
	for (i = 0; i < nstates; i++){
		states[i] = node_->inputs[i+2]->origin();
	}

	jive_node * store = jive_store_by_bitstring_node_create(node_->region, address, nbits,
		datatype, value, nstates, states);

	for (i = 0; i < nstates; i++){
		jive_output_replace(node_->outputs[i], store->outputs[i]);
	}
}

void
jive_label_to_address_node_address_transform(jive_label_to_address_node * node, size_t nbits)
{
	const jive_label * label = node->operation().label();

	jive_output * label_o = jive_label_to_bitstring_create(node->region->graph, label, nbits);
	jive_output * addr_o = jive_bitstring_to_address_create(label_o, nbits,
		&node->outputs[0]->type());
	jive_output_replace(node->outputs[0], addr_o);
}

void
jive_call_node_address_transform(jive_call_node * node, size_t nbits)
{
	const jive_node * node_ = node;

	size_t i;
	bool transform = false;
	jive_output * operands[node_->ninputs];
	for (i = 0; i < node_->ninputs; i++){
		if(dynamic_cast<jive::addr::input*>(node_->inputs[i])){
			operands[i] = jive_address_to_bitstring_create(node_->inputs[i]->origin(), nbits,
				&node->inputs[i]->origin()->type());
			transform = true;
		} else {
			operands[i] = node_->inputs[i]->origin();
		}
	}

	const jive_type * return_types[node_->noutputs];
	jive::bits::type address_type(nbits);
	for (i = 0; i < node_->noutputs; i++){
		if (dynamic_cast<jive::addr::output*>(node_->outputs[i])){
			return_types[i] = &address_type;
			transform = true;
		} else {
			return_types[i] = &node->outputs[i]->type();
		}
	}

	if (!transform)
		return;

	jive_node * call = jive_call_by_bitstring_node_create(node_->region, operands[0], nbits,
		node->operation().calling_convention(), node_->ninputs - 1, operands + 1, node_->noutputs,
		return_types);

	for (i = 0; i < node_->noutputs; i++){
		jive_output * output = call->outputs[i];
		if(dynamic_cast<jive::addr::output*>(node_->outputs[i]))
			output = jive_bitstring_to_address_create(call->outputs[i], nbits, &node->outputs[i]->type());
		jive_output_replace(node_->outputs[i], output);
	}
}

void
jive_memberof_node_address_transform(jive_memberof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t index = node->operation().index();
	const jive_record_declaration * decl = node->operation().record_decl();

	JIVE_DEBUG_ASSERT(index < decl->nelements);
	size_t elem_offset = jive_memlayout_mapper_map_record(mapper, decl)->element[index].offset;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * offset = jive_bitconstant_unsigned(node_->graph, nbits, elem_offset);
	jive_output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive_output * tmparray0[] = {address, offset};
	jive_output * sum = jive_bitsum(2, tmparray0);
	jive_output * off_address = jive_bitstring_to_address_create(sum, nbits,
		&node->outputs[0]->type());

	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_containerof_node_address_transform(
	jive_containerof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t index = node->operation().index();
	const jive_record_declaration * decl = node->operation().record_decl();

	JIVE_DEBUG_ASSERT(index < decl->nelements);
	size_t elem_offset = jive_memlayout_mapper_map_record(mapper, decl)->element[index].offset;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * offset = jive_bitconstant_unsigned(node_->graph, nbits, elem_offset);
	jive_output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive_output * sum = jive_bitdifference(address, offset);
	jive_output * off_address = jive_bitstring_to_address_create(sum, nbits,
		&node->outputs[0]->type());

	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_arraysubscript_node_address_transform(jive_arraysubscript_node * node,
	struct jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t elem_type_size = jive_memlayout_mapper_map_value_type(mapper,
		&node->operation().element_type())->total_size;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * index = node_->inputs[1]->origin();
	jive_output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive_output * elem_size = jive_bitconstant_unsigned(node_->graph, nbits, elem_type_size);
	jive_output * tmparray1[] = {elem_size, index};
	jive_output * offset = jive_bitmultiply(2, tmparray1);
	jive_output * tmparray2[] = {address, offset};
	jive_output * sum = jive_bitsum(2, tmparray2);
	jive_output * off_address = jive_bitstring_to_address_create(sum, nbits,
		&node->outputs[0]->type());
	
	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_arrayindex_node_address_transform(jive_arrayindex_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = node;

	size_t elem_type_size = jive_memlayout_mapper_map_value_type(mapper,
		&node->operation().element_type())->total_size;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * address1 = jive_address_to_bitstring_create(node_->inputs[0]->origin(), nbits,
		&node->inputs[0]->origin()->type());
	jive_output * address2 = jive_address_to_bitstring_create(node_->inputs[1]->origin(), nbits,
		&node->inputs[1]->origin()->type());
	jive_output * elem_size = jive_bitconstant_unsigned(node_->graph, nbits, elem_type_size);
	jive_output * diff = jive_bitdifference(address1, address2);
	jive_output * div = jive_bitsquotient(diff, elem_size);

	jive_output_replace(node_->outputs[0], div);
}

void
jive_apply_node_address_transform(const jive_apply_node * node, size_t nbits)
{
	jive_input * fct = node->inputs[0];

	const jive_type * fcttype = &fct->type();
	if (!type_contains_address(fcttype))
		return;

	size_t nresults = node->noutputs;
	size_t narguments = node->ninputs-1;

	size_t n;
	jive_output * arguments[narguments];
	for (n = 1; n < node->ninputs; n++) {
		jive_input * argument = node->inputs[n];
		arguments[n-1] = jive_address_to_bitstring_create(argument->origin(), nbits, &argument->type());
	}
	jive_output * function = jive_address_to_bitstring_create(fct->origin(), nbits, fcttype);

	jive_output * results[nresults];
	jive_apply_create(function, narguments, arguments, results);

	for (n = 0; n < nresults; n++) {
		jive_output * original = node->outputs[n];
		jive_output * substitute = jive_bitstring_to_address_create(results[n], nbits,
			&original->type());
		jive_output_replace(original, substitute);
	}
}

void
jive_lambda_node_address_transform(const jive_lambda_node * node, size_t nbits)
{
	JIVE_DEBUG_ASSERT(node->noutputs == 1);

	jive_graph * graph = node->graph;
	jive_context * context = graph->context;
	jive_output * fct = node->outputs[0];

	const jive_type * type = &fct->type();
	if (!type_contains_address(type))
		return;

	jive_node * enter = jive_lambda_node_get_enter_node(node);
	jive_node * leave = jive_lambda_node_get_leave_node(node);
	jive_region * region = jive_lambda_node_get_region(node);

	const jive_function_type * fcttype = dynamic_cast<const jive_function_type*>(type);
	jive_function_type * new_fcttype = (jive_function_type *) convert_address_to_bitstring_type(
		fcttype, nbits, context);

	size_t n;
	size_t nparameters = fcttype->narguments();
	const char * parameter_names[nparameters];
	for (n = 1; n < enter->noutputs; n++)
		parameter_names[n-1] = enter->outputs[n]->gate->name;

	const jive_type * argument_types[new_fcttype->narguments()];
	for (size_t i = 0; i < new_fcttype->narguments(); i++)
		argument_types[i] = new_fcttype->argument_type(i);

	jive_lambda * lambda = jive_lambda_begin(graph, new_fcttype->narguments(),
		argument_types, parameter_names);

	jive_substitution_map * map = jive_substitution_map_create(context);

	jive_node * new_enter = jive_region_get_top_node(lambda->region);
	for (n = 1; n < enter->noutputs; n++) {
		jive_output * parameter = jive_bitstring_to_address_create(new_enter->outputs[n], nbits,
			fcttype->argument_type(n-1));
		jive_substitution_map_add_output(map, enter->outputs[n], parameter);
	}

	jive_region_copy_substitute(region, lambda->region, map, false, false);

	size_t nresults = fcttype->nreturns();
	jive_output * results[nresults];
	for (n = 1; n < leave->ninputs; n++) {
		jive_output * substitute = jive_substitution_map_lookup_output(map, leave->inputs[n]->origin());
		results[n-1] = jive_address_to_bitstring_create(substitute, nbits, fcttype->return_type(n-1));
	}

	jive_substitution_map_destroy(map);

	const jive_type * return_types[new_fcttype->nreturns()];
	for (size_t i = 0; i < new_fcttype->nreturns(); i++)
		return_types[i] = new_fcttype->return_type(i);

	jive_output * new_fct = jive_lambda_end(lambda, new_fcttype->nreturns(), return_types, results);

	delete new_fcttype;
	jive_output_replace(fct, jive_bitstring_to_address_create(new_fct, nbits, type));
}

static void
convert_regions(const struct jive_region * region, jive_memlayout_mapper * mapper)
{
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		convert_regions(subregion, mapper);

	jive_node * anchor = jive_region_get_anchor_node(region);
	if (anchor == NULL)
		return;

	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	const jive_lambda_node * lambda = jive_lambda_node_const_cast(anchor);
	if (lambda != NULL)
		jive_lambda_node_address_transform(lambda, nbits);
}

void
jive_graph_address_transform(jive_graph * graph, jive_memlayout_mapper * mapper)
{
	convert_regions(graph->root_region, mapper);

	jive_traverser * traverser = jive_topdown_traverser_create(graph);
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_node * node = jive_traverser_next(traverser);
	for(; node; node = jive_traverser_next(traverser)){
		if (jive_node_isinstance(node, &JIVE_MEMBEROF_NODE))
			jive_memberof_node_address_transform(jive_memberof_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_CONTAINEROF_NODE))
			jive_containerof_node_address_transform(jive_containerof_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_ARRAYINDEX_NODE))
			jive_arrayindex_node_address_transform(jive_arrayindex_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_ARRAYSUBSCRIPT_NODE))
			jive_arraysubscript_node_address_transform(jive_arraysubscript_node_cast(node), mapper);
		else if (jive_node_isinstance(node, &JIVE_LABEL_TO_ADDRESS_NODE))
			jive_label_to_address_node_address_transform(jive_label_to_address_node_cast(node), nbits);
		else if (jive_node_isinstance(node, &JIVE_LOAD_NODE))
			jive_load_node_address_transform(jive_load_node_cast(node), nbits);
		else if (jive_node_isinstance(node, &JIVE_STORE_NODE))
			jive_store_node_address_transform(jive_store_node_cast(node), nbits);
		else if (jive_node_isinstance(node, &JIVE_CALL_NODE))
			jive_call_node_address_transform(jive_call_node_cast(node), nbits);

		const jive_apply_node * apply_node = jive_apply_node_const_cast(node);
		if (apply_node != NULL)
			jive_apply_node_address_transform(apply_node, nbits);
	}

	jive_traverser_destroy(traverser);
}
