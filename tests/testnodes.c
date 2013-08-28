/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "testnodes.h"

#include <jive/vsdg/node-private.h>

/* test node */

static void
jive_test_node_init_(jive_test_node * self, jive_region * region,
	size_t noperands, const jive_type * const operand_types[], jive_output * const operands[],
	size_t nresults, const jive_type * const result_types[])
{
	jive_context * context = region->graph->context;

	jive_node_init_(&self->base, region, noperands, operand_types, operands, nresults, result_types);

	self->attrs.noperands = noperands;
	self->attrs.operand_types = jive_context_malloc(context,
		sizeof(*self->attrs.operand_types) * noperands);

	self->attrs.nresults = nresults;
	self->attrs.result_types = jive_context_malloc(context,
		sizeof(*self->attrs.result_types) * nresults);

	size_t n;
	for (n = 0; n < noperands; n++)
		self->attrs.operand_types[n] = jive_type_copy(operand_types[n], context);

	for (n = 0; n < nresults; n++)
		self->attrs.result_types[n] = jive_type_copy(result_types[n], context);
}

static void
jive_test_node_fini_(jive_node * self_)
{
	jive_test_node * self = (jive_test_node *)self_;
	jive_context * context = self_->graph->context;

	size_t n;
	for (n = 0; n < self->attrs.noperands; n++) {
		jive_type_fini((jive_type *)self->attrs.operand_types[n]);
		jive_context_free(context, (jive_type *)self->attrs.operand_types[n]);
	}
	jive_context_free(context, self->attrs.operand_types);

	for (n = 0; n < self->attrs.nresults; n++) {
		jive_type_fini((jive_type *)self->attrs.result_types[n]);
		jive_context_free(context, (jive_type *)self->attrs.result_types[n]);
	}
	jive_context_free(context, self->attrs.result_types);

	jive_node_fini_(self_);
}

static const jive_node_attrs *
jive_test_node_get_attrs_(const jive_node * self_)
{
	const jive_test_node * self = (const jive_test_node *)self_;
	return &self->attrs.base;
}

static bool
jive_test_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_test_node_attrs * first = &((const jive_test_node *)self)->attrs;
	const jive_test_node_attrs * second = (const jive_test_node_attrs *)attrs;

	if (first->noperands != second->noperands)
		return false;

	if (first->nresults != second->nresults)
		return false;

	size_t n;
	for (n = 0; n < first->noperands; n++) {
		if (!jive_type_equals(first->operand_types[n], second->operand_types[n]))
			return false;
	}

	for (n = 0; n < first->nresults; n++) {
		if (!jive_type_equals(first->result_types[n], second->result_types[n]))
			return false;
	}

	return true;
}

static void
jive_test_node_check_operands_(const jive_node_class * cls, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[], jive_context * context)
{

	const jive_test_node_attrs * attrs = (const jive_test_node_attrs *)attrs_;

	if (noperands != attrs->noperands)
		jive_context_fatal_error(context, "Type mismatch: Number of operands do not coincide.");

	size_t n;
	for (n = 0; n < attrs->noperands; n++) {
		if (!jive_type_equals(attrs->operand_types[n], jive_output_get_type(operands[n])))
			jive_raise_type_error(attrs->operand_types[n], jive_output_get_type(operands[n]), context);
	}
}

static jive_node *
jive_test_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_test_node_attrs * attrs = (const jive_test_node_attrs *)attrs_;
	JIVE_DEBUG_ASSERT(noperands == attrs->noperands);

	jive_test_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_TEST_NODE;
	jive_test_node_init_(node, region, attrs->noperands, attrs->operand_types, operands,
		attrs->nresults, attrs->result_types);

	return &node->base;
}

const jive_node_class JIVE_TEST_NODE = {
	.parent = &JIVE_NODE,
	.name = "TEST_NODE",
	.fini = jive_test_node_fini_, /* override */
	.get_default_normal_form = jive_node_get_default_normal_form_, /* inherit */
	.get_label = jive_node_get_label_, /* inherit */
	.get_attrs = jive_test_node_get_attrs_, /* override */
	.match_attrs = jive_test_node_match_attrs_, /* override */
	.check_operands = jive_test_node_check_operands_, /* override */
	.create = jive_test_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

jive_node *
jive_test_node_create(struct jive_region * region,
	size_t noperands, const jive_type * const operand_types[], jive_output * const operands[],
	size_t nresults, const jive_type * const result_types[])
{
	jive_test_node_attrs attrs;
	attrs.noperands = noperands;
	attrs.operand_types = (const jive_type **)operand_types;
	attrs.nresults = nresults;
	attrs.result_types = (const jive_type **)result_types;

	/* FIXME: maybe introduce a jive_node_create function that does all this */
	jive_test_node_check_operands_(&JIVE_TEST_NODE, &attrs.base, noperands, operands,
		region->graph->context);
	jive_node * node = jive_test_node_create_(region, &attrs.base, noperands, operands);

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
	jive_test_node_attrs attrs;
	attrs.noperands = noperands;
	attrs.operand_types = (const jive_type **)operand_types;
	attrs.nresults = nresults;
	attrs.result_types = (const jive_type **)result_types;

	jive_node_create_normalized(&JIVE_TEST_NODE, graph, &attrs.base, noperands, operands, results);
}
