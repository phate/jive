/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/concat.h>

#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/slice.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

static void
jive_bitstring_multiop_node_init_(
	jive_node * self,
	struct jive_region * region,
	size_t noperands,
	struct jive_output * const operands[],
	size_t nbits)
{
	const jive_type * operand_types[noperands];
	jive_bitstring_type * operand_type_structs[noperands];
	size_t n;
	
	for(n=0; n<noperands; n++) {
		size_t nbits = ((const jive_bitstring_type *)&operands[n]->type())->nbits();
		operand_type_structs[n] = new jive_bitstring_type(nbits);
		operand_types[n] = operand_type_structs[n];
	}
	
	jive_bitstring_type output_type(nbits);
	const jive_type * type_array[] = {&output_type};
	jive_node_init_(self, region,
		noperands, operand_types, operands,
		1, type_array);

	for (n = 0; n < noperands; n++)
		delete operand_type_structs[n];
}

static void
jive_bitconcat_node_init_(
	jive_node * self,
	jive_region * region,
	size_t noperands,
	jive_output * const operands[])
{
	size_t nbits = 0, n;
	for(n=0; n<noperands; n++)
		nbits += ((const jive_bitstring_type *)&operands[n]->type())->nbits();
	jive_bitstring_multiop_node_init_(self, region, noperands, operands, nbits);
}

static void
jive_bitconcat_node_get_label_(const jive_node * self, struct jive_buffer * buffer);

static void
jive_bitconcat_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context);

static jive_node *
jive_bitconcat_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static jive_binop_reduction_path_t
jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2);

static jive_output *
jive_bitconcat_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * op1, jive_output * op2);

const jive_binary_operation_class JIVE_BITCONCAT_NODE_ = {
	base : { /* jive_node_class */
		/* note that parent is JIVE_BINARY_OPERATION, not
		JIVE_BITBINARY_OPERATION: the latter one is assumed
		to represent "width-preserving" bit operations (i.e.
		number of bits per operand/output matches), while
		the concat operator violates this assumption */
		parent : &JIVE_BINARY_OPERATION,
		name : "BITCONCAT",
		fini : jive_node_fini_, /* inherit */
		get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
		get_label : jive_bitconcat_node_get_label_, /* override */
		match_attrs : jive_node_match_attrs_, /* inherit */
		check_operands : jive_bitconcat_node_check_operands_, /* override */
		create : jive_bitconcat_node_create_, /* override */
	},
	
	flags : jive_binary_operation_associative,
	single_apply_under : NULL,
	multi_apply_under : NULL,
	distributive_over : NULL,
	distributive_under : NULL,
	
	can_reduce_operand_pair : jive_bitconcat_can_reduce_operand_pair_, /* override */
	reduce_operand_pair : jive_bitconcat_reduce_operand_pair_ /* override */
};

static void
jive_bitconcat_node_get_label_(const jive_node * self, struct jive_buffer * buffer)
{
	jive_buffer_putstr(buffer, "BITCONCAT");
}

static void
jive_bitconcat_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[], jive_context * context)
{
	if (noperands == 0)
		jive_context_fatal_error(context, "Bitconcat needs at least one operand.");

	size_t n;
	for (n = 0; n < noperands; n++) {
		const jive_bitstring_output * output = dynamic_cast<const jive_bitstring_output*>(operands[n]);
		if (!output)
			jive_context_fatal_error(context, "bitconcat node requires bitstring operands.");

		size_t nbits = jive_bitstring_output_nbits(output);
		if (nbits == 0)
			jive_context_fatal_error(context,
				"Type mismatch: length of bitstring must be greater than zero.");
	}
}

static jive_node *
jive_bitconcat_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	JIVE_DEBUG_ASSERT(noperands >= 2);

	jive_node * node = jive::create_operation_node(jive::bitstring::concat_operation());
	node->class_ = &JIVE_BITCONCAT_NODE;
	jive_bitconcat_node_init_(node, region, noperands, operands);
	return node;
}

static jive_binop_reduction_path_t
jive_bitconcat_can_reduce_operand_pair_(const jive_node_class * cls,
	const jive_node_attrs * attrs, const jive_output * op1, const jive_output * op2)
{
	if(jive_node_isinstance(op1->node(), &JIVE_BITCONSTANT_NODE) &&
		jive_node_isinstance(op2->node(), &JIVE_BITCONSTANT_NODE))
		return jive_binop_reduction_constants;

	const jive_bitslice_node * n1 = dynamic_cast<jive_bitslice_node *>(op1->node());
	const jive_bitslice_node * n2 = dynamic_cast<jive_bitslice_node *>(op2->node());

	if(n1 && n2){
		jive_output * origin1 = op1->node()->inputs[0]->origin();
		jive_output * origin2 = op2->node()->inputs[0]->origin();

		if (origin1 != origin2)
			return jive_binop_reduction_none;

		if (n1->operation().high() == n2->operation().low())
			return jive_binop_reduction_merge;

		/* FIXME: support sign bit */
	}

	return jive_binop_reduction_none;
}

static jive_output *
jive_bitconcat_reduce_operand_pair_(jive_binop_reduction_path_t path, const jive_node_class * cls,
	const jive_node_attrs * attrs, jive_output * op1, jive_output * op2)
{
	jive_graph * graph = op1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const jive_bitconstant_node * n1 = (const jive_bitconstant_node *) op1->node();
		const jive_bitconstant_node * n2 = (const jive_bitconstant_node *) op2->node();

		size_t nbits = n1->operation().bits.size() + n2->operation().bits.size();
		char bits[nbits];
		memcpy(bits, &n1->operation().bits[0], n1->operation().bits.size());
		memcpy(bits + n1->operation().bits.size(), &n2->operation().bits[0], n2->operation().bits.size());

		return jive_bitconstant(graph, nbits, bits);
	}

	if (path == jive_binop_reduction_merge) {
		const jive_bitslice_node * n1 = (const jive_bitslice_node *) op1->node();
		const jive_bitslice_node * n2 = (const jive_bitslice_node *) op2->node();
		jive_output * origin1 = op1->node()->inputs[0]->origin();

		return jive_bitslice(origin1, n1->operation().low(), n2->operation().high());

		/* FIXME: support sign bit */
	}

	return NULL;
}

jive_output *
jive_bitconcat(size_t noperands, struct jive_output * const * operands)
{
	JIVE_DEBUG_ASSERT(noperands != 0);

	jive_graph * graph = operands[0]->node()->graph;
	jive::bitstring::concat_operation op;
	return jive_binary_operation_create_normalized(&JIVE_BITCONCAT_NODE_, graph,
		&op, noperands, operands);
}
