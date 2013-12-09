/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/address-transform.h>

#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/arch/memlayout.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>
#include <jive/types/bitstring/arithmetic/bitnegate.h>

/* address_to_bitstring node */

static void
jive_address_to_bitstring_node_init_(jive_address_to_bitstring_node * self, jive_region * region,
	jive_output * address, size_t nbits);

static const jive_node_attrs *
jive_address_to_bitstring_node_get_attrs_(const jive_node * self);

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
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.name = "ADDRESS_TO_BITSTRING",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_address_to_bitstring_node_get_attrs_, /* override */
		.match_attrs = jive_address_to_bitstring_node_match_attrs_, /* override */
		.check_operands = jive_address_to_bitstring_node_check_operands_, /* override */
		.create = jive_address_to_bitstring_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.single_apply_over = NULL,
	.multi_apply_over = NULL,
	
	.can_reduce_operand = jive_address_to_bitstring_node_can_reduce_operand_, /* override */
	.reduce_operand = jive_address_to_bitstring_node_reduce_operand_ /* override */
};

static void
jive_address_to_bitstring_node_init_(
	jive_address_to_bitstring_node * self,
	jive_region * region,
	jive_output * address,
	size_t nbits)
{
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);

	jive_node_init_(&self->base, region,
		1, &addrtype, &address,
		1, &btype);

	self->attrs.nbits = nbits;
}

static const jive_node_attrs *
jive_address_to_bitstring_node_get_attrs_(const jive_node * self_)
{
	const jive_address_to_bitstring_node * self = (const jive_address_to_bitstring_node *) self_;
	
	return &self->attrs.base;
}

static bool
jive_address_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_address_to_bitstring_node_attrs * first = &((const jive_address_to_bitstring_node *)
		self)->attrs;
	const jive_address_to_bitstring_node_attrs * second =
		(const jive_address_to_bitstring_node_attrs *) attrs;
	
	return (first->nbits == second->nbits);
}

static void
jive_address_to_bitstring_node_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	if(!jive_address_output_const_cast(operands[0]))
		jive_raise_type_error(addrtype, jive_output_get_type(operands[0]), context);
}

static jive_node *
jive_address_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive_address_to_bitstring_node_attrs * attrs =
		(const jive_address_to_bitstring_node_attrs *) attrs_;

	jive_address_to_bitstring_node * node = jive_context_malloc(region->graph->context,
		sizeof(*node));
	node->base.class_ = &JIVE_ADDRESS_TO_BITSTRING_NODE;
	jive_address_to_bitstring_node_init_(node, region, operands[0], attrs->nbits);

	return &node->base;
}

static jive_unop_reduction_path_t
jive_address_to_bitstring_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand_)
{
	const jive_address_to_bitstring_node_attrs * attrs =
		(const jive_address_to_bitstring_node_attrs *) attrs_;

	if (jive_node_isinstance(operand_->node, &JIVE_BITSTRING_TO_ADDRESS_NODE)) {
		const jive_bitstring_to_address_node * node = (const jive_bitstring_to_address_node *)
			operand_->node;
		JIVE_DEBUG_ASSERT(node->attrs.nbits == attrs->nbits);
		return jive_unop_reduction_inverse;
	}

	return jive_unop_reduction_none;
}

static jive_output * 
jive_address_to_bitstring_node_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand_)
{
	if (path == jive_unop_reduction_inverse)
		return operand_->node->inputs[0]->origin;

	return NULL;
}

jive_node *
jive_address_to_bitstring_node_create(struct jive_region * region,
	jive_output * address, size_t nbits)
{
	jive_address_to_bitstring_node_attrs attrs;
	attrs.nbits = nbits; 

	return jive_unary_operation_create_normalized(&JIVE_ADDRESS_TO_BITSTRING_NODE_, region->graph,
		&attrs.base, address)->node;
}

jive_output *
jive_address_to_bitstring_create(jive_output * address, size_t nbits)
{
	jive_address_to_bitstring_node_attrs attrs;
	attrs.nbits = nbits; 

	return jive_unary_operation_create_normalized(&JIVE_ADDRESS_TO_BITSTRING_NODE_,
		address->node->graph, &attrs.base, address);
}


/* bitstring_to_address node */

