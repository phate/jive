/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/regselector.h>

#include <stdint.h>

#include <jive/arch/load.h>
#include <jive/arch/regvalue.h>
#include <jive/arch/store.h>
#include <jive/common.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/nullary.h>
#include <jive/rvsdg/splitnode.h>
#include <jive/rvsdg/traverser.h>
#include <jive/rvsdg/type.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/float/flttype.h>

/* register classifier */

namespace jive {

register_classifier::~register_classifier() noexcept
{}

}

class jive_regselector_option final : public jive_negotiator_option {
public:
	virtual
	~jive_regselector_option() noexcept {}

	inline constexpr
	jive_regselector_option() noexcept : mask(0) {}

	inline constexpr
	jive_regselector_option(uint32_t init_mask) noexcept : mask(init_mask) {}

	/* test two options for equality */
	virtual bool
	operator==(const jive_negotiator_option & generic_other) const noexcept override
	{
		const jive_regselector_option & other =
			static_cast<const jive_regselector_option &>(generic_other);
		return mask == other.mask;
	}

	/* specialize option, return true if changed */
	virtual bool
	specialize() noexcept override
	{
		/* if this is a power of two already, nothing to do */
		if ( (mask & (mask - 1)) == 0 )
			return false;

		uint32_t x = 0x80000000;
		while ( !(x & mask) )
			x >>= 1;
		mask = x;
		return true;
	}

	virtual bool
	intersect(const jive_negotiator_option & generic_other) noexcept override
	{
		const jive_regselector_option & other =
			static_cast<const jive_regselector_option &>(generic_other);

		uint32_t tmp = other.mask & mask;
		if (tmp == 0) {
			return false;
		} else {
			mask = tmp;
			return true;
		}
	}

	virtual bool
	assign(const jive_negotiator_option & generic_other) noexcept override
	{
		const jive_regselector_option & other =
			static_cast<const jive_regselector_option &>(generic_other);
		if (mask == other.mask) {
			return false;
		} else {
			mask = other.mask;
			return true;
		}
	}

	virtual jive_regselector_option *
	copy() const
	{
		return new jive_regselector_option(mask);
	}

	uint32_t mask;
};

static uint32_t
jive_regselector_classify_regcls(
	const jive::register_selector * self,
	const jive::register_class * regcls)
{
	uint32_t option_mask = 0;
	size_t n = 0;
	for (n = 0; n < self->classifier->nclasses(); ++n) {
		if (regcls == self->classifier->classes()[n])
			option_mask |= (1 << n);
	}
	return option_mask;
}

