/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/substitution.h>
#include <jive/rvsdg/theta.h>

namespace jive {

theta_op::~theta_op() noexcept
{
}
std::string
theta_op::debug_string() const
{
	return "THETA";
}

std::unique_ptr<jive::operation>
theta_op::copy() const
{
	return std::unique_ptr<jive::operation>(new theta_op(*this));
}

theta_node::~theta_node()
{}

jive::theta_node *
theta_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	auto nf = graph()->node_normal_form(typeid(jive::operation));
	nf->set_mutable(false);

	jive::substitution_map rmap;
	auto theta = create(region);

	/* add loop variables */
	for (auto olv : *this) {
		auto nlv = theta->add_loopvar(smap.lookup(olv.input()->origin()));
		rmap.insert(olv.argument(), nlv->argument());
	}

	/* copy subregion */
	subregion()->copy(theta->subregion(), rmap, false, false);
	theta->set_predicate(rmap.lookup(predicate()->origin()));

	/* redirect loop variables */
	for (auto olv = begin(), nlv = theta->begin(); olv != end(); olv++, nlv++) {
		nlv->result()->divert_origin(rmap.lookup(olv->result()->origin()));
		smap.insert(olv->output(), nlv->output());
	}

	nf->set_mutable(true);
	return theta;
}

}