static void
jive_bitstring_to_address_node_init_(jive_bitstring_to_address_node * self, jive_region * region,
	jive_output * bitstring, size_t nbits);

static const jive_node_attrs *
jive_bitstring_to_address_node_get_attrs_(const jive_node * self);

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
	.base = { /* jive_node_class */
		.parent = &JIVE_UNARY_OPERATION,
		.name = "BITSTRING_TO_ADDRESS",
		.fini = jive_node_fini_, /* inherit */
		.get_default_normal_form = jive_unary_operation_get_default_normal_form_, /* inherit */
		.get_label = jive_node_get_label_, /* inherit */
		.get_attrs = jive_bitstring_to_address_node_get_attrs_, /* override */
		.match_attrs = jive_bitstring_to_address_node_match_attrs_, /* override */
		.check_operands = jive_bitstring_to_address_node_check_operands_, /* override */
		.create = jive_bitstring_to_address_node_create_, /* override */
		.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
	},

	.single_apply_over = NULL,
	.multi_apply_over = NULL,

	.can_reduce_operand = jive_bitstring_to_address_node_can_reduce_operand_, /* override */
	.reduce_operand = jive_bitstring_to_address_node_reduce_operand_ /* override */	
};

static void
jive_bitstring_to_address_node_init_(
	jive_bitstring_to_address_node * self,
	jive_region * region,
	jive_output * bitstring,
	size_t nbits)
{
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);

	jive_node_init_(&self->base, region,
		1, &btype, &bitstring,
		1, &addrtype);

	self->attrs.nbits = nbits;
}

static const jive_node_attrs *
jive_bitstring_to_address_node_get_attrs_(const jive_node * self_)
{
	const jive_bitstring_to_address_node * self = (const jive_bitstring_to_address_node *) self_;

	return &self->attrs.base;
}

static bool
jive_bitstring_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_bitstring_to_address_node_attrs * first = &((const jive_bitstring_to_address_node *)
		self)->attrs;
	const jive_bitstring_to_address_node_attrs * second =
		(const jive_bitstring_to_address_node_attrs *) attrs;

	return (first->nbits == second->nbits);
}

static void
jive_bitstring_to_address_node_check_operands_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, size_t noperands, jive_output * const operands[],
	jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive_bitstring_to_address_node_attrs * attrs;
	attrs = (const jive_bitstring_to_address_node_attrs *)attrs_;

	JIVE_DECLARE_BITSTRING_TYPE(bstype, attrs->nbits);
	if (!jive_type_equals(bstype, jive_output_get_type(operands[0])))
		jive_raise_type_error(bstype, jive_output_get_type(operands[0]), context);
}

static jive_node *
jive_bitstring_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive_bitstring_to_address_node_attrs * attrs =
		(const jive_bitstring_to_address_node_attrs *) attrs_;

	jive_bitstring_to_address_node * node = jive_context_malloc(region->graph->context,
		sizeof(*node));
	node->base.class_ = &JIVE_BITSTRING_TO_ADDRESS_NODE;
	jive_bitstring_to_address_node_init_(node, region, operands[0], attrs->nbits);

	return &node->base; 
}

static jive_unop_reduction_path_t 
jive_bitstring_to_address_node_can_reduce_operand_(const jive_node_class * cls,
	const jive_node_attrs * attrs_, const jive_output * operand_)
{
	const jive_bitstring_to_address_node_attrs * attrs =
		(const jive_bitstring_to_address_node_attrs *) attrs_;

	if (jive_node_isinstance(operand_->node, &JIVE_ADDRESS_TO_BITSTRING_NODE)) {
		const jive_address_to_bitstring_node * node = (const jive_address_to_bitstring_node *)
			operand_->node;
		JIVE_DEBUG_ASSERT(node->attrs.nbits == attrs->nbits);
		return jive_unop_reduction_inverse;
	}

	return jive_unop_reduction_none;
}

static jive_output * 
jive_bitstring_to_address_node_reduce_operand_(jive_unop_reduction_path_t path,
	const jive_node_class * cls, const jive_node_attrs * attrs_, jive_output * operand_)
{
	if (path == jive_unop_reduction_inverse)
		return operand_->node->inputs[0]->origin;

	return NULL;
}

