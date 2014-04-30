/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/types/bitstring.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fctsymbolic.h>
#include <jive/view.h>

static int test_main(void)
{
	setlocale( LC_ALL, "" ) ;

	jive_context* context = jive_context_create() ;
	jive_graph* graph = jive_graph_create( context ) ; 

	jive_bitstring_type btype(8);
	const jive_type*  tmparray0[] = { &btype };
	const jive_type*  tmparray1[] = { &btype };
	jive_function_type ftype(1, tmparray0, 1, tmparray1) ;

	jive_output* constant = jive_bitconstant( graph, 8, "00001111" ) ;
	jive_output* func = jive_symbolicfunction_create( graph, "sin", &ftype ) ;
	jive_output*  tmparray2[] = { constant };
	jive_node* apply = jive_apply_node_create( graph->root_region, func, 1, tmparray2 ) ;
	assert(apply->noutputs == 1);
	jive_output* ret = apply->outputs[0];

	const jive_type * ret_type = &ret->type();
	assert(jive_type_equals(ret_type, &btype));

	jive_view( graph, stderr ) ;

	jive_graph_destroy( graph ) ;
 
	assert( jive_context_is_empty(context) ) ;
	jive_context_destroy(context) ;

	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-call", test_main);
