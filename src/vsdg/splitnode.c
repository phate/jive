/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/splitnode.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/valuetype.h>

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

jive_node *
split_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(narguments== 1);
	
	jive_splitnode * node = new jive_splitnode(*this);
	const jive::base::type * in_type = &argument_type(0);
	const jive::base::type * out_type = &result_type(0);
	jive_node_init_(node, region,
		1, &in_type, arguments,
		1, &out_type);
	node->inputs[0]->required_rescls = in_class();
	node->outputs[0]->required_rescls = out_class();
	
	return node;
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
	return *in_class_->type;
}

const jive::base::type &
split_operation::result_type(size_t index) const noexcept
{
	return *out_class_->type;
}

/* reduction methods */
jive_unop_reduction_path_t
split_operation::can_reduce_operand(
	const jive::output * arg) const noexcept
{
	return jive_unop_reduction_none;
}

jive::output *
split_operation::reduce_operand(
	jive_unop_reduction_path_t path,
	jive::output * arg) const
{
	return nullptr;
}

std::unique_ptr<jive::operation>
split_operation::copy() const
{
	return std::unique_ptr<jive::operation>(new split_operation(*this));
}

}

const jive_node_class JIVE_SPLITNODE = {
	parent : &JIVE_NODE,
	name : "SPLIT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive_node *
jive_splitnode_create(jive_region * region,
	const jive::base::type * in_type,
	jive::output * in_origin,
	const struct jive_resource_class * in_class,
	const jive::base::type * out_type,
	const struct jive_resource_class * out_class)
{
	jive::split_operation op(in_class, out_class);
	
	jive::node_normal_form * nf =
		jive_graph_get_nodeclass_form(region->graph, typeid(jive::split_operation));
	if (nf->get_mutable() && nf->get_cse())
		jive_graph_mark_denormalized(region->graph);

	return op.create_node(region, 1, &in_origin);
}