jive_node *
jive_bitstring_to_address_node_create(struct jive_region * region,
	jive_output * bitstring, size_t nbits)
{
	jive_bitstring_to_address_node_attrs attrs;
	attrs.nbits = nbits; 

	return jive_unary_operation_create_normalized(&JIVE_BITSTRING_TO_ADDRESS_NODE_, region->graph,
		&attrs.base, bitstring)->node;
}

jive_output *
jive_bitstring_to_address_create(jive_output * bitstring, size_t nbits)
{
	jive_bitstring_to_address_node_attrs attrs;
	attrs.nbits = nbits;

	return jive_unary_operation_create_normalized(&JIVE_BITSTRING_TO_ADDRESS_NODE_,
		bitstring->node->graph, &attrs.base, bitstring);
}

/* reductions */

void
jive_load_node_address_transform(jive_load_node * node, size_t nbits)
{
	const jive_node * node_ = &node->base;

	bool input_is_address = jive_input_isinstance(node_->inputs[0], &JIVE_ADDRESS_INPUT);
	bool output_is_address = jive_output_isinstance(node_->outputs[0], &JIVE_ADDRESS_OUTPUT);

	if (!input_is_address && !output_is_address)
		return;

	jive_output * address = node_->inputs[0]->origin;
	if (input_is_address)
		address = jive_address_to_bitstring_create(node_->inputs[0]->origin, nbits);

	JIVE_DEBUG_ASSERT(jive_bitstring_output_nbits((const jive_bitstring_output *) address) == nbits);

	JIVE_DECLARE_BITSTRING_TYPE(bits, nbits);
	const jive_value_type * datatype = node->attrs.datatype;
	if (output_is_address)
		datatype = jive_value_type_cast(bits); 

	size_t i;
	size_t nstates = node_->ninputs - 1;
	jive_output * states[nstates];
	for (i = 0; i < nstates; i++){
		states[i] = node_->inputs[i+1]->origin;
	}

	jive_output * load = jive_load_by_bitstring_create(address, nbits, datatype, nstates, states);
	
	if (output_is_address)
		load = jive_bitstring_to_address_create(load, nbits);
	
	jive_output_replace(node_->outputs[0], load);
}

