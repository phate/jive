#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/types/float.h>
#include <jive/types/float/fltconstant.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>


static int
types_float_arithmetic_test_fltdifference(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * sub = jive_fltdifference(s0, s1);

	jive_graph_export(&graph, sub);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltdifference", types_float_arithmetic_test_fltdifference)

static int
types_float_arithmetic_test_fltnegate(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * neg = jive_fltnegate(s0);

	jive_graph_export(&graph, neg);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltnegate", types_float_arithmetic_test_fltnegate)


static int types_float_arithmetic_test_fltproduct(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * mul = jive_fltproduct(s0, s1);

	jive_graph_export(&graph, mul);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltproduct", types_float_arithmetic_test_fltproduct)

static int types_float_arithmetic_test_fltquotient(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * div = jive_fltquotient(s0, s1);

	jive_graph_export(&graph, div);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltquotient", types_float_arithmetic_test_fltquotient)

static int types_float_arithmetic_test_fltsum(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * add = jive_fltsum(s0, s1);

	jive_graph_export(&graph, add);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/arithmetic/test-fltsum", types_float_arithmetic_test_fltsum)

static int types_float_comparison_test_fltequal(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * equal = jive_fltequal(s0, s1);

	jive_graph_export(&graph, equal);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltequal", types_float_comparison_test_fltequal)

static int types_float_comparison_test_fltgreater(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * greater = jive_fltgreater(s0, s1);

	jive_graph_export(&graph, greater);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltgreater", types_float_comparison_test_fltgreater)

static int types_float_comparison_test_fltgreatereq(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * geq = jive_fltgreatereq(s0, s1);

	jive_graph_export(&graph, geq);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltgreatereq", types_float_comparison_test_fltgreatereq)

static int types_float_comparison_test_fltless(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * less = jive_fltless(s0, s1);

	jive_graph_export(&graph, less);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltless", types_float_comparison_test_fltless)

static int types_float_comparison_test_fltlesseq(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * lesseq = jive_fltlesseq(s0, s1);

	jive_graph_export(&graph, lesseq);

	graph.normalize();
	jive_graph_prune(&graph);
	jive_view(&graph, stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/float/comparison/test-fltlesseq", types_float_comparison_test_fltlesseq)

static int
types_float_comparison_test_fltnotequal(void)
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive::output * s0 = jive_fltsymbolicconstant(&graph, "s0");
	jive::output * s1 = jive_fltsymbolicconstant(&graph, "s1");
	jive::output * neq = jive_fltnotequal(s0, s1);

	jive_graph_export(&graph, neq);

	graph.normalize();
	jive_graph_prune(&graph);
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
