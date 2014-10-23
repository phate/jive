/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTLAMBDA_H
#define JIVE_TYPES_FUNCTION_FCTLAMBDA_H

#include <memory>
#include <vector>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>

namespace jive {
namespace fct {

class lambda_head_op final : public region_head_op {
public:
	virtual
	~lambda_head_op() noexcept;

	virtual size_t
	nresults() const noexcept override;

	virtual const base::type &
	result_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class lambda_tail_op final : public region_tail_op {
public:
	virtual
	~lambda_tail_op() noexcept;

	virtual size_t
	narguments() const noexcept override;

	virtual const base::type &
	argument_type(size_t index) const noexcept override;

	virtual jive_node *
	create_node(
		jive_region * region,
		size_t narguments,
		jive::output * const arguments[]) const override;

	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

class lambda_op final : public region_anchor_op {
public:
	virtual
	~lambda_op() noexcept;

	inline
	lambda_op(const lambda_op & other) = default;

	inline
	lambda_op(lambda_op && other) = default;

	inline lambda_op(
		jive::fct::type function_type,
		std::vector<std::string> argument_names,
		std::vector<std::string> result_names) noexcept
		: function_type_(std::move(function_type))
		, argument_names_(std::move(argument_names))
		, result_names_(std::move(result_names))
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override;

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

	inline const jive::fct::type &
	function_type() const noexcept
	{
		return function_type_;
	}

	inline const std::vector<std::string> &
	argument_names() const noexcept
	{
		return argument_names_;
	}

	inline const std::vector<std::string> &
	result_names() const noexcept
	{
		return result_names_;
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	jive::fct::type function_type_;
	std::vector<std::string> argument_names_;
	std::vector<std::string> result_names_;
};

}
}

bool
jive_lambda_is_self_recursive(const jive_node * self);

void
jive_inline_lambda_apply(jive_node * apply_node);

bool
jive_lambda_node_remove_dead_parameters(const jive_node * self);

/* lambda instantiation */

/**
	\brief Represent a lambda construct under construction
*/

typedef struct jive_lambda jive_lambda;

struct jive_lambda {
	struct jive_region * region;
	size_t narguments;
	jive::output ** arguments;
	struct jive_lambda_build_state * internal_state;
};

/**
	\brief Begin constructing a lambda region
*/
struct jive_lambda *
jive_lambda_begin(struct jive_graph * graph, size_t narguments,
	const jive::base::type * const argument_types[], const char * const argument_names[]);

/**
	\brief End constructing a lambda region
*/
jive::output *
jive_lambda_end(struct jive_lambda * lambda, size_t nresults,
	const jive::base::type * const result_types[], struct jive::output * const results[]);

#endif
