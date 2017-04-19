/*
 * Copyright 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gate-interference-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/structural_node.h>
#include <jive/vsdg/substitution.h>

namespace jive {

/* structural input */

structural_input::~structural_input()
{
	JIVE_DEBUG_ASSERT(arguments.first == nullptr && arguments.last == nullptr);

	node()->graph()->on_iport_destroy(this);

	if (gate()) {
		for (size_t n = 0; n < node()->ninputs(); n++) {
			auto other = node()->input(n);
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(node()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < node()->ninputs(); n++)
		static_cast<jive::structural_node*>(node())->input(n)->set_index(n-1);
}

structural_input::structural_input(
	jive::structural_node * node,
	size_t index,
	jive::oport * origin,
	const jive::base::type & type)
	: iport(node->region(), type, index, origin)
	, node_(node)
	, type_(type.copy())
{
	arguments.first = arguments.last = nullptr;
	node->graph()->on_iport_create(this);
}

structural_input::structural_input(
	jive::structural_node * node,
	size_t index,
	jive::oport * origin,
	jive::gate * gate)
	: iport(node->region(), gate->type(), index, origin, gate)
	, node_(node)
	, type_(gate->type().copy())
{
	arguments.first = arguments.last = nullptr;

	for (size_t n = 0; n < index; n++) {
		jive::structural_input * other = node->input(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}

	node->graph()->on_iport_create(this);
}

structural_input::structural_input(
	jive::structural_node * node,
	size_t index,
	jive::oport * origin,
	const struct jive_resource_class * rescls)
	: iport(node->region(), *jive_resource_class_get_type(rescls), index, origin, rescls)
	, node_(node)
	, type_(jive_resource_class_get_type(rescls)->copy())
{
	arguments.first = arguments.last = nullptr;
	node->graph()->on_iport_create(this);
}

const jive::base::type &
structural_input::type() const noexcept
{
	return *type_;
}

jive::region *
structural_input::region() const noexcept
{
	return node()->region();
}

jive::node *
structural_input::node() const noexcept
{
	return node_;
}

/* structural output */

structural_output::~structural_output()
{
	JIVE_DEBUG_ASSERT(results.first == nullptr && results.last == nullptr);

	node()->graph()->on_oport_destroy(this);

	if (gate()) {
		for (size_t n = 0; n < node()->noutputs(); n++) {
			auto other = node()->output(n);
			if (other == this || !other->gate())
				continue;
			jive_gate_interference_remove(node()->graph(), gate(), other->gate());
		}
	}

	for (size_t n = index()+1; n < node()->noutputs(); n++)
		static_cast<jive::structural_node*>(node())->output(n)->set_index(n-1);
}

structural_output::structural_output(
	jive::structural_node * node,
	size_t index,
	const jive::base::type & type)
	: oport(index)
	, node_(node)
	, type_(type.copy())
{
	results.first = results.last = nullptr;

	node->graph()->on_oport_create(this);
}

structural_output::structural_output(
	jive::structural_node * node,
	size_t index,
	jive::gate * gate)
	: oport(index, gate)
	, node_(node)
	, type_(gate->type().copy())
{
	results.first = results.last = nullptr;

	for (size_t n = 0; n < index; n++) {
		jive::oport * other = node->output(n);
		if (!other->gate()) continue;
		jive_gate_interference_add(node->graph(), gate, other->gate());
	}

	node->graph()->on_oport_create(this);
}

structural_output::structural_output(
	jive::structural_node * node,
	size_t index,
	const struct jive_resource_class * rescls)
	: oport(index, rescls)
	, node_(node)
	, type_(jive_resource_class_get_type(rescls)->copy())
{
	results.first = results.last = nullptr;

	node->graph()->on_oport_create(this);
}

const jive::base::type &
structural_output::type() const noexcept
{
	return *type_;
}

jive::region *
structural_output::region() const noexcept
{
	return node()->region();
}

jive::node *
structural_output::node() const noexcept
{
	return node_;
}

/* structural node */

structural_node::~structural_node()
{
	graph()->on_node_destroy(this);

	subregions_.clear();

	while (ninputs())
		remove_input(ninputs()-1);

	while(noutputs())
		remove_output(noutputs()-1);
}

structural_node::structural_node(
	const jive::operation & op,
	jive::region * region,
	size_t nsubregions)
	: node(op.copy(), region, 0)
{
	/* FIXME: check that nsubregions is unequal zero */
	for (size_t n = 0; n < nsubregions; n++)
		subregions_.emplace_back(std::unique_ptr<jive::region>(new jive::region(this)));

	graph()->on_node_create(this);
}

bool
structural_node::has_users() const noexcept
{
	for (const auto & output : outputs_) {
		if (output->nusers() != 0)
			return true;
	}

	return false;
}

bool
structural_node::has_successors() const noexcept
{
	for (const auto & output : outputs_) {
		for (const auto & user : *output) {
			if (user->node())
				return true;
		}
	}

	return false;
}

size_t
structural_node::ninputs() const noexcept
{
	return inputs_.size();
}

jive::structural_input *
structural_node::input(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < ninputs());
	return inputs_[index].get();
}

size_t
structural_node::noutputs() const noexcept
{
	return outputs_.size();
}

jive::structural_output *
structural_node::output(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < noutputs());
	return outputs_[index].get();
}

jive::structural_input *
structural_node::add_input(const jive::base::type * type, jive::oport * origin)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	if (origin->node() && !origin->node()->has_users())
		JIVE_LIST_REMOVE(origin->node()->region()->bottom_nodes, origin->node(), region_bottom_list);

	inputs_.emplace_back(std::unique_ptr<structural_input>(
		new structural_input(this, inputs_.size(), origin, *type)));
	auto input = inputs_[inputs_.size()-1].get();

	recompute_depth();

	return input;
}

