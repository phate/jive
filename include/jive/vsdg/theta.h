/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_THETA_H
#define JIVE_VSDG_THETA_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>

struct jive_gate;
struct jive_graph;
struct jive_node;
struct jive_output;
struct jive_region;
struct jive_theta_build_state;
struct jive_type;

extern const jive_node_class JIVE_THETA_HEAD_NODE;
extern const jive_node_class JIVE_THETA_TAIL_NODE;
extern const jive_node_class JIVE_THETA_NODE;

class jive_op_theta_head final : public jive::operation {
};

class jive_op_theta_tail final : public jive::operation {
};

class jive_op_theta final : public jive::operation {
};

typedef struct jive_theta jive_theta;
typedef struct jive_theta_loopvar jive_theta_loopvar;

/**
	\brief Represent a theta construct under construction
*/
struct jive_theta {
	struct jive_region * region;
	struct jive_theta_build_state * internal_state;
};

/**
	\brief Represent information about a loop-variant value
*/
struct jive_theta_loopvar {
	struct jive_output * value;
	struct jive_gate * gate;
};

/**
	\brief Begin constructing a loop region
*/
jive_theta
jive_theta_begin(struct jive_graph * graph);

/**
	\brief Add a loop-variant variable with a pre-loop value
*/
jive_theta_loopvar
jive_theta_loopvar_enter(jive_theta self, struct jive_output * pre_value);

/**
	\brief Set post-iteration value of a loop-variant variable
*/
void
jive_theta_loopvar_leave(jive_theta self, jive_gate * var,
	struct jive_output * post_value);

/**
	\brief End constructing a loop region, specify repetition predicate
*/
struct jive_node *
jive_theta_end(jive_theta self, struct jive_output * predicate,
	size_t npost_values, jive_theta_loopvar * post_values);

#endif
