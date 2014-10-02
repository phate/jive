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

class group_op final : public jive::operation {
public:
	virtual
	~group_op() noexcept;

	inline constexpr
	group_op(const jive::rcd::declaration * declaration) noexcept
		: result_type_(declaration)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	inline const jive::rcd::declaration *
	declaration() const noexcept { return result_type_.declaration(); }

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	const jive::rcd::type result_type_;
};

}
}

typedef jive::operation_node<jive::rcd::group_op> jive_group_node;

jive::output *
jive_group_create(const jive::rcd::declaration * decl,
	size_t narguments, jive::output * const * arguments);

jive::output *
jive_empty_group_create(struct jive_graph * graph, const jive::rcd::declaration * decl);

#endif
