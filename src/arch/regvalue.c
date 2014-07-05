/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/regvalue.h>

#include <string.h>

#include <jive/types/bitstring/type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/region.h>
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
	static const jive::ctl::type ctl;
	if (index == 0) {
		return ctl;
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

jive_node *
regvalue_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_regvalue_node * node = new jive_regvalue_node(*this);
	node->class_ = &JIVE_REGVALUE_NODE;

	const jive::base::type * argument_types[2] = {
		&argument_type(0),
		&argument_type(1)
	};
	const jive::base::type * result_types[1] = {
		&result_type(0)
	};

	jive_node_init_(node, region,
		2, argument_types, arguments,
		1, result_types);

	return node;
}

std::string
regvalue_op::debug_string() const
{
	return regcls()->base.name;
}

}

const jive_node_class JIVE_REGVALUE_NODE = {
	parent : &JIVE_NODE,
	name : "REGVALUE_NODE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_regvalue(jive::output * ctl, const jive_register_class * regcls, jive::output * value)
{
	jive::regvalue_op op(regcls);
	
	jive::output * arguments[] = {ctl, value};
	jive_region * region = jive_region_innermost(2, arguments);
	
	const jive_node_normal_form * nf =
		jive_graph_get_nodeclass_form(region->graph, &JIVE_REGVALUE_NODE);
	jive_node * node = jive_node_cse_create(nf, region, &op, 2, arguments);
	return node->outputs[0];
}
