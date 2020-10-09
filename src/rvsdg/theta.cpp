/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/substitution.hpp>
#include <jive/rvsdg/theta.hpp>

namespace jive {

/* theta operation */

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

/* theta input */

theta_input::~theta_input() noexcept
{
	if (output_)
		output_->input_ = nullptr;
}

jive::theta_node *
theta_input::node() const noexcept
{
	return static_cast<theta_node*>(structural_input::node());
}

/* theta output */

theta_output::~theta_output() noexcept
{
	if (input_)
		input_->output_ = nullptr;
}

jive::theta_node *
theta_output::node() const noexcept
{
	return static_cast<theta_node*>(structural_output::node());
}

/* theta node */

theta_node::~theta_node()
{}

const theta_node::loopvar_iterator &
theta_node::loopvar_iterator::operator++() noexcept
{
	if (output_ == nullptr)
		return *this;

	auto node = output_->node();
	auto index = output_->index();
	if (index == node->noutputs()-1) {
		output_ = nullptr;
		return *this;
	}

	index++;
	output_ = node->output(index);
	return *this;
}

jive::theta_output *
theta_node::add_loopvar(jive::output * origin)
{
	node::add_input(std::unique_ptr<jive::input>(
		new theta_input(this, ninputs(), origin, origin->type())));
	node::add_output(std::unique_ptr<jive::output>(
		new theta_output(this, noutputs(), origin->type())));

	auto input = theta_node::input(ninputs()-1);
	auto output = theta_node::output(noutputs()-1);
	input->output_ = output;
	output->input_ = input;

	auto argument = subregion()->add_argument(input, origin->type());
	subregion()->add_result(argument, output, origin->type());
	return output;
}

jive::theta_node *
theta_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	auto nf = graph()->node_normal_form(typeid(jive::operation));
	nf->set_mutable(false);

	jive::substitution_map rmap;
	auto theta = create(region);

	/* add loop variables */
	for (auto olv : *this) {
		auto nlv = theta->add_loopvar(smap.lookup(olv->input()->origin()));
		rmap.insert(olv->argument(), nlv->argument());
	}

	/* copy subregion */
	subregion()->copy(theta->subregion(), rmap, false, false);
	theta->set_predicate(rmap.lookup(predicate()->origin()));

	/* redirect loop variables */
	for (auto olv = begin(), nlv = theta->begin(); olv != end(); olv++, nlv++) {
		(*nlv)->result()->divert_to(rmap.lookup((*olv)->result()->origin()));
		smap.insert(olv.output(), nlv.output());
	}

	nf->set_mutable(true);
	return theta;
}

}
