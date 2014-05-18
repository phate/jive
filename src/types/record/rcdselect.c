/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record.h>

#include <stdio.h>
#include <string.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/load.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

static void
jive_select_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static jive_node *
jive_select_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
jive_select_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static void
jive_select_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_unop_reduction_path_t
jive_select_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs,
	const jive_output * operand);

static jive_output *
jive_select_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * operand);

const jive_unary_operation_class JIVE_SELECT_NODE_ = {
	base : { /* jive_node_class */
		parent : &JIVE_UNARY_OPERATION,
		name : "SELECT",
		fini : jive_node_fini_, /* inherit */
		get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_select_node_get_label_, /* override */
		match_attrs : jive_select_node_match_attrs_, /* override */
		check_operands : jive_select_node_check_operands_, /* override */
		create : jive_select_node_create_,	/* override */
	},

	single_apply_over : NULL,
	multi_apply_over : NULL,
	
	can_reduce_operand : jive_select_can_reduce_operand_, /* override */
	reduce_operand : jive_select_reduce_operand_ /* override */
};

static void
perform_check(jive_context * context, const jive_output * operand, size_t element)
{
	if (!dynamic_cast<const jive_record_output*>(operand))
		jive_context_fatal_error(context, "Type mismatch: need 'record' type as input to 'select' node");
	const jive_record_type * operand_type = (const jive_record_type *)
		jive_output_get_type(operand);
	
	if (element >= operand_type->declaration()->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp),
			"Type mismatch: attempted to select element #%zd from record of %zd elements",
			element, operand_type->declaration()->nelements);
		jive_context_fatal_error(context, jive_context_strdup(context, tmp));
	}
}

static void
jive_select_node_init_(jive_select_node * self, struct jive_region * region,
	size_t element, jive_output * operand)
{
	jive_context * context = region->graph->context;
	perform_check(context, operand, element);
	
	const jive_record_type * operand_type = (const jive_record_type *)
		jive_output_get_type(operand);
	const jive_type * output_type = operand_type->declaration()->elements[element];
	const jive_type *  tmparray0[] = {jive_output_get_type(operand)};
	jive_node_init_(self, region,
		1, tmparray0, &operand,
		1, &output_type);
}

static void
jive_select_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_select_node * self = (const jive_select_node *) self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SELECT(%zd)", self->operation().element);
	jive_buffer_putstr(buffer, tmp);
}

static const jive_node_attrs *
jive_select_node_get_attrs_(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *)self_;

	return &self->operation();
}

static bool
jive_select_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::rcd::select_operation * first = &((const jive_select_node *)self)->operation();
	const jive::rcd::select_operation * second = (const jive::rcd::select_operation *) attrs;

	if(first->element != second->element)
		return false;

	return true;
}

static void
jive_select_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive::rcd::select_operation * attrs = (const jive::rcd::select_operation *)attrs_;

	const jive_record_output * output = dynamic_cast<const jive_record_output*>(operands[0]);
	if (!output)
		jive_context_fatal_error(context, "Type mismatch: need 'record' type as input to 'select' node");

	const jive_record_type * type = (const jive_record_type *)jive_output_get_type(operands[0]);
	if (attrs->element >= type->declaration()->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp),
			"Type mismatch: attempted to select element #%zd from record of %zd elements", attrs->element,
			type->declaration()->nelements);
		jive_context_fatal_error(context, tmp);
	}
}

static jive_node *
jive_select_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands == 1);

	const jive::rcd::select_operation * attrs = (const jive::rcd::select_operation *)attrs_;
	jive_select_node * node = new jive_select_node(*attrs);
	node->class_ = &JIVE_SELECT_NODE;
	jive_select_node_init_(node, region, attrs->element, operands[0]);

	return node;
}

static const jive_unop_reduction_path_t jive_select_reduction_load = 128;

static jive_unop_reduction_path_t
jive_select_can_reduce_operand_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	const jive_output * operand)
{
	const jive::rcd::select_operation * attrs = (const jive::rcd::select_operation *) attrs_;
	
	perform_check(operand->node->graph->context, operand, attrs->element);
	
	if (jive_node_isinstance(operand->node, &JIVE_GROUP_NODE))
		return jive_unop_reduction_inverse;

	if (jive_node_isinstance(operand->node, &JIVE_LOAD_NODE))
		return jive_select_reduction_load;

	return jive_unop_reduction_none;
}

static jive_output *
jive_select_reduce_operand_(jive_unop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs_, jive_output * operand)
{
	const jive::rcd::select_operation * attrs = (const jive::rcd::select_operation *) attrs_;

	if (path == jive_unop_reduction_inverse)
		return operand->node->inputs[attrs->element]->origin();

	if (path == jive_select_reduction_load) {
		jive_node * load_node = operand->node;
		jive_output * address = load_node->inputs[0]->origin();

		size_t nbits = 0;
		if (dynamic_cast<jive_bitstring_output*>(address)) {
			nbits = jive_bitstring_output_nbits((const jive_bitstring_output *) address);
			address = jive_bitstring_to_address_create(address, nbits, jive_output_get_type(address));
		}
		
		const jive_record_declaration * decl = ((const jive_record_type *)
			jive_output_get_type(load_node->outputs[0]))->declaration();

		size_t n;
		size_t nstates = load_node->ninputs-1;
		jive_output * states[nstates];
		for(n = 0; n < nstates; n++)
			states[n] = load_node->inputs[n+1]->origin();

		jive_output * element_address = jive_memberof(address, decl, attrs->element);
		if (dynamic_cast<jive_address_output*>(address)) {
			return jive_load_by_address_create(element_address, decl->elements[attrs->element],
				nstates, states);
		} else {
			return jive_load_by_bitstring_create(element_address, nbits, decl->elements[attrs->element],
				nstates, states);
		}
	}

	return NULL;
}

jive_output *
jive_select_create(size_t member, jive_output * operand)
{
	jive::rcd::select_operation op;
	op.element = member;

	return jive_unary_operation_create_normalized(&JIVE_SELECT_NODE_, operand->node->graph,
		&op, operand);
}