jive::structural_input *
structural_node::add_input(jive::gate * gate, jive::oport * origin)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	if (origin->node() && !origin->node()->has_users())
		JIVE_LIST_REMOVE(origin->node()->region()->bottom_nodes, origin->node(), region_bottom_list);

	inputs_.emplace_back(std::unique_ptr<structural_input>(
		new structural_input(this, inputs_.size(), origin, gate)));
	auto input = inputs_[inputs_.size()-1].get();

	recompute_depth();

	return input;
}

jive::structural_input *
structural_node::add_input(const struct jive_resource_class * rescls, jive::oport * origin)
{
	if (ninputs() == 0)
		JIVE_LIST_REMOVE(region()->top_nodes, this, region_top_node_list);

	if (origin->node() && !origin->node()->has_users())
		JIVE_LIST_REMOVE(origin->node()->region()->bottom_nodes, origin->node(), region_bottom_list);

	inputs_.emplace_back(std::unique_ptr<structural_input>(
		new structural_input(this, inputs_.size(), origin, rescls)));
	auto input = inputs_[inputs_.size()-1].get();

	recompute_depth();

	return input;
}

void
structural_node::remove_input(size_t index)
{
	JIVE_DEBUG_ASSERT(index < inputs_.size());

	for (size_t n = index; n < ninputs()-1; n++) {
		JIVE_DEBUG_ASSERT(input(n+1)->index() == n);
		inputs_[n] = std::move(inputs_[n+1]);
	}
	inputs_.pop_back();

	if (ninputs() == 0)
		JIVE_LIST_PUSH_BACK(region()->top_nodes, this, region_top_node_list);
}

jive::structural_output *
structural_node::add_output(const jive::base::type * type)
{
	outputs_.emplace_back(std::unique_ptr<structural_output>(
		new structural_output(this, noutputs(), *type)));
	return this->output(noutputs()-1);
}

jive::structural_output *
structural_node::add_output(jive::gate * gate)
{
	outputs_.emplace_back(std::unique_ptr<structural_output>(
		new structural_output(this, noutputs(), gate)));
	return this->output(noutputs()-1);
}

jive::structural_output *
structural_node::add_output(const struct jive_resource_class * rescls)
{
	outputs_.emplace_back(std::unique_ptr<structural_output>(
		new structural_output(this, noutputs(), rescls)));
	return this->output(noutputs()-1);
}

void
structural_node::remove_output(size_t index)
{
	JIVE_DEBUG_ASSERT(index < outputs_.size());

	for (size_t n = index; n < noutputs()-1; n++) {
		JIVE_DEBUG_ASSERT(output(n+1)->index() == n);
		outputs_[n] = std::move(outputs_[n+1]);
	}
	outputs_.pop_back();
}

jive::structural_node *
structural_node::copy(jive::region * region, const std::vector<jive::oport*> & operands) const
{
	jive::substitution_map smap;

	size_t noperands = std::min(operands.size(), ninputs());
	for (size_t n = 0; n < noperands; n++)
		smap.insert(input(n)->origin(), operands[n]);

	return copy(region, smap);
}

jive::structural_node *
structural_node::copy(jive::region * region, jive::substitution_map & smap) const
{
	graph()->mark_denormalized();
	jive::structural_node * new_node = new structural_node(operation(), region, 0);

	/* copy inputs */
	for (size_t n = 0; n < ninputs(); n++) {
		auto origin = smap.lookup(input(n)->origin());
		if (!origin) origin = input(n)->origin();

		jive::structural_input * new_input;
		if (input(n)->gate()) {
			auto gate = input(n)->gate();
			auto new_gate = smap.lookup(gate);
			if (!new_gate) {
				new_gate = graph()->create_gate(gate);
				smap.insert(gate, new_gate);
			}

			new_input = new_node->add_input(new_gate, origin);
		} else if (input(n)->rescls() != &jive_root_resource_class) {
			new_input = new_node->add_input(input(n)->rescls(), origin);
		} else {
			new_input = new_node->add_input(&input(n)->type(), origin);
		}
		smap.insert(input(n), new_input);
	}

	/* copy outputs */
	for (size_t n = 0; n < noutputs(); n++) {
		jive::structural_output * new_output;
		if (output(n)->gate()) {
			auto gate = output(n)->gate();
			auto new_gate = smap.lookup(gate);
			if (!new_gate) {
				new_gate = graph()->create_gate(gate);
				smap.insert(gate, new_gate);
			}

			new_output = new_node->add_output(new_gate);
		} else {
			new_output = new_node->add_output(output(n)->rescls());
		}
		smap.insert(output(n), new_output);
	}

	/* copy regions */
	for (size_t n = 0; n < nsubregions(); n++) {
		new_node->subregions_.emplace_back(std::unique_ptr<jive::region>(new jive::region(new_node)));
		auto new_subregion = new_node->subregions_.back().get();
		subregion(n)->copy(new_subregion, smap);
	}

	return new_node;
}

}
