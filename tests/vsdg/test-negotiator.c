/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/context.h>
#include <jive/types/bitstring/type.h>
#include <jive/view.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/negotiator.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/node.h>

/* negotiator test node */

typedef uint32_t test_option_t;

extern const jive_node_class NEGTESTNODE;

typedef struct negtestnode negtestnode;
typedef struct negtestnode_attrs negtestnode_attrs;

struct negtestnode_attrs : public jive_node_attrs {
	size_t ninputs;
	test_option_t * input_options;
	size_t noutputs;
	test_option_t * output_options;
	jive_type ** output_types;
};

struct negtestnode : public jive_node {
	negtestnode_attrs attrs;
	struct {
		negtestnode * prev;
		negtestnode * next;
	} split_node_list;
};

static void
negtestnode_init_(
	negtestnode * self,
	jive_region * region,
	
	size_t noperands,
	const test_option_t input_options[],
	const jive_type * const operand_types[],
	jive_output * const operands[],
	
	size_t noutputs,
	const test_option_t output_options[],
	const jive_type * const output_types[]);

static void
negtestnode_fini_(jive_node * self);

static const jive_node_attrs *
negtestnode_get_attrs_(const jive_node * self);

static bool
negtestnode_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
negtestnode_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class NEGTESTNODE = {
	parent : &JIVE_NODE,
	name : "NEGTESTNODE",
	fini : negtestnode_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	get_attrs : negtestnode_get_attrs_, /* override */
	match_attrs : negtestnode_match_attrs_, /* override */
	check_operands : jive_node_check_operands_, /* inherit */
	create : negtestnode_create_, /* override */
};

static void
negtestnode_init_(
	negtestnode * self,
	jive_region * region,
	
	size_t noperands,
	const test_option_t input_options[],
	const jive_type * const operand_types[],
	jive_output * const operands[],
	
	size_t noutputs,
	const test_option_t output_options[],
	const jive_type * const output_types[])
{
	jive_node_init_(self, region,
		noperands, operand_types, operands,
		noutputs, output_types);
	jive_context * context = self->region->graph->context;
	
	self->attrs.ninputs = noperands;
	self->attrs.input_options = jive_context_malloc(context, sizeof(test_option_t) * noperands);
	self->attrs.noutputs = noutputs;
	self->attrs.output_options = jive_context_malloc(context, sizeof(test_option_t) * noutputs);
	self->attrs.output_types = jive_context_malloc(context, sizeof(jive_type *) * noutputs);
	
	size_t n;
	for (n = 0; n < noperands; ++n)
		self->attrs.input_options[n] = input_options[n];
	for (n = 0; n < noutputs; ++n)
		self->attrs.output_options[n] = output_options[n];
	for (n = 0; n < noutputs; ++n)
		self->attrs.output_types[n] = jive_type_copy(output_types[n]);
}

static void
negtestnode_fini_(jive_node * self_)
{
	negtestnode * self = (negtestnode *) self_;
	
	jive_context * context = self->region->graph->context;
	jive_context_free(context, self->attrs.input_options);
	jive_context_free(context, self->attrs.output_options);
	size_t n;
	for (n = 0; n < self->attrs.noutputs; n++) {
		jive_type_destroy(self->attrs.output_types[n]);
	}
	jive_context_free(context, self->attrs.output_types);
	jive_node_fini_(self_);
}

static const jive_node_attrs *
negtestnode_get_attrs_(const jive_node * self_)
{
	const negtestnode * self = (const negtestnode *) self_;
	return &self->attrs;
}

static bool
negtestnode_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	return false;
}

static jive_node *
negtestnode_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const negtestnode_attrs * attrs = (const negtestnode_attrs *) attrs_;
	
	negtestnode * node = new negtestnode;
	node->class_ = &NEGTESTNODE;
	const jive_type * operand_types[noperands];
	size_t n;
	for (n = 0; n < noperands; n++)
		operand_types[n] = jive_output_get_type(operands[n]);
	
	negtestnode_init_(node, region,
		noperands, attrs->input_options, operand_types, operands,
		attrs->noutputs, attrs->output_options, (const jive_type * const *) attrs->output_types);
	
	return node;
}

static jive_node *
jive_negtestnode_create(
	jive_region * region,
	
	size_t noperands,
	const test_option_t input_options[],
	const jive_type * const operand_types[],
	jive_output * const operands[],
	
	size_t noutputs,
	const test_option_t output_options[],
	const jive_type * const output_types[])
{
	negtestnode_attrs attrs;
	attrs.ninputs = noperands;
	attrs.input_options = (test_option_t *) input_options;
	attrs.noutputs = noutputs;
	attrs.output_options = (test_option_t *) output_options;
	attrs.output_types = (jive_type **) output_types;
	
	const jive_node_normal_form * nf =
		jive_graph_get_nodeclass_form(region->graph, &NEGTESTNODE);
	jive_node * node = jive_node_cse_create(nf, region, &attrs, noperands, operands);
	return node;
}

/**/

typedef struct test_negotiator_option test_negotiator_option;

struct test_negotiator_option {
	jive_negotiator_option base;
	test_option_t mask;
};

static void
test_negotiator_option_fini_(const jive_negotiator * self, jive_negotiator_option * option)
{
}

static jive_negotiator_option *
test_negotiator_option_create_(const jive_negotiator * self)
{
	test_negotiator_option * option = jive_context_malloc(self->context, sizeof(*option));
	option->mask = 0;
	return &option->base;
}

static bool
test_negotiator_option_equals_(
	const jive_negotiator * self,
	const jive_negotiator_option * o1_,
	const jive_negotiator_option * o2_)
{
	const test_negotiator_option * o1 = (const test_negotiator_option *) o1_;
	const test_negotiator_option * o2 = (const test_negotiator_option *) o2_;
	return o1->mask == o2->mask;
}

