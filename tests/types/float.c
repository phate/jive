#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/float.h>
#include <jive/types/float/fltconstant.h>
#include <jive/view.h>
#include <jive/vsdg.h>


static int
types_float_arithmetic_test_fltdifference(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto sub = jive_fltdifference(s0, s1);

	graph.export_port(sub, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltdifference", types_float_arithmetic_test_fltdifference)

static int
types_float_arithmetic_test_fltnegate(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto neg = jive_fltnegate(s0);

	graph.export_port(neg, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltnegate", types_float_arithmetic_test_fltnegate)


static int types_float_arithmetic_test_fltproduct(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto mul = jive_fltproduct(s0, s1);

	graph.export_port(mul, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltproduct", types_float_arithmetic_test_fltproduct)

static int types_float_arithmetic_test_fltquotient(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto div = jive_fltquotient(s0, s1);

	graph.export_port(div, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltquotient", types_float_arithmetic_test_fltquotient)

static int types_float_arithmetic_test_fltsum(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto add = jive_fltsum(s0, s1);

	graph.export_port(add, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltsum", types_float_arithmetic_test_fltsum)

static int types_float_comparison_test_fltequal(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto equal = jive_fltequal(s0, s1);

	graph.export_port(equal, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltequal", types_float_comparison_test_fltequal)

static int types_float_comparison_test_fltgreater(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto greater = jive_fltgreater(s0, s1);

	graph.export_port(greater, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltgreater", types_float_comparison_test_fltgreater)

static int types_float_comparison_test_fltgreatereq(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto geq = jive_fltgreatereq(s0, s1);

	graph.export_port(geq, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltgreatereq", types_float_comparison_test_fltgreatereq)

static int types_float_comparison_test_fltless(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto less = jive_fltless(s0, s1);

	graph.export_port(less, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltless", types_float_comparison_test_fltless)

static int types_float_comparison_test_fltlesseq(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto lesseq = jive_fltlesseq(s0, s1);

	graph.export_port(lesseq, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltlesseq", types_float_comparison_test_fltlesseq)

static int
types_float_comparison_test_fltnotequal(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	auto s0 = jive_fltsymbolicconstant(&graph, "s0");
	auto s1 = jive_fltsymbolicconstant(&graph, "s1");
	auto neq = jive_fltnotequal(s0, s1);

	graph.export_port(neq, "dummy");

	graph.normalize();
	graph.prune();
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltnotequal", types_float_comparison_test_fltnotequal)

static int
types_float_test_fltconstant(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive_fltconstant_float(graph.root(), -1.0);
	jive_fltconstant_float(graph.root(), 0.0);
	jive_fltconstant_float(graph.root(), 1.0);
	jive_fltconstant_float(graph.root(), 0.0 / 0.0);
	jive_fltconstant_float(graph.root(), 1.0 / 0.0);

	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/test-fltconstant", types_float_test_fltconstant);
