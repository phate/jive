/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_UNNUNIFY_H
#define JIVE_TYPES_UNION_UNNUNIFY_H

#include <jive/types/union/unntype.h>
#include <jive/vsdg/nullary.h>
#include <jive/vsdg/unary.h>

/* unify node */

namespace jive {
namespace unn {

struct declaration;

class unify_op final : public base::unary_op {
public:
	inline
	unify_op(
		const jive::unn::type & type,
		size_t option) noexcept
		: type_(type)
		, option_(option)
	{
	}

	inline
	unify_op(const unify_op & other) noexcept = default;

	inline
	unify_op(unify_op && other) noexcept = default;

	virtual
	~unify_op() noexcept;

	virtual bool
	operator==(const operation & other) const noexcept override;
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

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	type type_;
	size_t option_;
};

class empty_unify_op final : public base::nullary_op {
public:
	virtual
	~empty_unify_op() noexcept;

	inline constexpr
	empty_unify_op(
		const jive::unn::declaration * declaration) noexcept
		 : type_(declaration)
	{}

	inline const jive::unn::declaration *
	declaration() const noexcept { return type_.declaration(); }

	virtual bool
	operator==(const operation & other) const noexcept override;
	virtual std::string
	debug_string() const override;

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	type type_;
};

}
}

jive::output *
jive_unify_create(
	const jive::unn::declaration * decl,
	size_t option,
	jive::output * const operand);

/* empty unify node */

jive::output *
jive_empty_unify_create(
	jive::region * region,
	const jive::unn::declaration * decl);

#endif
