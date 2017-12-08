/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>

#include <stdio.h>
#include <string.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/phi.h>
#include <jive/rvsdg/substitution.h>

/* lambda node */

namespace jive {
namespace fct {

lambda_op::~lambda_op() noexcept
{
}

bool
lambda_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const lambda_op*>(&other);
	return op && op->function_type() == function_type();
}

size_t
lambda_op::narguments() const noexcept
{
	return 0;
}

size_t
lambda_op::nresults() const noexcept
{
	return 1;
}

std::string
lambda_op::debug_string() const
{
	return "LAMBDA";
}

std::unique_ptr<jive::operation>
lambda_op::copy() const
{
	return std::unique_ptr<jive::operation>(new lambda_op(*this));
}

}

/* lambda node class */

lambda_node::~lambda_node()
{}

jive::lambda_node *
lambda_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	jive::lambda_builder lb;
	auto arguments = lb.begin_lambda(region, function_type());

	/* add dependencies */
	jive::substitution_map rmap;
	for (size_t n = 0; n < function_type().narguments(); n++)
		rmap.insert(subregion()->argument(n), arguments[n]);
	for (const auto & odv : *this) {
		auto ndv = lb.add_dependency(smap.lookup(odv->origin()));
		rmap.insert(dynamic_cast<structural_input*>(odv)->arguments.first, ndv);
	}

	/* copy subregion */
	subregion()->copy(lb.subregion(), rmap, false, false);

	/* collect results */
	std::vector<jive::output*> results;
	for (size_t n = 0; n < subregion()->nresults(); n++)
		results.push_back(rmap.lookup(subregion()->result(n)->origin()));

	auto lambda = lb.end_lambda(results);
	smap.insert(output(0), lambda->output(0));
	return lambda;
}

}

bool
jive_lambda_is_self_recursive(const jive::node * self)
{
	JIVE_DEBUG_ASSERT(self->noutputs() == 1);

	auto lambda = dynamic_cast<const jive::structural_node*>(self);

	auto phi_region = self->region();
	auto phi = phi_region->node();
	if (phi && typeid(phi->operation()) != typeid(jive::phi_op))
		return false;

	/* find index of lambda output in the phi leave node */
	size_t index = phi_region->nresults();
	for (const auto & user : *self->output(0)) {
		if (dynamic_cast<const jive::result*>(user)) {
			index = user->index();
			break;
		}
	}
	if (index == phi_region->nresults())
		return false;

	/* the lambda is self-recursive if one of its external dependencies originates from the same
	*  index in the phi enter node
	*/
	for (size_t n = 0; n < lambda->ninputs(); n++) {
		if (lambda->input(n)->origin()->index() == index)
			return true;
	}

	return false;
}
