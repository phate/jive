/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <memory>
#include <vector>

#include <jive/types/bitstring/type.h>
#include <jive/util/ptr-collection.h>
#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/negotiator.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>

/* negotiator test node */

typedef uint32_t test_option_t;

class negtest_op : public jive::operation {
public:
	virtual
	~negtest_op() noexcept {}

	negtest_op(const negtest_op & other)
		: argument_types_(jive::detail::unique_ptr_vector_copy(other.argument_types_))
		, result_types_(jive::detail::unique_ptr_vector_copy(other.result_types_))
		, input_options_(other.input_options_)
		, output_options_(other.output_options_)
	{
	}

	negtest_op(negtest_op && other) = default;

	negtest_op(
		size_t ninputs,
		const test_option_t * input_options,
		const jive::base::type * const * argument_types,
		size_t noutputs,
		const test_option_t * output_options,
		const jive::base::type * const * result_types)
		: argument_types_(jive::detail::unique_ptr_vector_copy(
			jive::detail::make_array_slice(argument_types, ninputs)))
		, result_types_(jive::detail::unique_ptr_vector_copy(
			jive::detail::make_array_slice(result_types, noutputs)))
		, input_options_(input_options, input_options + ninputs)
		, output_options_(output_options, output_options + noutputs)
	{
	}

	virtual bool
	operator==(const operation & other) const noexcept override
	{
		const negtest_op * op =
			dynamic_cast<const negtest_op *>(&other);
		return op &&
			op->input_options() == input_options() &&
			op->output_options() == output_options() &&
			jive::detail::ptr_container_equals(op->argument_types_, argument_types_) &&
			jive::detail::ptr_container_equals(op->result_types_, result_types_);
	}

	virtual size_t
	narguments() const noexcept override
	{
		return argument_types_.size();
	}

	virtual const jive::base::type &
	argument_type(size_t index) const noexcept override
	{
		return *argument_types_[index];
	}

	virtual size_t
	nresults() const noexcept override
	{
		return result_types_.size();
	}

	virtual const jive::base::type &
	result_type(size_t index) const noexcept override
	{
		return *result_types_[index];
	}

	virtual std::string
	debug_string() const override
	{
		return "NEGTESTNODE";
	}

	inline size_t ninputs() const noexcept { return input_options_.size(); }
	inline size_t noutputs() const noexcept { return output_options_.size(); }
	inline const std::vector<test_option_t> &
	input_options() const noexcept { return input_options_; }
	inline const std::vector<test_option_t> &
	output_options() const noexcept { return output_options_; }

	virtual std::unique_ptr<jive::operation> copy() const override
	{
		return std::unique_ptr<jive::operation>(new negtest_op(*this));
	}

private:
	std::vector<std::unique_ptr<const jive::base::type>> argument_types_;
	std::vector<std::unique_ptr<const jive::base::type>> result_types_;
	std::vector<test_option_t> input_options_;
	std::vector<test_option_t> output_options_;
};

static jive_node *
jive_negtestnode_create(
	jive_region * region,
	
	size_t noperands,
	const test_option_t input_options[],
	const jive::base::type * const operand_types[],
	jive::output * const operands[],
	
	size_t noutputs,
	const test_option_t output_options[],
	const jive::base::type * const output_types[])
{
	negtest_op op(
		noperands, input_options, operand_types,
		noutputs, output_options, output_types);
	
	const jive::node_normal_form * nf =
		jive_graph_get_nodeclass_form(region->graph, typeid(negtest_op));
	std::vector<jive::output *> arguments(operands, operands + noperands);
	return jive_node_cse_create(nf, region, op, arguments);
}

/**/

typedef struct test_negotiator_option test_negotiator_option;

class test_negotiator_option final : public jive_negotiator_option {
public:
	virtual
	~test_negotiator_option() noexcept {}

	inline constexpr
	test_negotiator_option(test_option_t init_mask) noexcept
		: mask(init_mask)
	{
	}

	inline constexpr
	test_negotiator_option() noexcept : mask(0) {}

	/* test two options for equality */
	virtual bool
	operator==(const jive_negotiator_option & generic_other) const noexcept override
	{
		const test_negotiator_option & other =
			static_cast<const test_negotiator_option &>(generic_other);
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
		const test_negotiator_option & other =
			static_cast<const test_negotiator_option &>(generic_other);

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
		const test_negotiator_option & other =
			static_cast<const test_negotiator_option &>(generic_other);
		if (mask == other.mask) {
			return false;
		} else {
			mask = other.mask;
			return true;
		}
	}

	virtual test_negotiator_option *
	copy() const
	{
		return new test_negotiator_option(mask);
	}

	test_option_t mask;
};

static jive_negotiator_option *
test_negotiator_option_create_(const jive_negotiator * self)
{
	test_negotiator_option * option = new test_negotiator_option(0);
	return option;
}

