/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_UNION_H
#define JIVE_TYPES_UNION_H

#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/rvsdg/unary.h>

namespace jive {
namespace unn {

/* union declaration */

struct declaration {
	size_t nelements;
	const jive::valuetype ** elements;
};

/* union type */

class type final : public jive::valuetype {
public:
	virtual
	~type() noexcept;

	inline constexpr
	type(const jive::unn::declaration * decl) noexcept
	: decl_(decl)
	{}

	inline const jive::unn::declaration *
	declaration() const noexcept
	{
		return decl_;
	}

	virtual
	std::string debug_string() const override;

	virtual bool
	operator==(const jive::type & other) const noexcept override;

	virtual std::unique_ptr<jive::type>
	copy() const override;

private:
	const jive::unn::declaration * decl_;
};

/* unify operator */

class unify_op final : public jive::unary_op {
public:
	virtual
	~unify_op() noexcept;

	inline
	unify_op(const jive::unn::type & type, size_t option) noexcept
	: option_(option)
	, result_(type)
	, argument_(*type.declaration()->elements[option])
	{}

	inline
	unify_op(const unify_op & other) = default;

	inline
	unify_op(unify_op && other) = default;

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual  const jive::port &
	result(size_t index) const noexcept override;

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
	declaration() const noexcept
	{
		return static_cast<const jive::unn::type*>(&result_.type())->declaration();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	size_t option_;
	jive::port result_;
	jive::port argument_;
};

}}

jive::output *
jive_unify_create(
	const jive::unn::declaration * decl,
	size_t option,
	jive::output * const operand);

namespace jive {
namespace unn {

/* empty unify operator */

class empty_unify_op final : public base::nullary_op {
public:
	virtual
	~empty_unify_op() noexcept;

	inline
	empty_unify_op(const jive::unn::declaration * declaration) noexcept
	: port_(jive::unn::type(declaration))
	{}

	inline const jive::unn::declaration *
	declaration() const noexcept
	{
		return static_cast<const jive::unn::type*>(&port_.type())->declaration();
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::port port_;
};

}}

jive::output *
jive_empty_unify_create(
	jive::region * region,
	const jive::unn::declaration * decl);

namespace jive {
namespace unn {

/* choose operator */

class choose_op final : public jive::unary_op {
public:
	virtual
	~choose_op() noexcept;

	inline
	choose_op(
		const jive::unn::type & type,
		size_t option) noexcept
	: option_(option)
	, result_(*type.declaration()->elements[option])
	, argument_(type)
	{}

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual std::string
	debug_string() const override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual jive_unop_reduction_path_t
	can_reduce_operand(
		const jive::output * arg) const noexcept override;

	virtual jive::output *
	reduce_operand(
		jive_unop_reduction_path_t path,
		jive::output * arg) const override;

	inline size_t
	option() const noexcept
	{
		return option_;
	}

	inline const jive::unn::declaration *
	declaration() const noexcept
	{
		return static_cast<const jive::unn::type*>(&result_.type())->declaration();
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

	static inline jive::unary_normal_form *
	normal_form(jive::graph * graph)
	{
		return static_cast<jive::unary_normal_form*>(graph->node_normal_form(typeid(choose_op)));
	}

	static inline jive::output *
	create(jive::output * operand, size_t option)
	{
		auto ut = dynamic_cast<const jive::unn::type*>(&operand->type());
		if (!ut) throw jive::type_error("unn", operand->type().debug_string());

		choose_op op(*ut, option);
		return jive::create_normalized(operand->region(), op, {operand})[0];
	}

private:
	size_t option_;
	jive::port result_;
	jive::port argument_;
};

static inline bool
is_choose_op(const jive::operation & op) noexcept
{
	return dynamic_cast<const choose_op*>(&op) != nullptr;
}

static inline bool
is_choose_node(const jive::node * node) noexcept
{
	return is_opnode<choose_op>(node);
}

}}

#endif
