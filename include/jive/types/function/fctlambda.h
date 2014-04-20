/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTLAMBDA_H
#define JIVE_TYPES_FUNCTION_FCTLAMBDA_H

#include <memory>
#include <vector>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/node.h>

/* lambda node */

typedef struct jive_lambda jive_lambda;

extern const jive_node_class JIVE_LAMBDA_NODE;
extern const jive_node_class JIVE_LAMBDA_ENTER_NODE;
extern const jive_node_class JIVE_LAMBDA_LEAVE_NODE;

class jive_op_lambda final : public jive::operation {
public:
	virtual ~jive_op_lambda() noexcept;

	jive_op_lambda(
		const jive_op_lambda & other);

	jive_op_lambda(
		jive_op_lambda && other);

	jive_op_lambda(
		const jive_function_type & function_type,
		const std::vector<jive_gate *> & argument_gates,
		const std::vector<jive_gate *> & return_gates);

	jive_op_lambda(
		jive_function_type && function_type,
		std::vector<jive_gate *> && argument_gates,
		std::vector<jive_gate *> && return_gates) noexcept;

	inline const jive_function_type &
	function_type() const noexcept
	{
		return function_type_;
	}

	inline const std::vector<jive_gate *>
	argument_gates() const noexcept
	{
		return argument_gates_;
	}

	inline const std::vector<jive_gate *>
	return_gates() const noexcept
	{
		return return_gates_;
	}

private:
	jive_function_type function_type_;
	std::vector<jive_gate *> argument_gates_;
	std::vector<jive_gate *> return_gates_;
};

typedef jive::operation_node<jive_op_lambda> jive_lambda_node;

class jive_op_lambda_enter final : public jive::operation {
};

class jive_op_lambda_leave final : public jive::operation {
};

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
	return self->inputs[0]->origin->node->region->top;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_leave_node(const jive_lambda_node * self)
{
	return self->inputs[0]->origin->node;
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
	struct jive_output ** arguments;
	struct jive_lambda_build_state * internal_state;
};

/**
	\brief Begin constructing a lambda region
*/
struct jive_lambda *
jive_lambda_begin(struct jive_graph * graph,
	size_t narguments, const jive_type * const argument_types[], const char * const argument_names[]);

/**
	\brief End constructing a lambda region
*/
struct jive_output *
jive_lambda_end(struct jive_lambda * lambda,
	size_t nresults, const jive_type * const result_types[], struct jive_output * const results[]);

#endif
