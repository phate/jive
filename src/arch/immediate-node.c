/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-node.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <jive/arch/immediate-type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/region.h>

/* immediate node */

namespace jive {

immediate_operation::~immediate_operation() noexcept {}

}

static void
jive_immediate_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_immediate_node * self = (const jive_immediate_node *) self_;
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "%" "lld", self->operation().value().offset);
	jive_buffer_putstr(buffer, tmp);
}

static bool
jive_immediate_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive::immediate_operation * first = &((const jive_immediate_node *) self)->operation();
	const jive::immediate_operation * second = (const jive::immediate_operation *) attrs;
	
	return jive_immediate_equals(&first->value(), &second->value());
}

static jive_node *
jive_immediate_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive::output * const operands[])
{
	const jive::immediate_operation * attrs = (const jive::immediate_operation *) attrs_;
	
	jive_immediate_node * self = new jive_immediate_node(*attrs);
	jive::imm::type immediate_type;
	const jive::base::type* tmparray0[] = {&immediate_type};
	jive_node_init_(self,
		region,
		0, 0, 0,
		1, tmparray0);
	self->class_ = &JIVE_IMMEDIATE_NODE;
	
	return self;
}

const jive_node_class JIVE_IMMEDIATE_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "IMMEDIATE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_nullary_operation_get_default_normal_form_, /* inherit */
	get_label : jive_immediate_node_get_label_, /* override */
	match_attrs : jive_immediate_node_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : jive_immediate_node_create_, /* override */
};

jive::output *
jive_immediate_create(
	jive_graph * graph,
	const jive_immediate * immediate_value)
{
	jive::immediate_operation op(*immediate_value);

	return jive_nullary_operation_create_normalized(&JIVE_IMMEDIATE_NODE, graph, &op);
}
