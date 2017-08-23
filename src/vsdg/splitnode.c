/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/splitnode.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/simple_node.h>

#include <string.h>

namespace jive {

split_operation::~split_operation() noexcept
{
}

bool
split_operation::operator==(const operation & gen_other) const noexcept
{
	/* treat this operation a bit specially: state that any two
	 * splits are not the same to unconditionally make them exempt
	 * from CSE */
	return false;
}
std::string
split_operation::debug_string() const
{
	return "SPLIT";
}

/* type signature methods */
const jive::base::type &
split_operation::argument_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_.type();
}

const jive::port &
split_operation::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return argument_;
}

const jive::base::type &
split_operation::result_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_.type();
}

const jive::port &
split_operation::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
}

jive_unop_reduction_path_t
split_operation::can_reduce_operand(const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
split_operation::reduce_operand(jive_unop_reduction_path_t path, jive::output * arg) const
{
	return nullptr;
}

std::unique_ptr<jive::operation>
split_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new split_operation(*this));
}

}

jive::node *
jive_splitnode_create(jive::region * region,
	jive::output * in_origin,
	const struct jive::resource_class * in_class,
	const struct jive::resource_class * out_class)
{
	jive::split_operation op(in_class, out_class);
	
	auto nf = region->graph()->node_normal_form(typeid(jive::split_operation));
	if (nf->get_mutable())
		region->graph()->mark_denormalized();

	return region->add_simple_node(op, {in_origin});
}
