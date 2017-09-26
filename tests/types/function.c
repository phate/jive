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
	jive::graph graph;
	jive::bits::type bits32(32);

	jive::lambda_builder lb;
	auto arguments = lb.begin_lambda(graph.root(), {{&bits32, &bits32}, {&bits32}});

	auto sum = jive::bits::create_add(32, arguments[0], arguments[1]);

	auto fct = lb.end_lambda({sum})->node()->output(0);

	jive::view(graph.root(), stderr);
	
	jive::fct::type ftype({&bits32, &bits32}, {&bits32});

	assert(ftype == fct->type());
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-build-lambda", function_test_build_lambda);

static int function_test_call(void)
{
	jive::graph graph;

	jive::bits::type btype(8);
	jive::fct::type ftype({&btype}, {&btype}) ;

	auto constant = create_bitconstant(graph.root(), "00001111");
	auto func = graph.import(ftype, "sin");
	auto ret = jive::fct::create_apply(func, {constant})[0];

	assert(ret->type() == btype);

	jive::view(graph.root(), stderr) ;

	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-call", function_test_call);

static int function_test_equals(void)
{
	jive::bits::type btype0(8);
	jive::bits::type btype1(8);
	jive::fct::type type0({&btype0}, {&btype0});
	jive::fct::type type1({&btype0}, {&btype1});
	jive::fct::type type2({&btype0}, {&btype1, &btype1});
	jive::fct::type type3({&btype0, &btype0}, {&btype0});

	assert(type0 == type0);
	assert(type0 == type1);
	assert(type0 != type2);
	assert(type0 != type3);
	
	return 0 ;
}

JIVE_UNIT_TEST_REGISTER("function/test-equals", function_test_equals);

static int function_test_memory_leak(void)
{
	jive::test::valuetype value_type;
	jive::fct::type t1({&value_type}, {&value_type});
	jive::fct::type t2({&t1}, {&t1});
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-memory-leak", function_test_memory_leak);
