/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/sizeof.h>

#include <jive/arch/memlayout.h>
#include <jive/types/bitstring.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {

sizeof_op::~sizeof_op() noexcept
{
}

bool
sizeof_op::operator==(const operation & other) const noexcept
{
	const sizeof_op * op =
		dynamic_cast<const sizeof_op *>(&other);
	return op && op->type() == type();
}

const jive::base::type &
sizeof_op::result_type(size_t index) const noexcept
{
	/* FIXME: either need a "universal" integer type,
	or some way to specify the representation type for the
	sizeof operator */
	static const jive::bits::type type(32);
	return type;
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
jive_sizeof_create(jive_region * region, const jive::value::type * type)
{
	jive::sizeof_op op(*type);
	return jive_node_create_normalized(region->graph, op, {})[0];
}

/* sizeof reduce */

void
jive_sizeof_node_reduce(const jive_node * node, jive::memlayout_mapper * mapper)
{
	const jive_dataitem_memlayout * layout = jive_memlayout_mapper_map_value_type(mapper,
		&static_cast<const jive::sizeof_op &>(node->operation()).type());
	
	jive::output * new_node = jive_bitconstant_unsigned(node->graph, 32, layout->total_size);
	node->outputs[0]->replace(new_node);
}
