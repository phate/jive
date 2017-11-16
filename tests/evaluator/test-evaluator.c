/*
 * Copyright 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/arch/addresstype.h>
#include <jive/arch/load.h>
#include <jive/arch/store.h>
#include <jive/evaluator/eval.h>
#include <jive/evaluator/literal.h>
#include <jive/rvsdg.h>
#include <jive/rvsdg/phi.h>
#include <jive/rvsdg/theta.h>
#include <jive/types/bitstring.h>
#include <jive/types/function.h>
#include <jive/view.h>

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

	jive::lambda_builder lb;
	auto arguments = lb.begin_lambda(graph->root(), {{&bits32}, {&bits32}});

	auto n = arguments[0];
	auto i = create_bitconstant(lb.subregion(), 32, 1);
	auto j = create_bitconstant(lb.subregion(), 32, 0);
	auto k = create_bitconstant(lb.subregion(), 32, 1);

	auto theta = jive::theta_node::create(lb.subregion());
	auto lv_i = theta->add_loopvar(i);
	auto lv_j = theta->add_loopvar(j);
	auto lv_k = theta->add_loopvar(k);
	auto lv_n = theta->add_loopvar(n);

	auto t = jive::bits::create_add(32, lv_i->argument(), lv_j->argument());

	auto one = create_bitconstant(theta->subregion(), 32, 1);

	auto new_k = jive::bits::create_add(32, one, lv_k->argument());

	auto cmp = jive::bits::create_ule(32, new_k, lv_n->argument());
	auto predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);

	lv_k->result()->divert_origin(new_k);
	lv_i->result()->divert_origin(lv_j->argument());
	lv_j->result()->divert_origin(t);
	lv_n->result()->divert_origin(lv_n->argument());
	theta->set_predicate(predicate);

	cmp = jive::bits::create_ule(32, k, n);
	predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);

	auto gamma = jive::gamma_node::create(predicate, 2);
	auto evi = gamma->add_entryvar(i);
	auto evj = gamma->add_entryvar(j);
	auto evk = gamma->add_entryvar(k);
	auto evn = gamma->add_entryvar(n);
	auto evlvi = gamma->add_entryvar(lv_i->output());
	auto evlvj = gamma->add_entryvar(lv_j->output());
	auto evlvk = gamma->add_entryvar(lv_k->output());
	auto evlvn = gamma->add_entryvar(lv_n->output());

	auto exi = gamma->add_exitvar({evi->argument(0), evlvi->argument(1)});
	auto exj = gamma->add_exitvar({evj->argument(0), evlvj->argument(1)});
	auto exk = gamma->add_exitvar({evk->argument(0), evlvk->argument(1)});
	auto exn = gamma->add_exitvar({evn->argument(0), evlvn->argument(1)});

	return lb.end_lambda({gamma->output(1)})->output(0);
}

static void
test_fib_iter(jive::graph * graph)
{
	using namespace jive::eval;

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
	std::vector<const jive::type*> args({&bits32});
	std::vector<const jive::type*> res({&bits32});
	jive::fct::type fcttype({&bits32}, {&bits32});

	jive::phi_builder pb;
	pb.begin_phi(graph->root());
	auto rv = pb.add_recvar(fcttype);

	jive::lambda_builder lb;
	auto arguments = lb.begin_lambda(pb.region(), {{&bits32}, {&bits32}});
	auto dep = lb.add_dependency(rv->value());

	auto n = arguments[0];
	auto one = create_bitconstant(lb.subregion(), 32, 1);
	auto two = create_bitconstant(lb.subregion(), 32, 2);

	auto tmp = jive::bits::create_sub(32, n, one);
	tmp = jive::fct::create_apply(dep, {tmp})[0];

	auto tmp2 = jive::bits::create_sub(32, n, two);
	tmp2 = jive::fct::create_apply(dep, {tmp2})[0];

	auto result = jive::bits::create_add(32, tmp, tmp2);

	auto cmp = jive::bits::create_ult(32, n, two);
	auto predicate = jive::ctl::match(1, {{0,0}}, 1, 2, cmp);

	auto gamma = jive::gamma_node::create(predicate, 2);
	auto ev1 = gamma->add_entryvar(result);
	auto ev2 = gamma->add_entryvar(n);
	auto ex = gamma->add_exitvar({ev1->argument(0), ev2->argument(1)});

	auto fib = lb.end_lambda({gamma->output(0)})->output(0);
	rv->set_value(fib);
	pb.end_phi();

	return rv->value();
}

static void
test_fib_rec(jive::graph * graph)
{
	using namespace jive::eval;

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
	using namespace jive::eval;

	jive::bits::type bits64(64), bits4(4);
	const auto & mem = jive::memtype::instance();

	jive::lambda_builder lb;
	auto arguments = lb.begin_lambda(graph->root(), {{&mem, &bits64}, {&mem}});

	jive::output * state = arguments[0];
	auto address = arguments[1];

	auto value = jive_load_by_bitstring_create(address, 64, &bits4, 1, &state);

	auto three = create_bitconstant(lb.subregion(), 4, 3);
	value = jive::bits::create_add(4, value, three);

	state = jive_store_by_bitstring_create(address, 64, &bits4, value, 1, &state)[0];

	auto f = lb.end_lambda({state})->output(0);

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
	using namespace jive::eval;

	jive::graph graph;
	graph.node_normal_form(typeid(jive::operation))->set_mutable(false);

	jive::bits::type bits64(64);
	auto i = graph.import(bits64, "v");

	jive::lambda_builder lb;
	auto arguments = lb.begin_lambda(graph.root(), {{&bits64}, {&bits64}});
	auto v = lb.add_dependency(i);
	auto sum = jive::bits::create_add(64, arguments[0], v);
	auto lambda = lb.end_lambda({sum});

	graph.export_port(lambda->output(0), "test");

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
	graph.node_normal_form(typeid(jive::operation))->set_mutable(false);

	test_fib_iter(&graph);
	test_fib_rec(&graph);
	test_loadstore(&graph);
	test_external_function();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("evaluator/test-evaluator", test_evaluator);
