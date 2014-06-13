/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNUNIFY_H
#define JIVE_TYPES_UNION_UNNUNIFY_H

#include <jive/types/union/unntype.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/operators/unary.h>

/* unify node */

extern const jive_node_class JIVE_UNIFY_NODE;

namespace jive {
namespace unn {

struct declaration;

class unify_operation final : public jive::unary_operation {
public:
	inline
	unify_operation(
		const jive::unn::type & type,
		size_t option) noexcept
		: type_(type)
		, option_(option)
	{
	}

	inline
	unify_operation(const unify_operation & other) noexcept = default;

	inline
	unify_operation(unify_operation && other) noexcept = default;

	virtual
	~unify_operation() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	/* type signature methods */
	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	/* reduction methods */
	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t
	option() const noexcept { return option_; }
	
	inline const jive::unn::declaration *
	declaration() const noexcept { return type_.declaration(); }

private:
	type type_;
	size_t option_;
};

class empty_unify_operation final : public nullary_operation {
public:
	inline constexpr
	empty_unify_operation(
		const jive::unn::declaration * declaration) noexcept
		 : declaration_(declaration)
	{}

	inline const jive::unn::declaration *
	declaration() const noexcept { return declaration_; }

private:
	const jive::unn::declaration * declaration_;
};

}
}

typedef jive::operation_node<jive::unn::unify_operation> jive_unify_node;
typedef jive::operation_node<jive::unn::empty_unify_operation> jive_empty_unify_node;
jive::output *
jive_unify_create(const struct jive::unn::declaration * decl,
	size_t option, jive::output * const operand);

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

jive::output *
jive_empty_unify_create(struct jive_graph * graph, const struct jive::unn::declaration * decl);

JIVE_EXPORTED_INLINE jive_empty_unify_node *
jive_empty_unify_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_EMPTY_UNIFY_NODE))
		return (jive_empty_unify_node *) node;
	else
		return NULL;
}

#endif