namespace jive {

jive_negotiator_option *
register_selector::create_option() const
{
	auto option = new jive_regselector_option(0);
	option->mask = 0;
	return option;
}

void
register_selector::annotate_node_proper(jive::node * node)
{
	auto gen_op = &node->operation();
	
	jive_regselector_option option;
	if (auto op = dynamic_cast<const jive::regvalue_op *>(gen_op)) {
		auto regcls = op->regcls();
		option.mask = jive_regselector_classify_regcls(this, regcls);
		jive_negotiator_annotate_simple_output(this,
			dynamic_cast<jive::simple_output*>(node->output(0)), &option);
	} else if (auto op = dynamic_cast<const jive::flt::unary_op *>(gen_op)) {
		option.mask = classifier->classify_float_unary(*op);
		jive_negotiator_annotate_identity_node(this, node, &option);
	} else if (auto op = dynamic_cast<const jive::flt::binary_op *>(gen_op)) {
		option.mask = classifier->classify_float_binary(*op);
		jive_negotiator_annotate_identity_node(this, node, &option);
	} else if (auto op = dynamic_cast<const jive::flt::compare_op *>(gen_op)) {
		option.mask = classifier->classify_float_compare(*op);
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		std::vector<jive::simple_output*> outputs;
		for (size_t n = 0; n < node->noutputs(); n++)
			outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
		jive_negotiator_annotate_identity(this, 2, &input, 0, &outputs[0], &option);
	} else if (auto op = dynamic_cast<const jive::bitunary_op *>(gen_op)) {
		option.mask = classifier->classify_fixed_unary(*op);
		jive_negotiator_annotate_identity_node(this, node, &option);
	} else if (auto op = dynamic_cast<const jive::bitbinary_op *>(gen_op)) {
		option.mask = classifier->classify_fixed_binary(*op);
		jive_negotiator_annotate_identity_node(this, node, &option);
	} else if (auto op = dynamic_cast<const jive::bitcompare_op *>(gen_op)) {
		option.mask = classifier->classify_fixed_compare(*op);
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		std::vector<jive::simple_output*> outputs;
		for (size_t n = 0; n < node->noutputs(); n++)
			outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
		jive_negotiator_annotate_identity(this, 2, &input, 0, &outputs[0], &option);
	} else if (dynamic_cast<const jive::load_op *>(gen_op)) {
		option.mask = classifier->classify_address();
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		jive_negotiator_annotate_identity(this, 1, &input, 0, NULL, &option);
		
		auto type = &node->output(0)->type();
		auto rescls = node->output(0)->port().rescls();
		option.mask = classifier->classify_type(type, rescls);
		std::vector<jive::simple_output*> outputs;
		for (size_t n = 0; n < node->noutputs(); n++)
			outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
		jive_negotiator_annotate_identity(this, 0, NULL, 1, &outputs[0], &option);
	} else if (dynamic_cast<const jive::store_op *>(gen_op)) {
		option.mask = classifier->classify_address();
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		jive_negotiator_annotate_identity(this, 1, &input, 0, NULL, &option);
		
		auto type = &node->input(1)->type();
		auto rescls = node->input(1)->port().rescls();
		option.mask = classifier->classify_type(type, rescls);
		input = dynamic_cast<jive::simple_input*>(node->input(1));
		jive_negotiator_annotate_identity(this, 1, &input, 0, NULL, &option);
	} else if (is_split_op(*gen_op)) {
		auto input = dynamic_cast<jive::simple_input*>(node->input(0));
		auto output = dynamic_cast<jive::simple_output*>(node->output(0));
		
		if (dynamic_cast<const jive::valuetype*>(&input->type())) {
			jive_regselector_option option;
			option.mask = classifier->classify_type(&input->type(), input->port().rescls());
			if (option.mask) {
				auto input = dynamic_cast<jive::simple_input*>(node->input(0));
				jive_negotiator_annotate_identity(this, 1, &input, 0, NULL, &option);
			}
		}
		
		if (dynamic_cast<const jive::valuetype*>(&output->type())) {
			jive_regselector_option option;
			option.mask = classifier->classify_type(&output->type(), output->port().rescls());
			if (option.mask) {
				std::vector<jive::simple_output*> outputs;
				for (size_t n = 0; n < node->noutputs(); n++)
					outputs.push_back(dynamic_cast<jive::simple_output*>(node->output(n)));
				jive_negotiator_annotate_identity(this, 0, NULL, 1, &outputs[0], &option);
			}
		}
	}
}

bool
register_selector::store_default_option(
	jive_negotiator_option * dst,
	const jive::gate * gate) const
{
	JIVE_DEBUG_ASSERT(dynamic_cast<const jive_negotiator_option*>(dst));
	auto option = static_cast<jive_regselector_option*>(dst);

	if (!dynamic_cast<const jive::valuetype*>(&gate->type()))
		return false;

	option->mask = classifier->classify_type(&gate->type(), gate->rescls());
	return !!option->mask;
}

} // jive namespace

static const jive::register_class *
jive_regselector_map_port(
	const jive::register_selector * self,
	const jive_negotiator_port * port)
{
	if (!port) return NULL;
	const jive_regselector_option * option;
	option = (const jive_regselector_option *) port->option;
	if (!option->mask) return NULL;
	
	jive_regselect_index index = 0;
	while (! (option->mask & (1 << index) ) )
		index ++;
	return self->classifier->classes()[index];
}

