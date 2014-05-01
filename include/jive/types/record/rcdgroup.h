/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_RECORD_RCDGROUP_H
#define JIVE_TYPES_RECORD_RCDGROUP_H

#include <jive/types/record/rcdtype.h>
#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_GROUP_NODE;

namespace jive {
namespace rcd {

class group_operation final : public jive::operation {
public:
	inline constexpr
	group_operation(const jive_record_declaration * declaration) noexcept
		: declaration_(declaration)
	{}

	inline const jive_record_declaration *
	declaration() const noexcept { return declaration_; }

private:
	const jive_record_declaration * declaration_;
};

}
}

typedef jive::operation_node<jive::rcd::group_operation> jive_group_node;

jive_output *
jive_group_create(const jive_record_declaration * decl,
	size_t narguments, jive_output * const * arguments);

jive_output *
jive_empty_group_create(struct jive_graph * graph, const jive_record_declaration * decl);

JIVE_EXPORTED_INLINE jive_group_node *
jive_group_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_GROUP_NODE)
		return (jive_group_node *) node;
	else
		return 0;
}

#endif
