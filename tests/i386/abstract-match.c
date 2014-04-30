/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/codegen.h>
#include <jive/arch/label-mapper.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instrmatch.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/relocation.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/types/bitstring/arithmetic.h>
#include <jive/types/function/fctlambda.h>
#include <jive/view.h>
#include <jive/vsdg.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);

	jive_bitstring_type bits32(32);
	const jive_type * tmparray0[] = {&bits32, &bits32};
	const char * tmparray1[] = {"arg1", "arg2"};
	jive_lambda * lambda = jive_lambda_begin(graph,
		2, tmparray0, tmparray1);

	jive_output * sum = jive_bitsum(lambda->narguments, lambda->arguments);

	const jive_type * tmparray11[] = {&bits32};
	jive_node * abstract_fn = jive_lambda_end(lambda, 1, tmparray11, &sum)->node;
	
	jive_node * i386_fn = jive_i386_subroutine_convert(graph->root_region, abstract_fn);
	jive_graph_export(graph, i386_fn->outputs[0]);
	jive_graph_prune(graph);
	
	jive_view(graph, stdout);
	
	jive_regselector regselector;
	jive_regselector_init(&regselector, graph, &jive_i386_reg_classifier);
	jive_regselector_process(&regselector);
	jive_i386_match_instructions(graph, &regselector);
	jive_regselector_fini(&regselector);
	
	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_view(graph, stdout);
	
	jive_compilate compilate;
	jive_compilate_init(&compilate, ctx);
	jive_label_symbol_mapper * symbol_mapper = jive_label_symbol_mapper_simple_create(ctx);
	jive_graph_generate_code(graph, symbol_mapper, &compilate);
	jive_label_symbol_mapper_destroy(symbol_mapper);
	
	jive_compilate_map * map = jive_compilate_load(&compilate,
		NULL, jive_i386_process_relocation);
	void * result = jive_compilate_map_get_stdsection(map, jive_stdsectionid_code);
	jive_compilate_map_destroy(map);
	int (*function)(int, int) = (int(*)(int, int)) result;
	jive_compilate_fini(&compilate);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	int ret_value = function(18, 24);
	assert(ret_value == 42);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("i386/abstract-match", test_main);
