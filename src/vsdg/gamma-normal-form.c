/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma-normal-form.h>

#include <jive/vsdg/control.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/substitution.h>

namespace jive {

gamma_normal_form::~gamma_normal_form() noexcept
{
}

gamma_normal_form::gamma_normal_form(
	const std::type_info & operator_class,
	jive::node_normal_form * parent,
	jive::graph * graph) noexcept
	: structural_normal_form(operator_class, parent, graph)
	, enable_predicate_reduction_(true)
	, enable_invariant_reduction_(true)
{
	if (auto p = dynamic_cast<gamma_normal_form *>(parent)) {
		enable_predicate_reduction_ = p->enable_predicate_reduction_;
		enable_invariant_reduction_ = p->enable_invariant_reduction_;
	}
}

bool
gamma_normal_form::normalize_node(jive::node * node_) const
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive::gamma_op*>(&node_->operation()));
	auto node = static_cast<const jive::structural_node*>(node_);

	if (!get_mutable())
		return true;

	bool was_normalized = true;
	if (get_predicate_reduction()) {
		auto constant = node->input(0)->origin()->node();
		if (constant && dynamic_cast<const jive::ctl::constant_op*>(&constant->operation())) {
			auto op = static_cast<const jive::ctl::constant_op*>(&constant->operation());
			size_t alternative = op->value().alternative();

			jive::substitution_map smap;
			JIVE_DEBUG_ASSERT(node->subregion(alternative)->narguments() == node->ninputs()-1);
			for (size_t n = 1; n < node->ninputs(); n++)
				smap.insert(node->subregion(alternative)->argument(n-1), node->input(n)->origin());

			node->subregion(alternative)->copy(node->region(), smap, false, false);

			JIVE_DEBUG_ASSERT(node->subregion(alternative)->nresults() == node->noutputs());
			for (size_t n = 0; n < node->noutputs(); n++) {
				auto result = node->subregion(alternative)->result(n);
				node->output(n)->replace(smap.lookup(result->origin()));
			}
			was_normalized = false;
		}
	}
#if 0
	/* FIXME: invariant reduction is broken */
	if (get_invariant_reduction()) {
		size_t nalternatives = node->ninputs()-1;
		for (size_t v = node->noutputs(); v > 0; --v) {
			size_t n;
			jive::node * tmp = dynamic_cast<jive::output*>(node->input(0)->origin())->node();
			jive::output * value = dynamic_cast<jive::output*>(tmp->input(v-1)->origin());
			for (n = 1; n < nalternatives; n++) {
				jive::node * tail_node = dynamic_cast<jive::output*>(node->input(n)->origin())->node();
				if (value != dynamic_cast<jive::output*>(tail_node->input(v-1)->origin()))
					break;
			}
			if (n == nalternatives) {
				node->output(v-1)->replace(tmp->input(v-1)->origin());
				delete node->output(v-1);
				for (size_t n = 0; n < nalternatives; n++)
					dynamic_cast<jive::output*>(node->input(n)->origin())->node()->remove_input(v-1);
				was_normalized = false;
			}
		}
	}
#endif

	return was_normalized;
}

bool
gamma_normal_form::operands_are_normalized(
	const jive::operation & op,
	const std::vector<jive::oport*> & arguments) const
{
	if (!get_mutable())
		return true;
	
	if (get_predicate_reduction()) {
		auto constant  = arguments[0]->node();
		if (constant && dynamic_cast<const jive::ctl::constant_op*>(&constant->operation()))
			return false;
	}
	
	if (get_invariant_reduction()) {
		size_t nalternatives = arguments.size()-1;
		auto arg0 = dynamic_cast<jive::output*>(arguments[0]);
		for (size_t v = 0; v < arg0->node()->ninputs(); v++) {
			size_t n;
			jive::output * value = dynamic_cast<jive::output*>(arg0->node()->input(v)->origin());
			for (n = 1; n < nalternatives; n++) {
				auto argn = dynamic_cast<jive::output*>(arguments[n]);
				if (value != argn->node()->input(v)->origin())
					break;
			}
			if (n == nalternatives)
				return false;
		}
	}

	return true;
}

void
gamma_normal_form::set_predicate_reduction(bool enable)
{
	if (enable_predicate_reduction_ == enable) {
		return;
	}
	
	children_set<gamma_normal_form, &gamma_normal_form::set_predicate_reduction>(enable);

	enable_predicate_reduction_ = enable;

	if (enable && get_mutable())
		graph()->mark_denormalized();
}

void
gamma_normal_form::set_invariant_reduction(bool enable)
{
	if (enable_invariant_reduction_ == enable) {
		return;
	}

	children_set<gamma_normal_form, &gamma_normal_form::set_invariant_reduction>(enable);

	enable_invariant_reduction_ = enable;

	if (enable && get_mutable())
		graph()->mark_denormalized();
}

}
