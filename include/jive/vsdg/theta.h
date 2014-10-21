/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_THETA_H
#define JIVE_VSDG_THETA_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/node.h>

namespace jive {
class gate;
}

struct jive_graph;
struct jive_node;
struct jive_region;
struct jive_theta_build_state;

namespace jive {

class theta_head_op final : public region_head_op {
public:
	virtual
	~theta_head_op() noexcept;

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

class theta_tail_op final : public region_tail_op {
public:
	virtual
	~theta_tail_op() noexcept;

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

class theta_op final : public region_anchor_op {
public:
	virtual
	~theta_op() noexcept;

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

}

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
	jive::output * value;
	jive::gate * gate;
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
jive_theta_loopvar_enter(jive_theta self, jive::output * pre_value);

/**
	\brief Set post-iteration value of a loop-variant variable
*/
void
jive_theta_loopvar_leave(jive_theta self, jive::gate * var,
	jive::output * post_value);

/**
	\brief End constructing a loop region, specify repetition predicate
*/
struct jive_node *
jive_theta_end(jive_theta self, jive::output * predicate,
	size_t npost_values, jive_theta_loopvar * post_values);

#endif
