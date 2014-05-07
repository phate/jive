/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNUNIFY_H
#define JIVE_TYPES_UNION_UNNUNIFY_H

#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/operators/unary.h>

struct jive_union_declaration;

/* unify node */

extern const jive_unary_operation_class JIVE_UNIFY_NODE_;
#define JIVE_UNIFY_NODE (JIVE_UNIFY_NODE_.base)

namespace jive {
namespace unn {

class unify_operation final : public unary_operation {
public:
	inline constexpr
	unify_operation(
		const jive_union_declaration * declaration,
		 size_t option) noexcept
		 : declaration_(declaration)
		 , option_(option)
	{}

	inline size_t
	option() const noexcept { return option_; }

	inline const jive_union_declaration *
	declaration() const noexcept { return declaration_; }

private:
	const jive_union_declaration * declaration_;
	size_t option_;
};

class empty_unify_operation final : public nullary_operation {
public:
	inline constexpr
	empty_unify_operation(
		const jive_union_declaration * declaration) noexcept
		 : declaration_(declaration)
	{}

	inline const jive_union_declaration *
	declaration() const noexcept { return declaration_; }

private:
	const jive_union_declaration * declaration_;
};

}
}

typedef jive::operation_node<jive::unn::unify_operation> jive_unify_node;
typedef jive::operation_node<jive::unn::empty_unify_operation> jive_empty_unify_node;
jive_output *
jive_unify_create(const struct jive_union_declaration * decl,
	size_t option, jive_output * const operand);

JIVE_EXPORTED_INLINE jive_unify_node *
jive_unify_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_UNIFY_NODE))
		return (jive_unify_node *) node;
	else
		return NULL;
}

/* empty unify node */

extern const jive_node_class JIVE_EMPTY_UNIFY_NODE;

jive_output *
jive_empty_unify_create(struct jive_graph * graph, const struct jive_union_declaration * decl);

JIVE_EXPORTED_INLINE jive_empty_unify_node *
jive_empty_unify_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_EMPTY_UNIFY_NODE))
		return (jive_empty_unify_node *) node;
	else
		return NULL;
}

#endif
