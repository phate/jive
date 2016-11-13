/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/regvalue.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/seqtype.h>

namespace jive {

regvalue_op::~regvalue_op() noexcept
{
}

bool
regvalue_op::operator==(const operation & other) const noexcept
{
	const regvalue_op * op =
		dynamic_cast<const regvalue_op *>(&other);
	return op && op->regcls() == regcls();
}

size_t
regvalue_op::narguments() const noexcept
{
	return 2;
}

const jive::base::type &
regvalue_op::argument_type(size_t index) const noexcept
{
	if (index == 0) {
		return seq::seqtype;
	} else {
		return *regcls()->base.type;
	}
}

size_t
regvalue_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
regvalue_op::result_type(size_t index) const noexcept
{
	return *regcls()->base.type;
}
std::string
regvalue_op::debug_string() const
{
	return regcls()->base.name;
}

std::unique_ptr<jive::operation>
regvalue_op::copy() const
{
	return std::unique_ptr<jive::operation>(new regvalue_op(*this));
}

}

jive::oport *
jive_regvalue(jive::oport * ctl, const jive_register_class * regcls, jive::oport * value)
{
	jive_graph * graph = value->region()->graph();
	jive::regvalue_op op(regcls);
	
	const auto nf = graph->node_normal_form(typeid(jive::regvalue_op));

	jive::region * region = ctl->region();
	return nf->normalized_create(region, op, {ctl, value})[0];
}
