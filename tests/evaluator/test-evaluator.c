/*
 * Copyright 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/arch/load.h>
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/evaluator/eval.h>
#include <jive/evaluator/literal.h>
#include <jive/types/bitstring.h>
#include <jive/types/function.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>
#include <jive/vsdg/theta.h>

static jive::output *
setup_fib_iter(struct jive_graph * graph)
{
/*
size_t
fib(size_t n)
{
  size_t i = 1, j = 0;
  for (size_t k = 1; k <= n; k++) {
    size_t t = i + j;
    i = j;
    j = t;
  }
  return j;
}
*/
	jive::bits::type bits32(32);
	std::vector<const jive::base::type*> types({&bits32});
	std::vector<const char *> names({"n"});
	jive_lambda * lambda = jive_lambda_begin(graph->root(), 1, &types[0], &names[0]);

	jive::output * n = lambda->arguments[0];
	jive::output * i = jive_bitconstant_unsigned(lambda->region, 32, 1);
	jive::output * j = jive_bitconstant_unsigned(lambda->region, 32, 0);
	jive::output * k = jive_bitconstant_unsigned(lambda->region, 32, 1);

	jive_theta theta = jive_theta_begin(lambda->region);
	jive_theta_loopvar lv_i = jive_theta_loopvar_enter(theta, i);
	jive_theta_loopvar lv_j = jive_theta_loopvar_enter(theta, j);
	jive_theta_loopvar lv_k = jive_theta_loopvar_enter(theta, k);
	jive_theta_loopvar lv_n = jive_theta_loopvar_enter(theta, n);

	jive::output * t = jive_bitsum({lv_i.value, lv_j.value});

	jive::output * one = jive_bitconstant_unsigned(theta.region, 32, 1);

	jive::output * new_k = jive_bitsum({one, lv_k.value});

	jive::output * cmp = jive_bitulesseq(new_k, lv_n.value);
	jive::output * predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);
	jive_theta_loopvar_leave(theta, lv_k.gate, new_k);
	jive_theta_loopvar_leave(theta, lv_i.gate, lv_j.value);
	jive_theta_loopvar_leave(theta, lv_j.gate, t);
	jive_theta_loopvar_leave(theta, lv_n.gate, lv_n.value);
	std::vector<jive_theta_loopvar> loopvars({lv_i, lv_j, lv_k, lv_n});
	jive_theta_end(theta, predicate, 4, &loopvars[0]);

	cmp = jive_bitulesseq(k, n);
	predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);
	std::vector<jive::output *> results = jive_gamma(predicate, {&bits32, &bits32, &bits32, &bits32},
		{{i, j, k, n}, {loopvars[0].value, loopvars[1].value, loopvars[2].value, loopvars[3].value}});

	return jive_lambda_end(lambda, 1, &types[0], &results[1]);
}

static void
test_fib_iter(struct jive_graph * graph)
{
	using namespace jive::evaluator;

	jive::output * fib_iter = setup_fib_iter(graph);
	graph->export_port(fib_iter, "fib_iter");

	std::unique_ptr<const literal> result;

	/* test fib(0) */
	bitliteral arg(jive::bits::value_repr(32, 0));
	result = std::move(eval(graph, "fib_iter", {&arg})->copy());
	const fctliteral * fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	const bitliteral * fib;
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 0);

	/* test fib(1) */
	arg = bitliteral(jive::bits::value_repr(32, 1));
	result = std::move(eval(graph, "fib_iter", {&arg})->copy());
	fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 1);

	/* test fib(2) */
	arg = bitliteral(jive::bits::value_repr(32, 2));
	result = std::move(eval(graph, "fib_iter", {&arg})->copy());
	fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 1);

	/* test fib(11) */
	arg = bitliteral(jive::bits::value_repr(32, 11));
	result = std::move(eval(graph, "fib_iter", {&arg})->copy());
	fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 89);
}