static void
jive_regselector_pull_node(jive::register_selector * self, jive::node * node)
{
	jive::region * root_region = self->graph->root();
	
	if (node->region() == root_region)
		return;
	
	/* FIXME: this is broken */
	return;

	/* determine function region */
	jive::region * region = node->region();
	//while (region->parent() != root_region)
	//	region = region->parent();

	if (auto op = dynamic_cast<const jive::regvalue_op *>(&node->operation())) {
		auto regcls = op->regcls();
		auto origin = node->input(1)->origin()->node();

		if (dynamic_cast<const jive::binary_op*>(&origin->operation())) {
			std::vector<jive::output*> operands(origin->ninputs());
			for (size_t n = 0; n < origin->ninputs(); n++) {
				auto operand = origin->input(n)->origin();
				auto regvalue = dynamic_cast<jive::simple_output*>(jive_regvalue(operand, regcls));
				jive_negotiator_port * reg_port = jive_negotiator_map_output(self, regvalue);
				if (!reg_port)
					self->annotate_node(regvalue->node());
				operands[n] = regvalue;
			}
			
			auto subst = dynamic_cast<jive::simple_output*>(jive::create_normalized(
				region,
				static_cast<const jive::simple_op&>(origin->operation()),
				std::vector<jive::output*>(&operands[0], &operands[0] + origin->ninputs()))[0]);
			
			jive_negotiator_port_split(jive_negotiator_map_output(self,
				dynamic_cast<jive::simple_output*>(node->output(0))));
			node->output(0)->replace(subst);
			
			if (!jive_negotiator_map_output(self, subst))
				self->annotate_node(subst->node());
			jive_negotiator_fully_specialize(self);
		} else if (dynamic_cast<const jive::unary_op*>(&origin->operation())) {
			std::vector<jive::simple_output*> operands(origin->ninputs());
			for (size_t n = 0; n < origin->ninputs(); n++) {
				auto operand = origin->input(n)->origin();
				auto regvalue = dynamic_cast<jive::simple_output*>(jive_regvalue(operand, regcls));
				auto reg_port = jive_negotiator_map_output(self, regvalue);
				if (!reg_port)
					self->annotate_node(regvalue->node());
				operands[n] = regvalue;
			}
			auto subst = dynamic_cast<jive::simple_output*>(jive::create_normalized(
				region, static_cast<const jive::simple_op&>(origin->operation()), {operands[0]})[0]);
			
			jive_negotiator_port_split(jive_negotiator_map_output(self,
				dynamic_cast<jive::simple_output*>(node->output(0))));
			node->output(0)->replace(subst);
			
			if (!jive_negotiator_map_output(self, subst))
				self->annotate_node(subst->node());
			jive_negotiator_fully_specialize(self);
		}
		return;
	}

	size_t n;
	for (n = 0; n < node->ninputs(); n++) {
		auto input = dynamic_cast<jive::simple_input*>(node->input(n));
		if (input->origin()->region() != root_region
		&& !dynamic_cast<const jive::base::nullary_op*>(
			&input->origin()->node()->operation()))
			continue;
		auto port = jive_negotiator_map_input(self, input);
		if (!port)
			continue;
		auto regcls = jive_regselector_map_port(self, port);
		if (!regcls)
			continue;
		
		auto regvalue = dynamic_cast<jive::simple_output*>(jive_regvalue(input->origin(), regcls));
		auto reg_port = jive_negotiator_map_output(self, regvalue);
		if (!reg_port) {
			self->annotate_node(regvalue->node());
			reg_port = jive_negotiator_map_output(self, regvalue);
		}
		
		jive_negotiator_port_divert(port, reg_port->connection);
		input->divert_origin(regvalue);
		jive_negotiator_fully_specialize(self);
	}
}

void
jive_regselector_pull(jive::register_selector * self)
{
	for (auto node : jive::bottomup_traverser(self->graph->root(), true)) {
		jive_regselector_pull_node(self, node);
	}
}

void
jive_regselector_process(jive::register_selector * self)
{
	jive_negotiator_process(self);
	jive_negotiator_insert_split_nodes(self);
	jive_regselector_pull(self);
	jive_negotiator_process(self);
	jive_negotiator_remove_split_nodes(self);
}

const jive::register_class *
jive_regselector_map_output(const jive::register_selector * self, jive::simple_output * output)
{
	auto port = jive_negotiator_map_output(self, output);
	return jive_regselector_map_port(self, port);
}

const jive::register_class *
jive_regselector_map_input(const jive::register_selector * self, jive::simple_input * input)
{
	auto port = jive_negotiator_map_input(self, input);
	return jive_regselector_map_port(self, port);
}

namespace jive {

/* register selector */

register_selector::~register_selector()
{}

register_selector::register_selector(
	jive::graph * graph,
	const jive::register_classifier * _classifier)
: negotiator(graph)
, classifier(_classifier)
{}

}