static bool
test_negotiator_option_specialize_(
	const jive_negotiator * self,
	jive_negotiator_option * option_)
{
	test_negotiator_option * option = (test_negotiator_option *) option_;
	
	JIVE_DEBUG_ASSERT(option->mask < 65535);
	/* if this is a power of two already, nothing to do */
	if ( (option->mask & (option->mask -1)) == 0 )
		return false;
	
	uint32_t x = 0x80000000;
	while ( !(x & option->mask) )
		x >>= 1;
	option->mask = x;
	return true;
}

static bool
test_negotiator_option_intersect_(
	const jive_negotiator * self,
	jive_negotiator_option * dst_,
	const jive_negotiator_option * src_)
{
	test_negotiator_option * dst = (test_negotiator_option *) dst_;
	const test_negotiator_option * src = (const test_negotiator_option *) src_;
	
	uint32_t tmp = dst->mask & src->mask;
	if (tmp == 0)
		return false;
	dst->mask = tmp;
	return true;
}

static bool
test_negotiator_option_assign_(
	const jive_negotiator * self,
	jive_negotiator_option * dst_,
	const jive_negotiator_option * src_)
{
	test_negotiator_option * dst = (test_negotiator_option *) dst_;
	const test_negotiator_option * src = (const test_negotiator_option *) src_;
	
	if (src->mask == dst->mask)
		return false;
	dst->mask = src->mask;
	return true;
}

static void
test_negotiator_annotate_node_proper_(jive_negotiator * self, jive_node * node_)
{
	if (jive_node_isinstance(node_, &NEGTESTNODE)) {
		negtestnode * node = (negtestnode *) node_;
		
		size_t n;
		for (n = 0; n < node_->ninputs; n++) {
			jive_input * input = node_->inputs[n];
			test_negotiator_option option;
			option.mask = node->attrs.input_options[n];
			jive_negotiator_annotate_simple_input(self, input, &option.base);
		}
		for (n = 0; n < node_->noutputs; n++) {
			jive_output * output = node_->outputs[n];
			test_negotiator_option option;
			option.mask = node->attrs.output_options[n];
			jive_negotiator_annotate_simple_output(self, output, &option.base);
		}
	}
}

static bool
test_negotiator_option_gate_default_(
	const jive_negotiator * self_,
	jive_negotiator_option * dst,
	const jive_gate * gate)
{
	test_negotiator_option * option = (test_negotiator_option *) dst;
	option->mask = 1;
	return !!option->mask;
}

static const jive_negotiator_class TEST_NEGOTIATOR_CLASS = {
	option_fini : test_negotiator_option_fini_,
	option_create : test_negotiator_option_create_,
	option_equals : test_negotiator_option_equals_,
	option_specialize : test_negotiator_option_specialize_,
	option_intersect : test_negotiator_option_intersect_,
	option_assign : test_negotiator_option_assign_,
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
	jive_output * o,
	test_option_t o_o,
	jive_input * i,
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
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	setlocale(LC_ALL, "");
	
	jive_bitstring_type bits32(32);
	test_option_t opt1 = 1;
	test_option_t opt2 = 2;
	test_option_t opt3 = 3;
	test_option_t opt4 = 6;

	const jive_type * tmparray0[] = {&bits32};
	jive_node * n1 = jive_negtestnode_create(graph->root_region,
		0, 0, 0, 0,
		1, &opt1, tmparray0);
	jive_node * n2 = jive_negtestnode_create(graph->root_region,
		1, &opt1, tmparray0, n1->outputs,
		0, 0, 0);
	jive_node * n3 = jive_negtestnode_create(graph->root_region,
		0, 0, 0, 0,
		1, &opt1, tmparray0);
	jive_node * n4 = jive_negtestnode_create(graph->root_region,
		1, &opt2, tmparray0, n3->outputs,
		0, 0, 0);
	
	jive_region * subregion = jive_region_create_subregion(graph->root_region);
	jive_node * n5 = jive_negtestnode_create(graph->root_region,
		0, 0, 0, 0,
		1, &opt1, tmparray0);
	jive_node * n6 = jive_negtestnode_create(subregion,
		1, &opt3, tmparray0, n5->outputs,
		0, 0, 0);
	jive_node * n7 = jive_negtestnode_create(subregion,
		1, &opt4, tmparray0, n5->outputs,
		0, 0, 0);
	
	test_negotiator nego;
	test_negotiator_init(&nego, graph);
	
	jive_negotiator_process(&nego);
	
	expect_options(&nego, n1->outputs[0], 1, n2->inputs[0], 1);
	expect_options(&nego, n3->outputs[0], 1, n4->inputs[0], 2);
	expect_options(&nego, n5->outputs[0], 1, n6->inputs[0], 2);
	expect_options(&nego, n5->outputs[0], 1, n7->inputs[0], 2);
	
	jive_negotiator_insert_split_nodes(&nego);
	
	assert(n2->inputs[0]->origin == n1->outputs[0]);
	assert(n4->inputs[0]->origin != n3->outputs[0]);
	jive_node * split_node = n4->inputs[0]->origin->node;
	expect_options(&nego, n3->outputs[0], 1, split_node->inputs[0], 1);
	expect_options(&nego, split_node->outputs[0], 2, n4->inputs[0], 2);
	
	jive_negotiator_remove_split_nodes(&nego);
	
	assert(n4->inputs[0]->origin == n3->outputs[0]);
	expect_options(&nego, n3->outputs[0], 1, n4->inputs[0], 2);
	
	test_negotiator_fini(&nego);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-negotiator", test_main);
