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
	struct jive::output * const operands[],
	size_t nbits)
{
	const jive::base::type * operand_types[noperands];
	jive::bits::type * operand_type_structs[noperands];
	size_t n;
	
	for(n=0; n<noperands; n++) {
		size_t nbits = static_cast<const jive::bits::output*>(operands[n])->nbits();
		operand_type_structs[n] = new jive::bits::type(nbits);
		operand_types[n] = operand_type_structs[n];
	}
	
	jive::bits::type output_type(nbits);
	const jive::base::type * type_array[] = {&output_type};
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
	jive::output * const operands[])
{
	size_t nbits = 0, n;
	for(n=0; n<noperands; n++)
		nbits += static_cast<const jive::bits::output*>(operands[n])->nbits();
	jive_bitstring_multiop_node_init_(self, region, noperands, operands, nbits);
}

const jive_node_class JIVE_BITCONCAT_NODE = {
	/* note that parent is JIVE_BINARY_OPERATION, not
	JIVE_BITBINARY_OPERATION: the latter one is assumed
	to represent "width-preserving" bit operations (i.e.
	number of bits per operand/output matches), while
	the concat operator violates this assumption */
	parent : &JIVE_BINARY_OPERATION,
	name : "BITCONCAT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitconcat(size_t noperands, struct jive::output * const * operands)
{
	JIVE_DEBUG_ASSERT(noperands != 0);

	jive_graph * graph = operands[0]->node()->graph;
	jive::bits::concat_operation op;
	return jive_binary_operation_create_normalized(&JIVE_BITCONCAT_NODE, graph,
		&op, noperands, operands);
}

namespace jive {
namespace bits {

concat_operation::~concat_operation() noexcept
{
}

bool
concat_operation::operator==(const operation & other) const noexcept
{
	const concat_operation * o = dynamic_cast<const concat_operation *>(&other);
	return o != nullptr;
}

jive_node *
concat_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments >= 2);

	jive_node * node = jive::create_operation_node(jive::bits::concat_operation());
	node->class_ = &JIVE_BITCONCAT_NODE;
	jive_bitconcat_node_init_(node, region, narguments, arguments);
	return node;
}


jive_binop_reduction_path_t
concat_operation::can_reduce_operand_pair(
	const jive::output * arg1,
	const jive::output * arg2) const noexcept
{
	const constant_operation * arg1_constant = dynamic_cast<const constant_operation *>(
		&arg1->node()->operation());
	const constant_operation * arg2_constant = dynamic_cast<const constant_operation *>(
		&arg2->node()->operation());

	if (arg1_constant && arg2_constant) {
		return jive_binop_reduction_constants;
	}

	const slice_operation * arg1_slice = dynamic_cast<const slice_operation *>(
		&arg1->node()->operation());
	const slice_operation * arg2_slice = dynamic_cast<const slice_operation *>(
		&arg2->node()->operation());

	if (arg1_slice && arg2_slice){
		jive::output * origin1 = arg1->node()->inputs[0]->origin();
		jive::output * origin2 = arg2->node()->inputs[0]->origin();

		if (origin1 == origin2 && arg1_slice->high() == arg2_slice->low()) {
			return jive_binop_reduction_merge;
		}

		/* FIXME: support sign bit */
	}

	return jive_binop_reduction_none;
}

jive::output *
concat_operation::reduce_operand_pair(
	jive_binop_reduction_path_t path,
	jive::output * arg1,
	jive::output * arg2) const
{
	jive_graph * graph = arg1->node()->graph;

	if (path == jive_binop_reduction_constants) {
		const constant_operation * arg1_constant = static_cast<const constant_operation *>(
			&arg1->node()->operation());
		const constant_operation * arg2_constant = static_cast<const constant_operation *>(
			&arg2->node()->operation());

		size_t nbits = arg1_constant->bits.size() + arg2_constant->bits.size();
		char bits[nbits];
		memcpy(bits, &arg1_constant->bits[0], arg1_constant->bits.size());
		memcpy(bits + arg1_constant->bits.size(), &arg2_constant->bits[0], arg2_constant->bits.size());

		return jive_bitconstant(graph, nbits, bits);
	}

	if (path == jive_binop_reduction_merge) {
		const slice_operation * arg1_slice = static_cast<const slice_operation *>(
			&arg1->node()->operation());
		const slice_operation * arg2_slice = static_cast<const slice_operation *>(
			&arg2->node()->operation());

		jive::output * origin1 = arg1->node()->inputs[0]->origin();

		return jive_bitslice(origin1, arg1_slice->low(), arg2_slice->high());

		/* FIXME: support sign bit */
	}

	return NULL;
}

jive_binary_operation_flags
concat_operation::flags() const noexcept
{
	return jive_binary_operation_associative;
}

std::string
concat_operation::debug_string() const
{
	return "BITCONCAT";
}

}
}
