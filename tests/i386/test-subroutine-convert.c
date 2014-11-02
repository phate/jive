/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <locale.h>
#include <assert.h>

#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/context.h>
#include <jive/types/bitstring.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create();

	jive::addr::type addr;
	const jive::base::type * addrptr = &addr;
	jive::bits::type bits32(32);
	const char * tmparray0[] = {"arg"};
	jive_lambda * lambda = jive_lambda_begin(graph, 1, &addrptr, tmparray0);

	jive::output * constant = jive_bitconstant_unsigned(graph, 32, 2);
	jive::output * address = jive_arraysubscript(lambda->arguments[0], &bits32, constant);

	jive_node * lambda_node = jive_lambda_end(lambda, 1, &addrptr, &address)->node();
	jive_graph_export(graph, lambda_node->outputs[0]);

	jive_i386_subroutine_convert(graph->root_region, lambda_node);

	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("i386/test-subroutine-convert", test_main);
