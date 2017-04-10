/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma.h>

#include <algorithm>
#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/util/strfmt.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma-normal-form.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/traverser.h>

namespace jive {

gamma_op::~gamma_op() noexcept
{
}

size_t
gamma_op::narguments() const noexcept
{
	return 1;
}

const base::type &
gamma_op::argument_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return predicate_type_;
}

std::string
gamma_op::debug_string() const
{
	return "GAMMA";
}

std::unique_ptr<jive::operation>
gamma_op::copy() const
{
	return std::unique_ptr<jive::operation>(new gamma_op(*this));
}

bool
gamma_op::operator==(const operation & other) const noexcept
{
	const gamma_op * op = dynamic_cast<const gamma_op*>(&other);
	return op && op->predicate_type_ == predicate_type_;
}

}

jive::node_normal_form *
jive_gamma_node_get_default_normal_form_(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	jive::gamma_normal_form * normal_form = new jive::gamma_normal_form(
		operator_class, parent, graph);
	return normal_form;
}

static void  __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(
		typeid(jive::gamma_op), jive_gamma_node_get_default_normal_form_);
}
