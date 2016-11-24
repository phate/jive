/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/serialization/driver.h>
#include <jive/serialization/token-stream.h>
#include <jive/types/bitstring.h>
#include <jive/util/buffer.h>
#include <jive/vsdg.h>
#include <jive/vsdg/equivalence.h>

static void
my_error(jive_serialization_driver * drv, const char msg[])
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_buffer buf;
	
	/* inhibit implicit normalization */
	jive_graph gr1;
	gr1.node_normal_form(typeid(jive::operation))->set_mutable(false);

	auto a = jive_bitconstant(gr1.root(), 8, "01010101");
	auto b = jive_bitslice(a, 2, 6);
	auto c = jive_bitconcat({b, b});
	auto d = jive_bitsum({a, c});
	auto e = jive_bitdifference(a, c);
	auto f = jive_bitand({d, e});
	auto g = jive_bitor({d, e});
	auto h = jive_bitxor({f, g});
	auto i = jive_bitmultiply({f, g});
	auto j = jive_bituhiproduct(h, i);
	auto k = jive_bitshiproduct(h, i);
	auto l = jive_bituquotient(j, k);
	auto m = jive_bitsquotient(j, k);
	auto n = jive_bitumod(l, m);
	auto o = jive_bitsmod(l, m);
	auto p = jive_bitshl(n, o);
	auto q = jive_bitshr(n, o);
	auto r = jive_bitnot(p);
	auto s = jive_bitnegate(q);
	auto t = jive_bitashr(r, s);
	
	jive::node * orig_node = dynamic_cast<jive::output*>(t)->node();

	jive_serialization_driver drv;
	jive_serialization_driver_init(&drv);
	jive_serialization_symtab_insert_nodesym(&drv.symtab, orig_node, "TARGET");
	jive_token_ostream * os = jive_token_ostream_simple_create(&buf);
	jive_serialize_graph(&drv, &gr1, os);
	jive_token_ostream_destroy(os);
	jive_serialization_driver_fini(&drv);
	fwrite(&buf.data[0], 1, buf.data.size(), stderr);
	
	jive_graph gr2;
	jive_token_istream * is = jive_token_istream_simple_create(
		(char *)&buf.data[0], buf.data.size() + (char *) &buf.data[0]);
	jive_serialization_driver_init(&drv);
	drv.error = my_error;
	jive_deserialize_graph(&drv, is, &gr2);
	jive::node * repl_node = jive_serialization_symtab_name_to_node(&drv.symtab, "TARGET")->node;
	jive_serialization_driver_fini(&drv);
	jive_token_istream_destroy(is);
	
	assert(repl_node);
	
	assert (jive_graphs_equivalent(&gr1, &gr2,
		1, &orig_node, &repl_node, 0, NULL, NULL));

	return 0;
}
JIVE_UNIT_TEST_REGISTER("serialization/test-bitstring-serialization", test_main);
