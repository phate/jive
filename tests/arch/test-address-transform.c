/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/call.h>
#include <jive/arch/linker-symbol.h>
#include <jive/arch/load.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/store.h>
#include <jive/rvsdg.h>
#include <jive/rvsdg/label.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>

#include "testnodes.h"

static int
test_address_transform(void)
{
	using namespace jive;

	memtype mt;
	addrtype at;

	jive::graph graph;
	auto i0 = graph.add_import(bit64, "");
	auto i1 = graph.add_import(bit64, "");
	auto i2 = graph.add_import(mt, "");

	auto address0 = bit2addr_op::create(i0, 64, at);
	auto address1 = bit2addr_op::create(i1, 64, at);

	auto dcl = rcddeclaration::create(&graph, {&at, &at});

	auto memberof = memberof_op::create(address0, dcl, 0);
	auto containerof = containerof_op::create(address1, dcl, 1);

	jive_linker_symbol write_symbol;
	jive::external_label write_label("write", &write_symbol);
	auto label = lbl2addr_op::create(graph.root(), &write_label);
	auto call = addrcall_op::create(label, {memberof, containerof}, {&at, &at}, nullptr);

	auto constant = create_bitconstant(graph.root(), 64, 1);
	auto arraysub = arraysubscript_op::create(call[0], at, constant);

	auto arrayindex = arrayindex_op::create(call[0], call[1], at, bit64);

	auto load = addrload_op::create(arraysub, at, {i2});
	auto store = addrstore_op::create(arraysub, arrayindex, bit64, {i2})[0]->node();

	auto o_addr = addr2bit_op::create(load, 64, load->type());

	graph.add_export(o_addr, "");
	graph.add_export(store->output(0), "");

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple mapper(8);
	transform_address(&graph, mapper);

	graph.prune();
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform", test_address_transform)

static int
test_address_transform_nodes(void)
{
	using namespace jive;

	addrtype at;

	jive::graph graph;
	auto i0 = graph.add_import(at, "i0");
	auto i1 = graph.add_import(bit32, "i1");
	auto i2 = graph.add_import(bit64, "i2");

	auto b0 = addr2bit_op::create(i0, 32, i0->type());
	auto a0 = bit2addr_op::create(b0, 32, at);

	auto a1 = bit2addr_op::create(i1, 32, at);
	auto b1 = addr2bit_op::create(a1, 32, at);

	auto x0 = graph.add_export(a0, "x0");
	auto x1 = graph.add_export(b1, "x1");

	assert(x0->origin() == i0);
	assert(x1->origin() == i1);

	auto b2 = bit2addr_op::create(i1, 32, at);
	auto b3 = bit2addr_op::create(i1, 32, at);
	auto a2 = addr2bit_op::create(i0, 32, i0->type());
	auto a3 = addr2bit_op::create(i0, 32, i0->type());

	assert(a2->node()->operation() == a3->node()->operation());
	assert(b2->node()->operation() == b3->node()->operation());

	auto b4 = bit2addr_op::create(i2, 64, at);
	auto a4 = addr2bit_op::create(i0, 64, at);

	assert(a2->node()->operation() != a4->node()->operation());
	assert(b2->node()->operation() != b4->node()->operation());

	jive::view(graph.root(), stderr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes",
	test_address_transform_nodes)

static int
test_apply_transform(void)
{
	using namespace jive;

	addrtype at;
	fct::type ft({&at}, {&at});

	jive::graph graph;
	auto i0 = graph.add_import(ft, "");
	auto i1 = graph.add_import(at, "");

	auto results = jive::fct::create_apply(i0, {i1});

	auto x0 = graph.add_export(results[0], "");

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(results[0]->node(), mapper);

	jive::view(graph.root(), stdout);

	assert(is_bit2addr_node(x0->origin()->node()));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-apply_transform", test_apply_transform)

static int
test_containerof_transform(void)
{
	using namespace jive;

	jive::graph graph;

	jive::addrtype addrtype;
	bittype bits8(8);
	bittype bits16(16);
	bittype bits32(32);
	auto dcl = rcddeclaration::create(&graph, {&bits8, &bits16, &bits32, &bits32});

	auto top = jive::test::simple_node_create(graph.root(), {}, {},
		std::vector<jive::port>(4, bits32));

	auto address0 = bit2addr_op::create(top->output(0), 32, addrtype);
	auto address1 = bit2addr_op::create(top->output(1), 32, addrtype);
	auto address2 = bit2addr_op::create(top->output(2), 32, addrtype);
	auto address3 = bit2addr_op::create(top->output(3), 32, addrtype);

	auto container0 = containerof_op::create(address0, dcl, 0);
	auto container1 = containerof_op::create(address1, dcl, 1);
	auto container2 = containerof_op::create(address2, dcl, 2);
	auto container3 = containerof_op::create(address3, dcl, 3);

	auto offset0 = addr2bit_op::create(container0, 32, container0->type());
	auto offset1 = addr2bit_op::create(container1, 32, container1->type());
	auto offset2 = addr2bit_op::create(container2, 32, container2->type());
	auto offset3 = addr2bit_op::create(container3, 32, container3->type());

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<jive::port>(4, bits32), {offset0, offset1, offset2, offset3}, {bits32});
	graph.add_export(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(&graph, mapper);

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->output(i)->type()));
		}
	}

	auto sum = bottom->input(0)->origin()->node();
	assert(sum->operation() == bitsub_op(32));
	auto constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 0));

	sum = bottom->input(1)->origin()->node();
	assert(sum->operation() == bitsub_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 2));

	sum = bottom->input(2)->origin()->node();
	assert(sum->operation() == bitsub_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 4));

	sum = bottom->input(3)->origin()->node();
	assert(sum->operation() == bitsub_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-containerof-transform",
	test_containerof_transform)

static int
test_memberof_transform(void)
{
	using namespace jive;

	jive::graph graph;

	jive::addrtype addrtype;
	jive::bittype bits8(8);
	jive::bittype bits16(16);
	jive::bittype bits32(32);
	auto dcl = rcddeclaration::create(&graph, {&bits8, &bits16, &bits32, &bits32});

	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {bits32});

	auto address = bit2addr_op::create(top->output(0), 32, addrtype);

	auto member0 = memberof_op::create(address, dcl, 0);
	auto member1 = memberof_op::create(address, dcl, 1);
	auto member2 = memberof_op::create(address, dcl, 2);
	auto member3 = memberof_op::create(address, dcl, 3);

	auto offset0 = addr2bit_op::create(member0, 32, member0->type());
	auto offset1 = addr2bit_op::create(member1, 32, member1->type());
	auto offset2 = addr2bit_op::create(member2, 32, member2->type());
	auto offset3 = addr2bit_op::create(member3, 32, member3->type());

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<jive::port>(4, bits32), {offset0, offset1, offset2, offset3},
		{bits32});
	graph.add_export(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(&graph, mapper);

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->output(i)->type()));
		}
	}

	auto sum = bottom->input(0)->origin()->node();
	assert(sum->operation() == bitadd_op(32));
	auto constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 0));

	sum = bottom->input(1)->origin()->node();
	assert(sum->operation() == bitadd_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 2));

	sum = bottom->input(2)->origin()->node();
	assert(sum->operation() == bitadd_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 4));

	sum = bottom->input(3)->origin()->node();
	assert(sum->operation() == bitadd_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-memberof-transform", test_memberof_transform)
