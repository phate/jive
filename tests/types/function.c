/*
 * Copyright 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/types/bitstring.h>
#include <jive/types/function.h>
#include <jive/types/function/fctapply.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fcttype.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>

static int function_test_build_lambda(void)
{
	jive_graph graph;

	jive::bits::type bits32(32);
	const jive::base::type * tmparray0[] = {&bits32, &bits32};
	jive_lambda * lambda = jive_lambda_begin(graph.root(),
		{{&bits32, "arg1"}, {&bits32, "arg1"}}, {{&bits32, "r"}});

	auto sum = jive_bitsum(
		std::vector<jive::oport*>(lambda->arguments, lambda->arguments+lambda->narguments));

	auto fct = jive_lambda_end(lambda, 1, tmparray0, &sum);
	
	jive::view(graph.root(), stderr);
	
	const jive::base::type * tmparray2[] = {&bits32, &bits32};
	jive::fct::type ftype(2, tmparray2, 1, tmparray2);

	assert(ftype == fct->type());
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-build-lambda", function_test_build_lambda);

static int function_test_call(void)
{
	jive_graph graph;

	jive::bits::type btype(8);
	const jive::base::type*  tmparray0[] = { &btype };
	const jive::base::type*  tmparray1[] = { &btype };
	jive::fct::type ftype(1, tmparray0, 1, tmparray1) ;

	auto constant = jive_bitconstant(graph.root(), 8, "00001111" ) ;
	auto func = graph.import(ftype, "sin");
	jive::oport *  tmparray2[] = { constant };
	auto ret = jive_apply_create(func, 1, tmparray2)[0];

	assert(ret->type() == btype);

	jive::view(graph.root(), stderr) ;

	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-call", function_test_call);

static int function_test_equals(void)
{
	jive::bits::type btype0(8);
	jive::bits::type btype1(8);
	const jive::base::type*  tmparray0[] = { &btype0 };
	const jive::base::type*  tmparray1[] = { &btype0 };

	jive::fct::type type0(1, tmparray0, 1, tmparray1);

	const jive::base::type*  tmparray2[] = { &btype0 };
	const jive::base::type*  tmparray3[] = { &btype1 };
	jive::fct::type type1(1, tmparray2, 1, tmparray3);

	const jive::base::type*  tmparray4[] = { &btype0 };
	const jive::base::type*  tmparray5[] = { &btype1, &btype1 };
	jive::fct::type type2(1, tmparray4, 2, tmparray5);

	const jive::base::type*  tmparray6[] = { &btype0, &btype0 };
	const jive::base::type*  tmparray7[] = { &btype0 };
	jive::fct::type type3(2, tmparray6, 1, tmparray7);

	assert(type0 == type0);
	assert(type0 == type1);
	assert(type0 != type2);
	assert(type0 != type3);
	
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-equals", function_test_equals);

static int function_test_memory_leak(void)
{
	jive_test_value_type value_type;
	const jive::base::type * value_type_ptr = &value_type;
	jive::fct::type t1(1, &value_type_ptr, 1, &value_type_ptr);

	const jive::base::type * tmparray2[] = {&t1};
	const jive::base::type * tmparray3[] = {&t1};
	jive::fct::type t2(1, tmparray2, 1, tmparray3);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-memory-leak", function_test_memory_leak);
