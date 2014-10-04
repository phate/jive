/*
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_PHI_H
#define JIVE_VSDG_PHI_H

#include <jive/vsdg/anchor.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

/* phi node */

extern const jive_anchor_node_class JIVE_PHI_NODE;
extern const jive_node_class JIVE_PHI_ENTER_NODE;
extern const jive_node_class JIVE_PHI_LEAVE_NODE;

namespace jive {

class phi_head_op final : public region_head_op {
public:
	virtual
	~phi_head_op() noexcept;

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

class phi_tail_op final : public region_tail_op {
public:
	virtual
	~phi_tail_op() noexcept;

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

class phi_op final : public region_anchor_op {
public:
	virtual
	~phi_op() noexcept;

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

typedef jive::operation_node<jive::phi_op> jive_phi_node;

JIVE_EXPORTED_INLINE struct jive_node *
jive_phi_node_get_enter_node(const jive_phi_node * self)
{
	return self->producer(0)->region->top;
}

JIVE_EXPORTED_INLINE struct jive_node *
jive_phi_node_get_leave_node(const jive_phi_node * self)
{
	return self->producer(0);
}

JIVE_EXPORTED_INLINE struct jive_region *
jive_phi_node_get_region(const jive_phi_node * self)
{
	return jive_phi_node_get_leave_node(self)->region;
}

JIVE_EXPORTED_INLINE struct jive_region *
jive_phi_region_cast(struct jive_region * region)
{
	if (region->graph->root_region == region)
		return NULL;
	if (region->bottom == NULL)
		return NULL;

	JIVE_DEBUG_ASSERT(region->bottom->outputs[0]->single_user());
	if (jive_node_isinstance(region->bottom->outputs[0]->users.first->node, &JIVE_PHI_NODE))
		return region;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_region *
jive_phi_region_const_cast(const struct jive_region * region)
{
	if (region->graph->root_region == region)
		return NULL;
	if (region->bottom == NULL)
		return NULL;

	JIVE_DEBUG_ASSERT(region->bottom->outputs[0]->single_user());
	if (jive_node_isinstance(region->bottom->outputs[0]->users.first->node, &JIVE_PHI_NODE))
		return region;
	else
		return NULL;
}

typedef struct jive_phi jive_phi;
typedef struct jive_phi_fixvar jive_phi_fixvar;

struct jive_phi_build_state;

/**
	\brief Represent a phi construct under construction
*/
struct jive_phi {
	struct jive_region * region;
	struct jive_phi_build_state * internal_state;
};

/**
	\brief Represent information about a phi fixpoint value
*/
struct jive_phi_fixvar {
	jive::output * value;
	jive::gate * gate;
};

/**
	\brief Begin constructing a phi region
*/
jive_phi
jive_phi_begin(struct jive_graph * graph);

/**
	\brief Add a fixpoint variable of given type
*/
jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive::base::type * type);

/**
	\brief Set fixpoint value of variable
*/
void
jive_phi_fixvar_leave(jive_phi self, jive::gate * var, jive::output * post_value);

/**
	\brief End constructing a phi region
*/
struct jive_node *
jive_phi_end(jive_phi self,
	     size_t npost_values, jive_phi_fixvar * fix_values);

/**
	\brief Represent a phi construct under extension
*/
typedef struct jive_phi_extension jive_phi_extension;
struct jive_phi_extension {
	jive_phi_node * phi_node;
	std::vector<jive::output*> fixvars;
};

/**
	\brief Begin extending a phi construct
*/
struct jive_phi_extension *
jive_phi_begin_extension(jive_phi_node * phi_node, size_t nfixvars,
	const jive::base::type * fixvar_types[]);

/**
	\brief End extending a phi construct
*/
jive::output **
jive_phi_end_extension(struct jive_phi_extension * self);

#endif
