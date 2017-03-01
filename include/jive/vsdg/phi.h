/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_PHI_H
#define JIVE_VSDG_PHI_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/structural.h>
#include <jive/vsdg/structural_node.h>

/* phi node */

namespace jive {

class phi_op final : public structural_op {
public:
	virtual
	~phi_op() noexcept;
	virtual std::string
	debug_string() const override;

	virtual std::unique_ptr<jive::operation>
	copy() const override;
};

}

JIVE_EXPORTED_INLINE jive::region *
jive_phi_region_cast(jive::region * region)
{
	/*
		FIXME: remove this function
	*/
	if (region->node() && typeid(region->node()->operation()) == typeid(jive::phi_op))
		return region;

	return nullptr;
}

JIVE_EXPORTED_INLINE const jive::region *
jive_phi_region_const_cast(const jive::region * region)
{
	/*
		FIXME: remove this function
	*/
	if (region->node() && typeid(region->node()->operation()) == typeid(jive::phi_op))
		return region;

	return nullptr;
}

typedef struct jive_phi jive_phi;
typedef struct jive_phi_fixvar jive_phi_fixvar;

struct jive_phi_build_state;

/**
	\brief Represent a phi construct under construction
*/
struct jive_phi {
	struct jive::region * region;
	struct jive_phi_build_state * internal_state;
};

/**
	\brief Represent information about a phi fixpoint value
*/
struct jive_phi_fixvar {
	jive::oport * value;
	jive::gate * gate;
};

/**
	\brief Begin constructing a phi region
*/
jive_phi
jive_phi_begin(struct jive::region * parent);

/**
	\brief Add a fixpoint variable of given type
*/
jive_phi_fixvar
jive_phi_fixvar_enter(jive_phi self, const struct jive::base::type * type);

/**
	\brief Set fixpoint value of variable
*/
void
jive_phi_fixvar_leave(jive_phi self, jive::gate * var, jive::oport * post_value);

/**
	\brief End constructing a phi region
*/
jive::node *
jive_phi_end(jive_phi self,
	     size_t npost_values, jive_phi_fixvar * fix_values);

#endif
