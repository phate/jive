/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <string.h>

#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive::output * base_const1 = jive_bitconstant(graph, 8, "00110111");
	jive::output * base_const2 = jive_bitconstant(graph, 8, "11001000");
	
	jive::output * base_x = jive_bitsymbolicconstant(graph, 8, "x");
	jive::output * base_y = jive_bitsymbolicconstant(graph, 8, "y");
	jive::output * base_z = jive_bitsymbolicconstant(graph, 8, "z");
	
	{
		/* slice of constant */
		jive::output * a = jive_bitslice(base_const1, 2, 6);
		
		assert(a->node()->class_ == &JIVE_BITCONSTANT_NODE);
		const jive::bits::type * type = static_cast<const jive::bits::type*>(&a->type());
		assert(type->nbits()==4);
		const jive::bits::constant_operation * attrs =
			(const jive::bits::constant_operation *)jive_node_get_attrs(a->node());
		assert(memcmp(&attrs->bits[0], "1101", 4) == 0);
	}
	
	{
		/* slice of slice */
		jive::output * a = jive_bitslice(base_x, 2, 6);
		jive::output * b = jive_bitslice(a, 1, 3);
	
		assert(b->node()->class_ == &JIVE_BITSLICE_NODE);
		const jive::bits::slice_op * attrs =
			(const jive::bits::slice_op *)jive_node_get_attrs(b->node());
		assert(attrs->low() == 3 && attrs->high() == 5);
	}
	
	{
		/* slice of full node */
		jive::output * a = jive_bitslice(base_x, 0, 8);
		
		assert(a == base_x);
	}
	
	{
		/* slice of concat */
		jive::output * list1[] = {base_x, base_y};
		jive::output * a = jive_bitconcat(2, list1);
		jive::output * b = jive_bitslice(a, 0, 8);
		
		assert(static_cast<jive::bits::output*>(b)->nbits() == 8);
		
		assert(b == base_x);
	}
	
	{
		/* concat flattening */
		jive::output * list1[] = {base_x, base_y};
		jive::output * a = jive_bitconcat(2, list1);
		
		jive::output * list2[] = {a, base_z};
		jive::output * b = jive_bitconcat(2, list2);
		
		assert(b->node()->class_ == &JIVE_BITCONCAT_NODE);
		assert(b->node()->ninputs == 3);
		assert(b->node()->inputs[0]->origin() == base_x);
		assert(b->node()->inputs[1]->origin() == base_y);
		assert(b->node()->inputs[2]->origin() == base_z);
	}
	
	{
		/* concat of single node */
		jive::output * a = jive_bitconcat(1, &base_x);
		
		assert(a==base_x);
	}
	
	{
		/* concat of slices */
		jive::output * a = jive_bitslice(base_x, 0, 4);
		jive::output * b = jive_bitslice(base_x, 4, 8);
		jive::output * list1[] = {a, b};
		jive::output * c = jive_bitconcat(2, list1);
		
		assert(c==base_x);
	}
	
	{
		/* concat of constants */
		jive::output * list1[] = {base_const1, base_const2};
		jive::output * a = jive_bitconcat(2, list1);
		
		assert(a->node()->class_ == &JIVE_BITCONSTANT_NODE);
		const jive::bits::constant_operation * attrs =
			(const jive::bits::constant_operation *) jive_node_get_attrs(a->node());
		assert(attrs->bits.size() == 16);
		assert(memcmp(&attrs->bits[0], "0011011111001000", 16) == 0);
	}
	
	{
		/* CSE */
		jive::output * a = jive_bitsymbolicconstant(graph, 8, "x");
		assert(a == base_x);
		
		jive::output * b = jive_bitconstant(graph, 8, "00110111");
		assert(b == base_const1);
		
		jive::output * c = jive_bitslice(base_x, 2, 6);
		jive::output * d = jive_bitslice(base_x, 2, 6);
		assert(c == d);
		
		jive::output * list1[] = {base_x, base_y};
		jive::output * e = jive_bitconcat(2, list1);
		jive::output * f = jive_bitconcat(2, list1);
		assert(e == f);
	}
	
	//jive_graph_view(graph);
	
	jive_context_destroy(ctx);
	
	return 0;
}



JIVE_UNIT_TEST_REGISTER("types/bitstring/test-slice-concat", test_main);
