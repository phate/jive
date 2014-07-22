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
};

class lambda_op final : public region_anchor_op {
public:
	virtual
	~lambda_op() noexcept;

	lambda_op(
		const lambda_op & other);

	lambda_op(
		lambda_op && other);

	lambda_op(
		const jive::fct::type & function_type,
		const std::vector<jive::gate *> & argument_gates,
		const std::vector<jive::gate *> & return_gates);

	lambda_op(
		jive::fct::type && function_type,
		std::vector<jive::gate *> && argument_gates,
		std::vector<jive::gate *> && return_gates) noexcept;

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

	inline const std::vector<jive::gate *>
	argument_gates() const noexcept
	{
		return argument_gates_;
	}

	inline const std::vector<jive::gate *>
	return_gates() const noexcept
	{
		return return_gates_;
	}

private:
	jive::fct::type function_type_;
	std::vector<jive::gate *> argument_gates_;
	std::vector<jive::gate *> return_gates_;
};

}
}

typedef jive::operation_node<jive::fct::lambda_op> jive_lambda_node;

extern const jive_node_class JIVE_LAMBDA_NODE;
extern const jive_node_class JIVE_LAMBDA_ENTER_NODE;
extern const jive_node_class JIVE_LAMBDA_LEAVE_NODE;

JIVE_EXPORTED_INLINE jive_lambda_node *
jive_lambda_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_LAMBDA_NODE))
		return (jive_lambda_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const jive_lambda_node *
jive_lambda_node_const_cast(const jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_LAMBDA_NODE))
		return (const jive_lambda_node *)node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_enter_node(const jive_lambda_node * self)
{
	return self->producer(0)->region->top;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_leave_node(const jive_lambda_node * self)
{
	return self->producer(0);
}

JIVE_EXPORTED_INLINE struct jive_region *
jive_lambda_node_get_region(const jive_lambda_node * self)
{
	return jive_lambda_node_get_leave_node(self)->region;
}

bool
jive_lambda_is_self_recursive(const jive_lambda_node * self);

void
jive_inline_lambda_apply(jive_node * apply_node);

bool
jive_lambda_node_remove_dead_parameters(const jive_lambda_node * self);

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
