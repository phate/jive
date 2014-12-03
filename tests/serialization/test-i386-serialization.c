/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/serialization/driver.h>
#include <jive/serialization/token-stream.h>
#include <jive/util/buffer.h>
#include <jive/view.h>
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
	
	jive_graph * gr1 = jive_graph_create();
	const jive_argument_type tmparray0[] = { jive_argument_int };
	const jive_argument_type tmparray1[] = { jive_argument_int };
	
	jive_subroutine sub = jive_i386_subroutine_begin(
		gr1,
		1,tmparray0,
		1,tmparray1);

	jive_subroutine_simple_set_result(sub, 0,
		jive_subroutine_simple_get_argument(sub, 0));

	jive_subroutine_end(sub);
	
	/* inhibit implicit normalization */
	jive_graph_get_nodeclass_form(gr1, typeid(jive::operation))->set_mutable(false);
	int64_t tmparray2[] = {42};
	
	jive_node * n1 = jive_instruction_node_create_simple(
		gr1->root_region,
		&jive_i386_instr_int_load_imm,
		NULL,
		tmparray2);
	jive::output * tmparray3[] = {n1->outputs[0]};
	int64_t tmparray4[] = {17};
	
	jive_node * n2 = jive_instruction_node_create_simple(
		gr1->root_region,
		&jive_i386_instr_int_load32_disp,
		tmparray3,
		tmparray4);
	jive::output * tmparray5[] = {n1->outputs[0], n2->outputs[0]};
	
	jive_node * n3 = jive_instruction_node_create_simple(
		gr1->root_region,
		&jive_i386_instr_int_mul_expand_signed,
		tmparray5,
		NULL);
	
	jive_node * orig_node = n3;
	
	jive_view(gr1, stdout);
	
	jive_serialization_driver drv;
	jive_serialization_driver_init(&drv);
	jive_serialization_symtab_insert_nodesym(&drv.symtab, orig_node, "TARGET");
	jive_token_ostream * os = jive_token_ostream_simple_create(&buf);
	jive_serialize_graph(&drv, gr1, os);
	jive_token_ostream_destroy(os);
	jive_serialization_driver_fini(&drv);
	fwrite(&buf.data[0], 1, buf.data.size(), stderr);
	
	jive_graph * gr2 = jive_graph_create();
	jive_token_istream * is = jive_token_istream_simple_create(
		(char *)&buf.data[0], buf.data.size() + (char *) &buf.data[0]);
	jive_serialization_driver_init(&drv);
	drv.error = my_error;
	jive_deserialize_graph(&drv, is, gr2);
	jive_node * repl_node = jive_serialization_symtab_name_to_node(&drv.symtab, "TARGET")->node;
	jive_serialization_driver_fini(&drv);
	jive_token_istream_destroy(is);
	
	assert(repl_node);
	
	assert (jive_graphs_equivalent(gr1, gr2,
		1, &orig_node, &repl_node, 0, NULL, NULL));
	
	jive_view(gr1, stdout);
	jive_view(gr2, stdout);
	
	jive_graph_destroy(gr1);
	jive_graph_destroy(gr2);
	
	return 0;
}
JIVE_UNIT_TEST_REGISTER("serialization/test-i386-serialization", test_main);