void
jive_store_node_address_transform(jive_store_node * node, size_t nbits)
{
	const jive_node * node_ = &node->base;

	bool input0_is_address = jive_input_isinstance(node_->inputs[0], &JIVE_ADDRESS_INPUT);
	bool input1_is_address = jive_input_isinstance(node_->inputs[1], &JIVE_ADDRESS_INPUT);

	if (!input0_is_address && !input1_is_address)
		return;

	jive_output * address = node_->inputs[0]->origin;
	if (input0_is_address)
		address = jive_address_to_bitstring_create(node_->inputs[0]->origin, nbits);

	JIVE_DEBUG_ASSERT(jive_bitstring_output_nbits((const jive_bitstring_output *) address) == nbits);

	JIVE_DECLARE_BITSTRING_TYPE(bits, nbits);
	const jive_value_type * datatype = node->attrs.datatype;
	jive_output * value = node_->inputs[1]->origin;
	if(input1_is_address){
		datatype = jive_value_type_cast(bits);
		value = jive_address_to_bitstring_create(node_->inputs[1]->origin, nbits);
	}

	size_t i;
	size_t nstates = node_->ninputs - 2;
	jive_output * states[nstates];
	for (i = 0; i < nstates; i++){
		states[i] = node_->inputs[i+2]->origin;
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
	const jive_node * node_ = &node->base;
	
	const jive_label * label = jive_label_to_address_node_get_label(node);

	jive_output * label_o = jive_label_to_bitstring_create(node_->region->graph, label, nbits);
	jive_output * addr_o = jive_bitstring_to_address_create(label_o, nbits);
	jive_output_replace(node_->outputs[0], addr_o);
}

void
jive_call_node_address_transform(jive_call_node * node, size_t nbits)
{
	const jive_node * node_ = &node->base;

	size_t i;
	bool transform = false;
	jive_output * operands[node_->ninputs];
	for (i = 0; i < node_->ninputs; i++){
		if(jive_input_isinstance(node_->inputs[i], &JIVE_ADDRESS_INPUT)){
			operands[i] = jive_address_to_bitstring_create(node_->inputs[i]->origin, nbits);	
			transform = true;
		} else {
			operands[i] = node_->inputs[i]->origin;
		}
	}

	const jive_type * return_types[node_->noutputs];
	JIVE_DECLARE_BITSTRING_TYPE(address_type, nbits);
	for (i = 0; i < node_->noutputs; i++){
		if(jive_output_isinstance(node_->outputs[i], &JIVE_ADDRESS_OUTPUT)){
			return_types[i] = address_type;			
			transform = true;
		} else {
			return_types[i] = jive_output_get_type(node_->outputs[i]);
		}
	}

	if (!transform)
		return;

	jive_node * call = jive_call_by_bitstring_node_create(node_->region, operands[0], nbits,
		node->attrs.calling_convention, node_->ninputs - 1, operands + 1, node_->noutputs,
		return_types);

	for (i = 0; i < node_->noutputs; i++){
		jive_output * output = call->outputs[i];
		if(jive_output_isinstance(node_->outputs[i], &JIVE_ADDRESS_OUTPUT))
			output = jive_bitstring_to_address_create(call->outputs[i], nbits);
		jive_output_replace(node_->outputs[i], output);
	}
}

void
jive_memberof_node_address_transform(jive_memberof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = &node->base;

	size_t index = node->attrs.index;
	const jive_record_declaration * decl = node->attrs.record_decl;

	JIVE_DEBUG_ASSERT(index < decl->nelements);
	size_t elem_offset = jive_memlayout_mapper_map_record(mapper, decl)->element[index].offset;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * offset = jive_bitconstant_unsigned(node_->graph, nbits, elem_offset);
	jive_output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin, nbits);
	jive_output * sum = jive_bitsum(2, (jive_output *[]){address, offset});
	jive_output * off_address = jive_bitstring_to_address_create(sum, nbits);

	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_containerof_node_address_transform(jive_containerof_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = &node->base;

	size_t index = node->attrs.index;
	const jive_record_declaration * decl = node->attrs.record_decl;

	JIVE_DEBUG_ASSERT(index < decl->nelements);
	size_t elem_offset = jive_memlayout_mapper_map_record(mapper, decl)->element[index].offset;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * offset = jive_bitconstant_unsigned(node_->graph, nbits, elem_offset);
	jive_output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin, nbits);
	jive_output * sum = jive_bitdifference(address, offset);
	jive_output * off_address = jive_bitstring_to_address_create(sum, nbits);

	jive_output_replace(node_->outputs[0], off_address);
}

void
jive_arraysubscript_node_address_transform(jive_arraysubscript_node * node,
	struct jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = &node->base;

	size_t elem_type_size = jive_memlayout_mapper_map_value_type(mapper,
		node->attrs.element_type)->total_size;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8; 

	jive_output * index = node_->inputs[1]->origin;
	jive_output * address = jive_address_to_bitstring_create(node_->inputs[0]->origin, nbits);
	jive_output * elem_size = jive_bitconstant_unsigned(node_->graph, nbits, elem_type_size);
	jive_output * offset = jive_bitmultiply(2, (jive_output *[]){elem_size, index});
	jive_output * sum = jive_bitsum(2, (jive_output *[]){address, offset});
	jive_output * off_address = jive_bitstring_to_address_create(sum, nbits);
	
	jive_output_replace(node_->outputs[0], off_address);	
}

void
jive_arrayindex_node_address_transform(jive_arrayindex_node * node, jive_memlayout_mapper * mapper)
{
	const jive_node * node_ = &node->base;

	size_t elem_type_size = jive_memlayout_mapper_map_value_type(mapper,
		node->attrs.element_type)->total_size;
	size_t nbits = jive_memlayout_mapper_map_address(mapper)->total_size * 8;

	jive_output * address1 = jive_address_to_bitstring_create(node_->inputs[0]->origin, nbits);
	jive_output * address2 = jive_address_to_bitstring_create(node_->inputs[1]->origin, nbits);
	jive_output * elem_size = jive_bitconstant_unsigned(node_->graph, nbits, elem_type_size);
	jive_output * diff = jive_bitdifference(address1, address2);
	jive_output * div = jive_bitsquotient(diff, elem_size);

	jive_output_replace(node_->outputs[0], div);
}

void
jive_graph_address_transform(jive_graph * graph, jive_memlayout_mapper * mapper)
{
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
	}

	jive_traverser_destroy(traverser);
}

