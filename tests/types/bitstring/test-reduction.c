/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static void
assert_constant(jive::output * bitstr, size_t nbits, const char bits[])
{
	jive_bitconstant_node * node = dynamic_cast<jive_bitconstant_node *>(bitstr->node());
	assert(node);
	
	assert(node->operation().bits.size() == nbits);
	assert(strncmp(&node->operation().bits[0], bits, nbits) == 0);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	jive::output * a = jive_bitconstant(graph, 4, "1100");
	jive::output * b = jive_bitconstant(graph, 4, "1010");
	
	jive::output * ops[] = {a, b};
	
	assert_constant(jive_bitand(2, ops), 4, "1000");
	assert_constant(jive_bitor(2, ops), 4, "1110");
	assert_constant(jive_bitxor(2, ops), 4, "0110");
	assert_constant(jive_bitsum(2, ops), 4, "0001");
	assert_constant(jive_bitmultiply(2, ops), 4, "1111");
	assert_constant(jive_bitconcat(2, ops), 8, "11001010");
	assert_constant(jive_bitnegate(a), 4, "1011");
	assert_constant(jive_bitnegate(b), 4, "1101");
	
	jive_graph_prune(graph);
	
	jive::output * x = jive_bitsymbolicconstant(graph, 16, "x");
	jive::output * y = jive_bitsymbolicconstant(graph, 16, "y");
	
	{
		jive::output *  tmparray0[] = {x, y};
		jive::output * concat = jive_bitconcat(2, tmparray0);
		jive::output * slice = jive_bitslice(concat, 8, 24);
		jive_node * node = ((jive::output *) slice)->node();
		assert(node->class_ == &JIVE_BITCONCAT_NODE);
		assert(node->ninputs == 2);
		assert(node->producer(0)->class_ == &JIVE_BITSLICE_NODE);
		assert(node->producer(1)->class_ == &JIVE_BITSLICE_NODE);
		
		const jive::bits::slice_op * attrs;
		attrs = (const jive::bits::slice_op *) jive_node_get_attrs(node->producer(0));
		assert( (attrs->low() == 8) && (attrs->high() == 16) );
		attrs = (const jive::bits::slice_op *) jive_node_get_attrs(node->producer(1));
		assert( (attrs->low() == 0) && (attrs->high() == 8) );
		
		assert(node->producer(0)->inputs[0]->origin() == x);
		assert(node->producer(1)->inputs[0]->origin() == y);
	}
	
	{
		jive::output * slice1 = jive_bitslice(x, 0, 8);
		jive::output * slice2 = jive_bitslice(x, 8, 16);
		jive::output * tmparray1[] = {slice1, slice2};
		jive::output * concat = jive_bitconcat(2, tmparray1);
		assert(concat == x);
	}
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-reduction", test_main);
