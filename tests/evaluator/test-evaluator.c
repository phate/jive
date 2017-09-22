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

	jive::lambda_builder lb;
	auto arguments = lb.begin(graph->root(), {{&bits32}, {&bits32}});

	auto n = arguments[0];
	auto i = jive_bitconstant_unsigned(lb.subregion(), 32, 1);
	auto j = jive_bitconstant_unsigned(lb.subregion(), 32, 0);
	auto k = jive_bitconstant_unsigned(lb.subregion(), 32, 1);

	jive::theta_builder tb;
	auto theta_region = tb.begin_theta(lb.subregion());
	auto lv_i = tb.add_loopvar(i);
	auto lv_j = tb.add_loopvar(j);
	auto lv_k = tb.add_loopvar(k);
	auto lv_n = tb.add_loopvar(n);

	auto t = jive::bits::create_add(32, lv_i->argument(), lv_j->argument());

	auto one = jive_bitconstant_unsigned(theta_region, 32, 1);

	auto new_k = jive::bits::create_add(32, one, lv_k->argument());

	auto cmp = jive_bitule(32, new_k, lv_n->argument());
	auto predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);

	lv_k->result()->divert_origin(new_k);
	lv_i->result()->divert_origin(lv_j->argument());
	lv_j->result()->divert_origin(t);
	lv_n->result()->divert_origin(lv_n->argument());
	tb.end_theta(predicate);

	cmp = jive_bitule(32, k, n);
	predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);

	jive::gamma_builder gb;
	gb.begin_gamma(predicate);
	auto evi = gb.add_entryvar(i);
	auto evj = gb.add_entryvar(j);
	auto evk = gb.add_entryvar(k);
	auto evn = gb.add_entryvar(n);
	auto evlvi = gb.add_entryvar(lv_i->output());
	auto evlvj = gb.add_entryvar(lv_j->output());
	auto evlvk = gb.add_entryvar(lv_k->output());
	auto evlvn = gb.add_entryvar(lv_n->output());

	auto exi = gb.add_exitvar({evi->argument(0), evlvi->argument(1)});
	auto exj = gb.add_exitvar({evj->argument(0), evlvj->argument(1)});
	auto exk = gb.add_exitvar({evk->argument(0), evlvk->argument(1)});
	auto exn = gb.add_exitvar({evn->argument(0), evlvn->argument(1)});
	auto gamma = gb.end_gamma();

	return lb.end({gamma->node()->output(1)})->node()->output(0);
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

static jive::output *
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

	jive::phi_builder pb;
	pb.begin(graph->root());
	auto rv = pb.add_recvar(fcttype);

	jive::lambda_builder lb;
	auto arguments = lb.begin(pb.region(), {{&bits32}, {&bits32}});
	auto dep = lb.add_dependency(rv->value());

	auto n = arguments[0];
	auto one = jive_bitconstant_unsigned(lb.subregion(), 32, 1);
	auto two = jive_bitconstant_unsigned(lb.subregion(), 32, 2);

	auto tmp = jive::bits::create_sub(32, n, one);
	tmp = jive::fct::create_apply(dep, {tmp})[0];

	auto tmp2 = jive::bits::create_sub(32, n, two);
	tmp2 = jive::fct::create_apply(dep, {tmp2})[0];

	auto result = jive::bits::create_add(32, tmp, tmp2);

	auto predicate = jive::ctl::match(1, {{0,0}}, 1, 2, jive_bitult(32, n, two));

	jive::gamma_builder gb;
	gb.begin_gamma(predicate);
	auto ev1 = gb.add_entryvar(result);
	auto ev2 = gb.add_entryvar(n);
	auto ex = gb.add_exitvar({ev1->argument(0), ev2->argument(1)});
	auto gamma = gb.end_gamma();

	auto fib = lb.end({gamma->node()->output(0)})->node()->output(0);
	rv->set_value(fib);
	pb.end();

	return rv->value();
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
	const auto & mem = jive::mem::type::instance();
	std::vector<const jive::base::type*> types({&jive::mem::type::instance(), &bits64});

	jive::lambda_builder lb;
	auto arguments = lb.begin(graph->root(), {{&mem, &bits64}, {&mem}});

	auto state = arguments[0];
	auto address = arguments[1];

	auto value = jive_load_by_bitstring_create(address, 64, &bits4, 1, &state);

	auto three = jive_bitconstant_unsigned(lb.subregion(), 4, 3);
	value = jive::bits::create_add(4, value, three);

	state = jive_store_by_bitstring_create(address, 64, &bits4, value, 1, &state)[0];

	auto f = lb.end({state})->node()->output(0);

	graph->export_port(f, "loadstore");

	jive::view(graph->root(), stdout);

	uint32_t v = 0xF05;

	memliteral s;
	bitliteral a(jive::bits::value_repr(64, (uint64_t)&v));
	eval(graph, "loadstore", {&s, &a});

	assert(v == 0xF08);
}

static void
test_external_function()
{
	using namespace jive::evaluator;

	jive::graph graph;

	jive::bits::type bits64(64);
	auto i = graph.import(bits64, "v");

	jive::lambda_builder lb;
	auto arguments = lb.begin(graph.root(), {{&bits64}, {&bits64}});
	auto v = lb.add_dependency(i);
	auto sum = jive::bits::create_add(64, arguments[0], v);
	auto lambda = lb.end({sum});

	graph.export_port(lambda->node()->output(0), "test");

	jive::view(graph.root(), stdout);

	bool exception_caught = false;
	try {
		uint32_t g = 42;
		bitliteral arg(jive::bits::value_repr(64, (uint64_t)&g));
		eval(&graph, "test", {&arg});
	} catch (jive::compiler_error e) {
		exception_caught = true;
	}

	assert(exception_caught);
}

static int
test_evaluator()
{
	jive::graph graph;

	test_fib_iter(&graph);
	test_fib_rec(&graph);
	test_loadstore(&graph);
	test_external_function();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("evaluator/test-evaluator", test_evaluator);