static void
test_negotiator_annotate_node_proper_(jive_negotiator * self, jive_node * node_)
{
	if (auto op = dynamic_cast<const negtest_op *>(&node_->operation())) {
		for (size_t n = 0; n < node_->ninputs(); n++) {
			jive::input * input = node_->input(n);
			test_negotiator_option option(op->input_options()[n]);
			jive_negotiator_annotate_simple_input(self, input, &option);
		}
		for (size_t n = 0; n < node_->noutputs(); n++) {
			jive::output * output = node_->output(n);
			test_negotiator_option option(op->output_options()[n]);
			jive_negotiator_annotate_simple_output(self, output, &option);
		}
	}
}

static bool
test_negotiator_option_gate_default_(
	const jive_negotiator * self_,
	jive_negotiator_option * dst,
	const jive::gate * gate)
{
	test_negotiator_option * option = (test_negotiator_option *) dst;
	option->mask = 1;
	return !!option->mask;
}

static const jive_negotiator_class TEST_NEGOTIATOR_CLASS = {
	option_create : test_negotiator_option_create_,
	option_gate_default : test_negotiator_option_gate_default_,
	annotate_node_proper : test_negotiator_annotate_node_proper_,
	annotate_node : jive_negotiator_annotate_node_,
	process_region : jive_negotiator_process_region_
};

void
test_negotiator_init(jive_negotiator * self, jive_graph * graph)
{
	jive_negotiator_init_(self, &TEST_NEGOTIATOR_CLASS, graph);
}

void
test_negotiator_process(jive_negotiator * self)
{
	jive_negotiator_process(self);
}

void
test_negotiator_fini(jive_negotiator * self)
{
	jive_negotiator_fini_(self);
}

typedef jive_negotiator test_negotiator;

static void
expect_options(
	jive_negotiator * nego,
	jive::output * o,
	test_option_t o_o,
	jive::input * i,
	test_option_t o_i)
{
	jive_negotiator_port * p_o = jive_negotiator_map_output(nego, o);
	jive_negotiator_port * p_i = jive_negotiator_map_input(nego, i);
	assert(p_o);
	assert(p_i);
	
	assert(((test_negotiator_option *)p_o->option)->mask == o_o);
	assert(((test_negotiator_option *)p_i->option)->mask == o_i);
	if (o_o == o_i) {
		assert(p_o->connection == p_i->connection);
	} else {
		assert(p_o->connection != p_i->connection);
	}
}

static int test_main(void)
{
	jive_graph * graph = jive_graph_create();
	
	setlocale(LC_ALL, "");
	
	jive::bits::type bits32(32);
	test_option_t opt1 = 1;
	test_option_t opt2 = 2;
	test_option_t opt3 = 3;
	test_option_t opt4 = 5;

	const jive::base::type * tmparray0[] = {&bits32};
	jive_node * n1 = jive_negtestnode_create(graph->root_region,
		0, 0, 0, 0,
		1, &opt1, tmparray0);
	jive::output * tmp = n1->output(0);
	jive_node * n2 = jive_negtestnode_create(graph->root_region,
		1, &opt1, tmparray0, &tmp,
		0, 0, 0);
	jive_node * n3 = jive_negtestnode_create(graph->root_region,
		0, 0, 0, 0,
		1, &opt1, tmparray0);
	tmp = n3->output(0);
	jive_node * n4 = jive_negtestnode_create(graph->root_region,
		1, &opt2, tmparray0, &tmp,
		0, 0, 0);
	
	jive_region * subregion = new jive_region(graph->root_region, graph);
	jive_node * n5 = jive_negtestnode_create(subregion,
		0, 0, 0, 0,
		1, &opt1, tmparray0);
	tmp = n5->output(0);
	jive_node * n6 = jive_negtestnode_create(subregion,
		1, &opt3, tmparray0, &tmp,
		0, 0, 0);
	jive_node * n7 = jive_negtestnode_create(subregion,
		1, &opt4, tmparray0, &tmp,
		0, 0, 0);
	
	test_negotiator nego;
	test_negotiator_init(&nego, graph);
	
	jive_negotiator_process(&nego);
	
	expect_options(&nego, n1->output(0), 1, n2->input(0), 1);
	expect_options(&nego, n3->output(0), 1, n4->input(0), 2);
	expect_options(&nego, n5->output(0), 1, n6->input(0), 1);
	expect_options(&nego, n5->output(0), 1, n7->input(0), 1);

	jive_negotiator_insert_split_nodes(&nego);
	
	assert(n2->input(0)->origin() == n1->output(0));
	assert(n4->input(0)->origin() != n3->output(0));
	jive_node * split_node = n4->producer(0);
	expect_options(&nego, n3->output(0), 1, split_node->input(0), 1);
	expect_options(&nego, split_node->output(0), 2, n4->input(0), 2);
	
	jive_negotiator_remove_split_nodes(&nego);
	
	assert(n4->input(0)->origin() == n3->output(0));
	expect_options(&nego, n3->output(0), 1, n4->input(0), 2);
	
	test_negotiator_fini(&nego);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-negotiator", test_main);
