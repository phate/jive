/*
 * Copyright 2010 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

/* test node */

test_operation::~test_operation() noexcept {}

test_operation::test_operation(
	size_t noperands, const jive_type * const operand_types[],
	size_t nresults, const jive_type * const result_types[])
{
	for (size_t n = 0; n < noperands; ++n) {
		operand_types_.emplace_back(operand_types[n]->copy());
	}
	for (size_t n = 0; n < nresults; ++n) {
		result_types_.emplace_back(result_types[n]->copy());
	}
}

test_operation::test_operation(const test_operation & other)
{
	for (const auto & optype : other.operand_types()) {
		operand_types_.emplace_back(optype->copy());
	}
	for (const auto & restype : other.result_types()) {
		result_types_.emplace_back(restype->copy());
	}
}


static void
jive_test_node_init_(jive_test_node * self, jive_region * region,
	size_t noperands, const jive_type * const operand_types[], jive_output * const operands[],
	size_t nresults, const jive_type * const result_types[])
{
	jive_context * context = region->graph->context;

	jive_node_init_(self, region, noperands, operand_types, operands, nresults, result_types);
}

static void
jive_test_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static bool
jive_test_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const test_operation * first = &((const jive_test_node *)self)->operation();
	const test_operation * second = (const test_operation *)attrs;

	if (first->operand_types().size() != second->operand_types().size())
		return false;

	if (first->result_types().size() != second->result_types().size())
		return false;

	for (size_t n = 0; n < first->operand_types().size(); ++n) {
		if (*first->operand_types()[n] != *second->operand_types()[n])
			return false;
	}

	for (size_t n = 0; n < first->result_types().size(); ++n) {
		if (*first->result_types()[n] != *second->result_types()[n])
			return false;
	}

	return true;
}

static void
jive_test_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[], jive_context * context)
{

	const test_operation * attrs = (const test_operation *)attrs_;

	if (noperands != attrs->operand_types().size())
		jive_context_fatal_error(context, "Type mismatch: Number of operands do not coincide.");

	size_t n;
	for (n = 0; n < attrs->operand_types().size(); n++) {
		const jive_type & type = *attrs->operand_types()[n];
		if (type != *jive_output_get_type(operands[n]))
			jive_raise_type_error(&type, jive_output_get_type(operands[n]), context);
	}
}

static jive_node *
jive_test_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const test_operation * attrs = (const test_operation *)attrs_;
	JIVE_DEBUG_ASSERT(noperands == attrs->operand_types().size());

	jive_test_node * node = new jive_test_node(*attrs);
	node->class_ = &JIVE_TEST_NODE;
	const jive_type * operand_types[noperands];
	for (size_t n = 0; n < noperands; ++n) {
		operand_types[n] = &*attrs->operand_types()[n];
	}
	size_t nresults = attrs->result_types().size();
	const jive_type * result_types[nresults];
	for (size_t n = 0; n < nresults; ++n) {
		result_types[n] = &*attrs->result_types()[n];
	}
	jive_test_node_init_(node, region, noperands, operand_types, operands,
		nresults, result_types);

	return node;
}

const jive_node_class JIVE_TEST_NODE = {
	parent : &JIVE_NODE,
	name : "TEST_NODE",
	fini : jive_test_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_node_get_label_, /* inherit */
	match_attrs : jive_test_node_match_attrs_, /* override */
	check_operands : jive_test_node_check_operands_, /* override */
	create : jive_test_node_create_, /* override */
};

jive_node *
jive_test_node_create(struct jive_region * region,
	size_t noperands, const jive_type * const operand_types[], jive_output * const operands[],
	size_t nresults, const jive_type * const result_types[])
{
	test_operation op(noperands, operand_types, nresults, result_types);
	assert(op.result_types().size() == nresults);

	/* FIXME: maybe introduce a jive_node_create function that does all this */
	jive_test_node_check_operands_(&JIVE_TEST_NODE, &op, noperands, operands,
		region->graph->context);
	jive_node * node = jive_test_node_create_(region, &op, noperands, operands);

	jive_node_normal_form * nf = jive_graph_get_nodeclass_form(region->graph, &JIVE_TEST_NODE);
	if (nf->enable_mutable && nf->enable_cse)
		jive_graph_mark_denormalized(region->graph);

	return node;
}

void
jive_test_node_create_normalized(jive_graph * graph, size_t noperands,
	const jive_type * const operand_types[], jive_output * const operands[], size_t nresults,
	const jive_type * const result_types[], jive_output * results[])
{
	test_operation op(noperands, operand_types, nresults, result_types);

	jive_node_create_normalized(&JIVE_TEST_NODE, graph, &op, noperands, operands, results);
}
