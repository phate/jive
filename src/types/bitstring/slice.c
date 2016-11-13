/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/slice.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>

#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/bitstring/concat.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

slice_op::~slice_op() noexcept
{
}

bool
slice_op::operator==(const operation & other) const noexcept
{
	const slice_op * op = dynamic_cast<const slice_op *>(&other);
	return op && op->argument_type_ == argument_type_ && op->low() == low() && op->high() == high();
}
std::string
slice_op::debug_string() const
{
	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SLICE[%zd:%zd)", low(), high());
	return tmp;
}

const jive::base::type &
slice_op::argument_type(size_t index) const noexcept
{
	return argument_type_;
}

const jive::base::type &
slice_op::result_type(size_t index) const noexcept
{
	return result_type_;
}

jive_unop_reduction_path_t
slice_op::can_reduce_operand(const jive::oport * arg) const noexcept
{
	auto tmp = dynamic_cast<const jive::output*>(arg);
	if (!tmp)
		return jive_unop_reduction_none;

	/*
		FIXME: this should be a dynamic_cast
	*/
	const jive::bits::type & arg_type = static_cast<const jive::bits::type &>(arg->type());
	const jive::operation * op = &tmp->node()->operation();

	if ((low() == 0) && (high() == arg_type.nbits())) {
		return jive_unop_reduction_idempotent;
	}
	if (dynamic_cast<const slice_op *>(op)) {
		return jive_unop_reduction_narrow;
	}
	if (dynamic_cast<const constant_op *>(op)) {
		return jive_unop_reduction_constant;
	}
	if (dynamic_cast<const concat_op *>(op)) {
		return jive_unop_reduction_distribute;
	}
	
	return jive_unop_reduction_none;
}

jive::oport *
slice_op::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::oport * arg) const
{
	auto tmp = dynamic_cast<jive::output*>(arg);
	const jive::operation & gen_op = tmp->node()->operation();
	
	if (path == jive_unop_reduction_idempotent) {
		return arg;
	}
	
	if (path == jive_unop_reduction_narrow) {
		auto op = static_cast<const slice_op &>(gen_op);
		tmp = dynamic_cast<jive::output*>(tmp->node()->input(0)->origin());
		return jive_bitslice(tmp, low() + op.low(), high() + op.low());
	}
	
	if (path == jive_unop_reduction_constant) {
		auto op = static_cast<const constant_op &>(gen_op);
		return jive_bitconstant(arg->region(), high() - low(), &op.value()[0] + low());
	}
	
	if (path == jive_unop_reduction_distribute) {
		jive_node * node = tmp->node();
		std::vector<jive::oport*> arguments;
		
		size_t pos = 0, n;
		for (n = 0; n < node->noperands(); n++) {
			jive::oport * argument = node->input(n)->origin();
			size_t base = pos;
			size_t nbits = static_cast<const jive::bits::type&>(argument->type()).nbits();
			pos = pos + nbits;
			if (base < high() && pos > low()) {
				size_t slice_low = (low() > base) ? (low() - base) : 0;
				size_t slice_high = (high() < pos) ? (high() - base) : (pos - base);
				argument = jive_bitslice(argument, slice_low, slice_high);
				arguments.push_back(argument);
			}
		}
		
		return jive_bitconcat(arguments);
	}
	
	return nullptr;
}

std::unique_ptr<jive::operation>
slice_op::copy() const
{
	return std::unique_ptr<jive::operation>(new slice_op(*this));
}

}
}

jive::oport *
jive_bitslice(jive::oport * argument, size_t low, size_t high)
{
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(argument->type());
	jive::bits::slice_op op(type, low, high);
	return jive_node_create_normalized(argument->region(), op, {argument})[0];
}
