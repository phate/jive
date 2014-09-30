/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/gamma.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/anchor-private.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/traverser.h>

namespace jive {

gamma_tail_op::~gamma_tail_op() noexcept
{
}

jive_node *
gamma_tail_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	JIVE_DEBUG_ASSERT(!region->bottom);
	jive_node * node = jive_opnode_create(
		*this,
		&JIVE_GAMMA_TAIL_NODE,
		region,
		arguments, arguments + narguments);
	region->bottom = node;
	return node;
}

std::string
gamma_tail_op::debug_string() const
{
	return "GAMMA_TAIL";
}

gamma_op::~gamma_op() noexcept
{
}

size_t
gamma_op::narguments() const noexcept
{
	return 1 + nalternatives_;
}

const base::type &
gamma_op::argument_type(size_t index) const noexcept
{
	if (index < nalternatives_) {
		static const achr::type anchor_type;
		return anchor_type;
	} else {
		static const ctl::type control_type;
		return control_type;
	}
}

jive_node *
gamma_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(
		*this,
		&JIVE_GAMMA_NODE,
		region,
		arguments, arguments + narguments);
}

std::string
gamma_op::debug_string() const
{
	return "GAMMA";
}

}

jive_node_normal_form *
jive_gamma_node_get_default_normal_form_(
	const jive_node_class * cls,
	jive_node_normal_form * parent,
	jive_graph * graph)
{
	jive_gamma_normal_form * normal_form = new jive_gamma_normal_form;
	normal_form->base.base.class_ = &JIVE_GAMMA_NORMAL_FORM;
	jive_gamma_normal_form_init_(normal_form, cls, parent, graph);
	return &normal_form->base.base;
}


