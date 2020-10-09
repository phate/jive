/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/sizeof.hpp>

#include <jive/arch/memlayout.hpp>
#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/region.hpp>
#include <jive/types/bitstring.hpp>

namespace jive {

sizeof_op::~sizeof_op() noexcept
{}

bool
sizeof_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const sizeof_op*>(&other);
	return op && op->type() == type();
}

std::string
sizeof_op::debug_string() const
{
	return "SIZEOF";
}

std::unique_ptr<jive::operation>
sizeof_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sizeof_op(*this));
}

}

jive::output *
jive_sizeof_create(jive::region * region, const jive::valuetype * type)
{
	jive::sizeof_op op(*type);
	return jive::simple_node::create_normalized(region, op, {})[0];
}

/* sizeof reduce */

void
jive_sizeof_node_reduce(const jive::node * node, jive::memlayout_mapper * mapper)
{
	const jive::dataitem_memlayout & layout = mapper->map_value_type(
		static_cast<const jive::sizeof_op &>(node->operation()).type());
	
	auto new_node = create_bitconstant(node->region(), 32, layout.size());
	node->output(0)->divert_users(new_node);
}
