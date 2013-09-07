/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTLAMBDA_H
#define JIVE_TYPES_FUNCTION_FCTLAMBDA_H

#include <jive/vsdg/node.h>
#include <jive/types/function/fcttype.h>

typedef struct jive_lambda jive_lambda;
typedef struct jive_lambda_node jive_lambda_node;
typedef struct jive_lambda_node_attrs jive_lambda_node_attrs;

extern const jive_node_class JIVE_LAMBDA_NODE;
extern const jive_node_class JIVE_LAMBDA_ENTER_NODE;
extern const jive_node_class JIVE_LAMBDA_LEAVE_NODE;

struct jive_lambda_node_attrs {
	jive_node_attrs base;
	jive_function_type function_type;
	jive_gate ** argument_gates;
	jive_gate ** return_gates;
};

struct jive_lambda_node {
	jive_node base;
	jive_lambda_node_attrs attrs;
};

JIVE_EXPORTED_INLINE jive_lambda_node *
jive_lambda_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_LAMBDA_NODE))
		return (jive_lambda_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_enter_node(const jive_lambda_node * self)
{
	return self->base.inputs[0]->origin->node->region->top;
}

JIVE_EXPORTED_INLINE jive_node *
jive_lambda_node_get_leave_node(const jive_lambda_node * self)
{
	return self->base.inputs[0]->origin->node;
}

struct jive_lambda_build_state;

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

void
jive_inline_lambda_apply(jive_node * apply_node);

#endif