const jive_node_class JIVE_GAMMA_TAIL_NODE = {
	parent : &JIVE_NODE,
	name : "GAMMA_TAIL",
	fini : jive_node_fini_,  /* inherit */
	get_default_normal_form : jive_node_get_default_normal_form_,  /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

const jive_node_class JIVE_GAMMA_NODE = {
	parent : &JIVE_ANCHOR_NODE,
	name : "GAMMA",
	fini : jive_node_fini_,  /* inherit */
	get_default_normal_form : jive_gamma_node_get_default_normal_form_,  /* override */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

static jive_node *
jive_gamma_create(
	jive_region * region,
	jive::output * predicate,
	size_t nvalues, const jive::base::type * const types[],
	jive::output * const true_values[],
	jive::output * const false_values[])
{
	jive_region * false_region = jive_region_create_subregion(region);
	jive_region * true_region = jive_region_create_subregion(region);
	jive_node * false_alt = jive::gamma_tail_op().create_node(
		false_region, 0, nullptr);
	jive_node * true_alt = jive::gamma_tail_op().create_node(
		true_region, 0, nullptr);
	jive::output * arguments[3] = {
		true_alt->outputs[0],
		false_alt->outputs[0],
		predicate
	};
	jive_node * gamma = jive::gamma_op(2).create_node(
		region,
		3, arguments);
	
	size_t n;
	for (n = 0; n < nvalues; n++) {
		char name[80];
		snprintf(name, sizeof(name), "gamma_%p_%zd", gamma, n);
		jive::gate * gate = types[n]->create_gate(region->graph, name);
		jive_node_gate_input(true_alt, gate, true_values[n]);
		jive_node_gate_input(false_alt, gate, false_values[n]);
		jive_node_gate_output(gamma, gate);
	}
	return gamma;
}

void
jive_gamma(jive::output * predicate,
	size_t nvalues, const struct jive::base::type * const types[],
	jive::output * const true_values[],
	jive::output * const false_values[],
	jive::output * results[])
{
	size_t n;
	
	jive_graph * graph = predicate->node()->region->graph;
	jive_gamma_normal_form * nf;
	nf = (jive_gamma_normal_form *) jive_graph_get_nodeclass_form(graph,
		&JIVE_GAMMA_NODE);
	
	if (nf->base.base.enable_mutable && nf->enable_predicate_reduction) {
		const jive::ctl::constant_op * op =
			dynamic_cast<const jive::ctl::constant_op *>(&predicate->node()->operation());
		if (op && op->value()) {
			for (n = 0; n < nvalues; ++n)
				results[n] = true_values[n];
			return;
		} else if (op && !op->value()) {
			for (n = 0; n < nvalues; ++n)
				results[n] = false_values[n];
			return;
		}
	}
	
	jive::output * tmp[nvalues * 2 + 1];
	tmp[0] = predicate;
	for (n = 0; n < nvalues; n++)
		tmp[n + 1] = false_values[n];
	for (n = 0; n < nvalues; n++)
		tmp[n + nvalues + 1] = true_values[n];
	jive_region * region = jive_region_innermost(nvalues * 2 + 1, tmp);
	
	jive_node * node = jive_gamma_create(region, predicate, nvalues, types, true_values, false_values);
	
	for (n = 0; n < nvalues; ++n)
		results[n] = node->outputs[n];
	
	if (nf->base.base.enable_mutable && nf->enable_invariant_reduction) {
		jive_node * true_branch = node->producer(0);
		jive_node * false_branch = node->producer(1);
		for (n = nvalues; n > 0; --n) {
			if (true_values[n-1] != false_values[n-1])
				continue;
			results[n-1] = true_values[n-1];
			delete node->outputs[n-1];
			delete true_branch->inputs[n-1];
			delete false_branch->inputs[n-1];
		}
	}
}

/* FIXME: this can be done easier. We are basically visiting the nodes in depth first traversal
	and moving them. Rely on traversers for that.
*/

typedef struct jive_move_context jive_move_context;
struct jive_move_context {
	std::vector<std::vector<jive_node*>> depths;
};

static void
jive_move_context_append(jive_move_context * self, jive_context * context, jive_node * node)
{
	if (node->depth_from_root >= self->depths.size())
		self->depths.resize(node->depth_from_root+1);

	self->depths[node->depth_from_root].push_back(node);
}

static void
pre_move_region(jive_region * target_region, const jive_region * original_region,
	jive_move_context * move_context, jive_context * context)
{
	jive_node * node;
	JIVE_LIST_ITERATE(original_region->nodes, node, region_nodes_list) {
		if (node != original_region->bottom)
			jive_move_context_append(move_context, context, node);
	}
}

void
jive_region_move(const jive_region * self, jive_region * target)
{
	jive_context * context = target->graph->context;
	jive_move_context move_context;

	pre_move_region(target, self, &move_context, context);

	for (size_t depth = 0; depth < move_context.depths.size(); depth++) {
		for (size_t n = 0; n < move_context.depths[depth].size(); n++) {
			jive_node * node = move_context.depths[depth][n];
			jive_node_move(node, target);
		}
	}
}

/* normal forms */

const jive_gamma_normal_form_class JIVE_GAMMA_NORMAL_FORM_ = {
	base : { /* jive_anchor_node_normal_form_class */
		base : {	/* jive_node_normal_form_class */
			parent : &JIVE_ANCHOR_NODE_NORMAL_FORM,
			fini : jive_node_normal_form_fini_,	/* inherit */
			normalize_node : jive_gamma_normal_form_normalize_node_,	/* override */
			operands_are_normalized : jive_gamma_normal_form_operands_are_normalized_,	/* override */
			normalized_create : NULL,
			set_mutable : jive_node_normal_form_set_mutable_,	/* inherit */
			set_cse : jive_node_normal_form_set_cse_	/* inherit */
		},
		set_reducible : jive_gamma_normal_form_class_set_reducible_	/* override */
	},
	set_predicate_reduction : jive_gamma_normal_form_class_set_predicate_reduction_,
	set_invariant_reduction : jive_gamma_normal_form_class_set_invariant_reduction_,
};

bool
jive_gamma_normal_form_normalize_node_(const jive_node_normal_form * self_, jive_node * node)
{
	const jive_gamma_normal_form * self = (const jive_gamma_normal_form *) self_;
	
	if (!self->base.base.enable_mutable)
		return true;
	
	JIVE_DEBUG_ASSERT(node->noperands == 3);
	
	if (self->enable_predicate_reduction) {
		jive::output * pred = node->inputs[2]->origin();
		jive_node * branch = 0;
		const jive::ctl::constant_op * op =
			dynamic_cast<const jive::ctl::constant_op *>(&pred->node()->operation());
		if (op && op->value() == true) {
			branch = node->producer(0);
		} else if (op && op->value() == false) {
			branch = node->producer(1);
		}
		
		if (!branch)
			return true;

		/* FIXME: jive_region_push_out should do the same, why do we have the custom
			implementation here?
		*/	
		jive_region_move(branch->region, node->region);
		size_t n;
		for (n = 0; n < node->noutputs; n++) {
			jive_output_replace(node->outputs[n], branch->inputs[n]->origin());
		}
		
		return false;
	}
	
	if (self->enable_invariant_reduction) {
		jive_node * true_branch = node->producer(0);
		jive_node * false_branch = node->producer(1);
		size_t n;
		for (n = node->noutputs; n > 0; --n) {
			if (true_branch->inputs[n-1]->origin() != false_branch->inputs[n-1]->origin())
				continue;
			jive_output_replace(node->outputs[n-1], true_branch->inputs[n-1]->origin());
			delete node->outputs[n-1];
			delete true_branch->inputs[n-1];
			delete false_branch->inputs[n-1];
		}
	}
	
	return true;
}

bool
jive_gamma_normal_form_operands_are_normalized_(const jive_node_normal_form * self_,
	size_t noperands, jive::output * const operands[],
	const jive_node_attrs * attrs)
{
	const jive_gamma_normal_form * self = (const jive_gamma_normal_form *) self_;
	
	if (!self->base.base.enable_mutable)
		return true;
	
	JIVE_DEBUG_ASSERT(noperands == 3);
	
	if (self->enable_predicate_reduction) {
		jive::output * pred = operands[2];
		const jive::ctl::constant_op * op =
			dynamic_cast<const jive::ctl::constant_op *>(&pred->node()->operation());
		if (op) {
			return false;
		}
	}
	
	if (self->enable_invariant_reduction) {
		jive_node * true_branch = operands[0]->node();
		jive_node * false_branch = operands[1]->node();
		size_t n;
		for (n = true_branch->ninputs; n > 0; --n) {
			if (true_branch->inputs[n-1]->origin() == false_branch->inputs[n-1]->origin())
				return false;
		}
	}
	
	return true;
}

void
jive_gamma_normal_form_init_(jive_gamma_normal_form * self,
	const jive_node_class * cls, jive_node_normal_form * parent_,
	jive_graph * graph)
{
	jive_anchor_node_normal_form_init_(&self->base, cls, parent_, graph);
	
	jive_gamma_normal_form * parent = jive_gamma_normal_form_cast(parent_);
	
	if (parent) {
		self->enable_predicate_reduction = parent->enable_predicate_reduction;
		self->enable_invariant_reduction = parent->enable_invariant_reduction;
	} else {
		self->enable_predicate_reduction = true;
		self->enable_invariant_reduction = true;
	}
}

void
jive_gamma_normal_form_class_set_reducible_(jive_anchor_node_normal_form * self_, bool enable)
{
	if (self_->enable_reducible == enable)
		return;

	jive_gamma_normal_form * self = (jive_gamma_normal_form *) self_;

	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.base.subclasses, child, normal_form_subclass_list)
		jive_gamma_normal_form_class_set_reducible_((jive_anchor_node_normal_form *)child, enable);

	self->base.enable_reducible = enable;
	self->enable_predicate_reduction = enable;
	self->enable_invariant_reduction = enable;

	if (self->base.base.enable_mutable && enable)
		jive_graph_mark_denormalized(self->base.base.graph);
}

void
jive_gamma_normal_form_class_set_predicate_reduction_(jive_gamma_normal_form * self, bool enable)
{
	if (self->enable_predicate_reduction == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.base.subclasses, child, normal_form_subclass_list) {
		jive_gamma_normal_form_set_predicate_reduction((jive_gamma_normal_form *)child, enable);
	}

	self->enable_predicate_reduction = enable;
	if (enable)
		self->base.enable_reducible = enable;

	if (enable && self->base.base.enable_mutable)
		jive_graph_mark_denormalized(self->base.base.graph);
}

void
jive_gamma_normal_form_class_set_invariant_reduction_(jive_gamma_normal_form * self, bool enable)
{
	if (self->enable_invariant_reduction == enable)
		return;
	
	jive_node_normal_form * child;
	JIVE_LIST_ITERATE(self->base.base.subclasses, child, normal_form_subclass_list) {
		jive_gamma_normal_form_set_invariant_reduction((jive_gamma_normal_form *)child, enable);
	}

	self->enable_invariant_reduction = enable;
	if (enable)
		self->base.enable_reducible = enable;

	if (enable && self->base.base.enable_mutable)
		jive_graph_mark_denormalized(self->base.base.graph);
}
