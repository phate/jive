/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
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
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/equivalence.h>
#include <jive/vsdg/theta.h>

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
	
	jive_graph * gr1 = jive_graph_create();
	jive_graph_get_nodeclass_form(gr1, typeid(jive::operation))->set_mutable(false);
	
	jive::output * a = jive_bitconstant(gr1, 8, "01010101");
	jive::output * b = jive_bitconstant(gr1, 8, "10101010");
	jive::output * true_out = jive_control_true(gr1);
	jive::output * c = jive_gamma(true_out, {&a->type()}, {{a}, {b}})[0];
	
	jive_theta theta = jive_theta_begin(gr1);
	jive_theta_loopvar loopvar = jive_theta_loopvar_enter(theta, c);
	jive_theta_end(theta, jive_control_false(gr1), 1, &loopvar);
	jive::output * d = loopvar.value;
	
	jive::output * e = jive_bitnot(d);
	
	jive_node * orig_node = e->node();
	
	jive_serialization_driver drv;
	jive_serialization_driver_init(&drv);
	jive_serialization_symtab_insert_nodesym(&drv.symtab, orig_node, "TARGET");
	jive_token_ostream * os = jive_token_ostream_simple_create(&buf);
	jive_serialize_graph(&drv, gr1, os);
	jive_token_ostream_destroy(os);
	jive_serialization_driver_fini(&drv);
	fwrite(&buf.data[0], 1, buf.data.size(), stderr);
	
	jive_view(gr1, stdout);
	jive_graph * gr2 = jive_graph_create();
	jive_token_istream * is = jive_token_istream_simple_create(
		(char *)&buf.data[0], buf.data.size() + (char *) &buf.data[0]);
	jive_serialization_driver_init(&drv);
	drv.error = my_error;
	jive_deserialize_graph(&drv, is, gr2);
	jive_node * repl_node = jive_serialization_symtab_name_to_node(&drv.symtab, "TARGET")->node;
	assert(repl_node);
	jive_serialization_driver_fini(&drv);
	jive_token_istream_destroy(is);
	
	jive_view(gr2, stdout);
	
	jive_view(gr2, stdout);
	
	assert (jive_graphs_equivalent(gr1, gr2,
		1, &orig_node, &repl_node, 0, NULL, NULL));
	
	jive_graph_destroy(gr1);
	jive_graph_destroy(gr2);
	
	return 0;
}
JIVE_UNIT_TEST_REGISTER("serialization/test-region-serialization", test_main);