static jive::output *
setup_fib_rec(struct jive_graph * &graph)
{
/*
unsigned int fib(unsigned int n){
   if (n < 2)
     return n;
   else
     return fib(n - 1) + fib(n - 2);
 }
*/
	jive::bits::type bits32(32);
	std::vector<const jive::base::type*> args({&bits32});
	std::vector<const jive::base::type*> res({&bits32});
	jive::fct::type fcttype(args.size(), &args[0], res.size(), &res[0]);

	jive_phi phi = jive_phi_begin(graph->root());
	jive_phi_fixvar fv_fib = jive_phi_fixvar_enter(phi, &fcttype);

	std::vector<const char*> names({"n"});
	jive_lambda * lambda = jive_lambda_begin(phi.region, 1, &args[0], &names[0]);
	jive::fct::lambda_dep depvar = jive::fct::lambda_dep_add(lambda, fv_fib.value);

	jive::output * n = lambda->arguments[0];
	jive::output * one = jive_bitconstant_unsigned(lambda->region, 32, 1);
	jive::output * two = jive_bitconstant_unsigned(lambda->region, 32, 2);

	jive::output * tmp = jive_bitdifference(n, one);
	tmp = jive_apply_create(depvar.output, 1, &tmp)[0];

	jive::output * tmp2 = jive_bitdifference(n, two);
	tmp2 = jive_apply_create(depvar.output, 1, &tmp2)[0];

	jive::output * result = jive_bitsum({tmp, tmp2});

	jive::output * predicate = jive::ctl::match(1, {{0,0}}, 1, 2, jive_bituless(n, two));
	result = jive_gamma(predicate, {&bits32}, {{result}, {n}})[0];

	jive::output * fib = jive_lambda_end(lambda, 1, &args[0], &result);

	jive_phi_fixvar_leave(phi, fv_fib.gate, fib);
	jive_phi_end(phi, 1, &fv_fib);

	return fv_fib.value;
}

static void
test_fib_rec(struct jive_graph * graph)
{
	using namespace jive::evaluator;

	jive::output * fib_rec = setup_fib_rec(graph);
	graph->export_port(fib_rec, "fib_rec");

	jive_view(graph, stdout);

	std::unique_ptr<const literal> result;

	/* test fib(0) */
	bitliteral arg(jive::bits::value_repr(32, 0));
	result = std::move(eval(graph, "fib_rec", {&arg})->copy());
	const fctliteral * fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	const bitliteral * fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 0);

	/* test fib(1) */
	arg = bitliteral(jive::bits::value_repr(32, 1));
	result = std::move(eval(graph, "fib_rec", {&arg})->copy());
	fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 1);

	/* test fib(2) */
	arg = bitliteral(jive::bits::value_repr(32, 2));
	result = std::move(eval(graph, "fib_rec", {&arg})->copy());
	fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 1);

	/* test fib(8) */
	arg = bitliteral(jive::bits::value_repr(32, 8));
	result = std::move(eval(graph, "fib_rec", {&arg})->copy());
	fctlit = dynamic_cast<const fctliteral*>(result.get());
	assert(fctlit->nresults() == 1);
	fib = dynamic_cast<const bitliteral*>(&fctlit->result(0));
	assert(fib->value_repr() == 21);
}

static void
test_loadstore(struct jive_graph * graph)
{
	using namespace jive::evaluator;

	jive::bits::type bits64(64), bits4(4);
	std::vector<const jive::base::type*> types({&jive::mem::type::instance(), &bits64});
	std::vector<const char *> names({"state", "address"});
	jive_lambda * lambda = jive_lambda_begin(graph->root(), 2, &types[0], &names[0]);

	jive::output * state = lambda->arguments[0];
	jive::output * address = lambda->arguments[1];

	jive::output * value = jive_load_by_bitstring_create(address, 64, &bits4, 1, &state);

	jive::output * three = jive_bitconstant_unsigned(lambda->region, 4, 3);
	value = jive_bitsum({value, three});

	state = jive_store_by_bitstring_create(address, 64, &bits4, value, 1, &state)[0];

	jive::output * f = jive_lambda_end(lambda, 1, &types[0], &state);

	graph->export_port(f, "loadstore");

	jive_view(graph, stdout);

	uint32_t v = 0xF05;

	memliteral s;
	bitliteral a(jive::bits::value_repr(64, (uint64_t)&v));
	eval(graph, "loadstore", {&s, &a});

	assert(v == 0xF08);
}

static int
test_evaluator()
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	test_fib_iter(&graph);
	test_fib_rec(&graph);
	test_loadstore(&graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("evaluator/test-evaluator", test_evaluator);
