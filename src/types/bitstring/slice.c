/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/slice.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/concat.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

const jive_node_class JIVE_BITSLICE_NODE = {
	/* note that parent is JIVE_UNARY_OPERATION, not
	JIVE_BITUNARY_OPERATION: the latter one is assumed
	to represent "width-preserving" bit operations (i.e.
	number of bits per operand/output matches), while
	the slice operator violates this assumption */
	parent : &JIVE_UNARY_OPERATION,
	name : "BITSLICE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

namespace jive {
namespace bits {

slice_operation::~slice_operation() noexcept
{
}

bool
slice_operation::operator==(const operation & other) const noexcept
{
	const slice_operation * op = dynamic_cast<const slice_operation *>(&other);
	return op && op->argument_type_ == argument_type_ && op->low() == low() && op->high() == high();
}

jive_node *
slice_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments == 1);
	
	jive_bitslice_node * node = new jive_bitslice_node(*this);
	node->class_ = &JIVE_BITSLICE_NODE;

	const jive::base::type * input_typeptr = &argument_type(0);
	const jive::base::type * output_typeptr = &result_type(0);
	jive_node_init_(node, region,
		1, &input_typeptr, arguments,
		1, &output_typeptr);

	return node;
}

std::string
slice_operation::debug_string() const
{
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SLICE[%zd:%zd)", low(), high());
	return tmp;
}

const jive::base::type &
slice_operation::argument_type(size_t index) const noexcept
{
	return argument_type_;
}

const jive::base::type &
slice_operation::result_type(size_t index) const noexcept
{
	return result_type_;
}

jive_unop_reduction_path_t
slice_operation::can_reduce_operand(const jive::output * arg) const noexcept
{
	const jive::bits::type & arg_type =
		static_cast<const jive::bits::type &>(arg->type());
	const jive::operation * op = &arg->node()->operation();

	if ((low() == 0) && (high() == arg_type.nbits())) {
		return jive_unop_reduction_idempotent;
	}
	if (dynamic_cast<const slice_operation *>(op)) {
		return jive_unop_reduction_narrow;
	}
	if (dynamic_cast<const constant_operation *>(op)) {
		return jive_unop_reduction_constant;
	}
	if (dynamic_cast<const concat_operation *>(op)) {
		return jive_unop_reduction_distribute;
	}
	
	return jive_unop_reduction_none;
}

jive::output *
slice_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	const jive::operation * gen_op = &arg->node()->operation();
	
	if (path == jive_unop_reduction_idempotent) {
		return arg;
	}
	
	if (path == jive_unop_reduction_narrow) {
		auto op = static_cast<const slice_operation *>(gen_op);
		return jive_bitslice(arg->node()->inputs[0]->origin(), low() + op->low(), high() + op->low());
	}
	
	if (path == jive_unop_reduction_constant) {
		auto op = static_cast<const constant_operation *>(gen_op);
		return jive_bitconstant(arg->node()->graph, high() - low(), &op->bits[0] + low());
	}
	
	if (path == jive_unop_reduction_distribute) {
		jive_node * node = arg->node();
		jive::output * arguments[node->ninputs];
		
		size_t narguments = 0, pos = 0, n;
		for (n=0; n<node->noperands; n++) {
			jive::output * argument = node->inputs[n]->origin();
			size_t base = pos;
			size_t nbits = static_cast<const jive::bits::output &>(
				*argument).nbits();
			pos = pos + nbits;
			if (base < high() && pos > low()) {
				size_t slice_low = (low() > base) ? (low() - base) : 0;
				size_t slice_high = (high() < pos) ? (high() - base) : (pos - base);
				argument = jive_bitslice(argument, slice_low, slice_high);
				arguments[narguments++] = argument;
			}
		}
		
		return jive_bitconcat(narguments, arguments);
	}
	
	return nullptr;
}

}
}

jive::output *
jive_bitslice(jive::output * argument, size_t low, size_t high)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(argument->type());
	jive::bits::slice_operation op(type, low, high);

	return jive_unary_operation_create_normalized(&JIVE_BITSLICE_NODE, argument->node()->graph,
		&op, argument);
}
