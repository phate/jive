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

static jive::oport *
setup_fib_iter(jive::graph * graph)
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
	jive_lambda * lambda = jive_lambda_begin(graph->root(), {{&bits32, "n"}}, {{&bits32, "r"}});

	auto n = lambda->arguments[0];
	auto i = jive_bitconstant_unsigned(lambda->region, 32, 1);
	auto j = jive_bitconstant_unsigned(lambda->region, 32, 0);
	auto k = jive_bitconstant_unsigned(lambda->region, 32, 1);

	jive::theta_builder tb;
	auto theta_region = tb.begin(lambda->region);
	auto lv_i = tb.add_loopvar(i);
	auto lv_j = tb.add_loopvar(j);
	auto lv_k = tb.add_loopvar(k);
	auto lv_n = tb.add_loopvar(n);

	auto t = jive_bitsum(32, {lv_i->value(), lv_j->value()});

	auto one = jive_bitconstant_unsigned(theta_region, 32, 1);

	auto new_k = jive_bitsum(32, {one, lv_k->value()});

	auto cmp = jive_bitulesseq(new_k, lv_n->value());
	auto predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);

	lv_k->set_value(new_k);
	lv_i->set_value(lv_j->value());
	lv_j->set_value(t);
	lv_n->set_value(lv_n->value());
	tb.end(predicate);

	cmp = jive_bitulesseq(k, n);
	predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);
	auto results = jive_gamma(predicate, {&bits32, &bits32, &bits32, &bits32},
		{{i, j, k, n}, {lv_i->value(), lv_j->value(), lv_k->value(), lv_n->value()}});

	return jive_lambda_end(lambda, 1, &types[0], &results[1]);
}

static void
test_fib_iter(jive::graph * graph)
{
	using namespace jive::evaluator;

	auto fib_iter = setup_fib_iter(graph);
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

static jive::oport *
setup_fib_rec(jive::graph * &graph)
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
	jive::fct::type fcttype({&bits32}, {&bits32});

	jive_phi phi = jive_phi_begin(graph->root());
	jive_phi_fixvar fv_fib = jive_phi_fixvar_enter(phi, &fcttype);

	std::vector<const char*> names({"n"});
	jive_lambda * lambda = jive_lambda_begin(phi.region, {{&bits32, "n"}}, {{&bits32, "r"}});
	jive::fct::lambda_dep depvar = jive::fct::lambda_dep_add(lambda, fv_fib.value);

	auto n = lambda->arguments[0];
	auto one = jive_bitconstant_unsigned(lambda->region, 32, 1);
	auto two = jive_bitconstant_unsigned(lambda->region, 32, 2);

	auto tmp = jive_bitsub(32, n, one);
	tmp = jive_apply_create(depvar.output, 1, &tmp)[0];

	auto tmp2 = jive_bitsub(32, n, two);
	tmp2 = jive_apply_create(depvar.output, 1, &tmp2)[0];

	auto result = jive_bitsum(32, {tmp, tmp2});

	auto predicate = jive::ctl::match(1, {{0,0}}, 1, 2, jive_bituless(n, two));
	result = jive_gamma(predicate, {&bits32}, {{result}, {n}})[0];

	auto fib = jive_lambda_end(lambda, 1, &args[0], &result);

	jive_phi_fixvar_leave(phi, fv_fib.gate, fib);
	jive_phi_end(phi, 1, &fv_fib);

	return fv_fib.value;
}

static void
test_fib_rec(jive::graph * graph)
{
	using namespace jive::evaluator;

	auto fib_rec = setup_fib_rec(graph);
	graph->export_port(fib_rec, "fib_rec");

	jive::view(graph->root(), stdout);

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
test_loadstore(jive::graph * graph)
{
	using namespace jive::evaluator;

	jive::bits::type bits64(64), bits4(4);
	std::vector<const jive::base::type*> types({&jive::mem::type::instance(), &bits64});
	std::vector<const char *> names({"state", "address"});
	jive_lambda * lambda = jive_lambda_begin(graph->root(),
		{{&jive::mem::type::instance(), "state"}, {&bits64, "address"}},
		{{&jive::mem::type::instance(), "state"}});

	jive::oport * state = lambda->arguments[0];
	jive::oport * address = lambda->arguments[1];

	auto value = jive_load_by_bitstring_create(address, 64, &bits4, 1, &state);

	auto three = jive_bitconstant_unsigned(lambda->region, 4, 3);
	value = jive_bitsum(4, {value, three});

	state = jive_store_by_bitstring_create(address, 64, &bits4, value, 1, &state)[0];

	auto f = jive_lambda_end(lambda, 1, &types[0], &state);

	graph->export_port(f, "loadstore");

	jive::view(graph->root(), stdout);

	uint32_t v = 0xF05;

	memliteral s;
	bitliteral a(jive::bits::value_repr(64, (uint64_t)&v));
	eval(graph, "loadstore", {&s, &a});

	assert(v == 0xF08);
}

static int
test_evaluator()
{
	jive::graph graph;

	test_fib_iter(&graph);
	test_fib_rec(&graph);
	test_loadstore(&graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("evaluator/test-evaluator", test_evaluator);
