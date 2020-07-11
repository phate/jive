/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 2016 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/address-transform.hpp>
#include <jive/arch/address.hpp>
#include <jive/arch/addresstype.hpp>
#include <jive/arch/call.hpp>
#include <jive/arch/linker-symbol.hpp>
#include <jive/arch/load.hpp>
#include <jive/arch/memlayout-simple.hpp>
#include <jive/arch/store.hpp>
#include <jive/rvsdg.hpp>
#include <jive/rvsdg/label.hpp>
#include <jive/types/bitstring.hpp>
#include <jive/view.hpp>

#include "testnodes.hpp"

static int
test_address_transform_nodes(void)
{
	using namespace jive;

	addrtype at(bit32);

	jive::graph graph;
	auto i0 = graph.add_import({at, "i0"});
	auto i1 = graph.add_import({bit32, "i1"});
	auto i2 = graph.add_import({bit64, "i2"});

	auto b0 = addr2bit_op::create(i0, 32, i0->type());
	auto a0 = bit2addr_op::create(b0, 32, at);

	auto a1 = bit2addr_op::create(i1, 32, at);
	auto b1 = addr2bit_op::create(a1, 32, at);

	auto x0 = graph.add_export(a0, {a0->type(), "x0"});
	auto x1 = graph.add_export(b1, {b1->type(), "x1"});

	assert(x0->origin() == i0);
	assert(x1->origin() == i1);

	auto b2 = bit2addr_op::create(i1, 32, at);
	auto b3 = bit2addr_op::create(i1, 32, at);
	auto a2 = addr2bit_op::create(i0, 32, i0->type());
	auto a3 = addr2bit_op::create(i0, 32, i0->type());

	assert(node_output::node(a2)->operation() == node_output::node(a3)->operation());
	assert(node_output::node(b2)->operation() == node_output::node(b3)->operation());

	auto b4 = bit2addr_op::create(i2, 64, at);
	auto a4 = addr2bit_op::create(i0, 64, at);

	assert(node_output::node(a2)->operation() != node_output::node(a4)->operation());
	assert(node_output::node(b2)->operation() != node_output::node(b4)->operation());

	jive::view(graph.root(), stderr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes",
	test_address_transform_nodes)

static int
test_apply_transform(void)
{
	using namespace jive;

	addrtype at(bit32);
	fcttype ft({&at}, {&at});

	jive::graph graph;
	auto i0 = graph.add_import({ft, ""});
	auto i1 = graph.add_import({at, ""});

	auto results = create_apply(i0, {i1});

	auto x0 = graph.add_export(results[0], {results[0]->type(), ""});

	jive::view(graph.root(), stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(node_output::node(results[0]), mapper);

	jive::view(graph.root(), stdout);

	assert(is<bit2addr_op>(node_output::node(x0->origin())));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-apply_transform", test_apply_transform)

static int
test_containerof_transform(void)
{
	using namespace jive;

	jive::graph graph;
	auto i0 = graph.add_import({bit32, ""});
	auto i1 = graph.add_import({bit32, ""});
	auto i2 = graph.add_import({bit32, ""});
	auto i3 = graph.add_import({bit32, ""});

	auto dcl = rcddeclaration::create({&bit8, &bit16, &bit32, &bit32});

	auto address0 = bit2addr_op::create(i0, 32, addrtype(bit8));
	auto address1 = bit2addr_op::create(i1, 32, addrtype(bit16));
	auto address2 = bit2addr_op::create(i2, 32, addrtype(bit32));
	auto address3 = bit2addr_op::create(i3, 32, addrtype(bit32));

	auto container0 = containerof_op::create(address0, dcl.get(), 0);
	auto container1 = containerof_op::create(address1, dcl.get(), 1);
	auto container2 = containerof_op::create(address2, dcl.get(), 2);
	auto container3 = containerof_op::create(address3, dcl.get(), 3);

	auto offset0 = addr2bit_op::create(container0, 32, container0->type());
	auto offset1 = addr2bit_op::create(container1, 32, container1->type());
	auto offset2 = addr2bit_op::create(container2, 32, container2->type());
	auto offset3 = addr2bit_op::create(container3, 32, container3->type());

	auto ex0 = graph.add_export(offset0, {offset0->type(), ""});
	auto ex1 = graph.add_export(offset1, {offset1->type(), ""});
	auto ex2 = graph.add_export(offset2, {offset2->type(), ""});
	auto ex3 = graph.add_export(offset3, {offset3->type(), ""});

	view(graph, stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(&graph, mapper);

	graph.normalize();
	graph.prune();
	view(graph, stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->output(i)->type()));
		}
	}

	auto sum = node_output::node(ex0->origin());
	assert(sum->operation() == bitsub_op(32));
	auto constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 0));

	sum = node_output::node(ex1->origin());
	assert(sum->operation() == bitsub_op(32));
	constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 2));

	sum = node_output::node(ex2->origin());
	assert(sum->operation() == bitsub_op(32));
	constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 4));

	sum = node_output::node(ex3->origin());
	assert(sum->operation() == bitsub_op(32));
	constant = node_output::node(sum->input(1)->origin());
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
	auto i0 = graph.add_import({bit32, ""});

	auto dcl = rcddeclaration::create({&bit8, &bit16, &bit32, &bit32});

	auto address = bit2addr_op::create(i0, 32, addrtype(rcdtype(dcl.get())));

	auto member0 = memberof_op::create(address, dcl.get(), 0);
	auto member1 = memberof_op::create(address, dcl.get(), 1);
	auto member2 = memberof_op::create(address, dcl.get(), 2);
	auto member3 = memberof_op::create(address, dcl.get(), 3);

	auto offset0 = addr2bit_op::create(member0, 32, member0->type());
	auto offset1 = addr2bit_op::create(member1, 32, member1->type());
	auto offset2 = addr2bit_op::create(member2, 32, member2->type());
	auto offset3 = addr2bit_op::create(member3, 32, member3->type());

	auto ex0 = graph.add_export(offset0, {offset0->type(), ""});
	auto ex1 = graph.add_export(offset1, {offset1->type(), ""});
	auto ex2 = graph.add_export(offset2, {offset2->type(), ""});
	auto ex3 = graph.add_export(offset3, {offset3->type(), ""});

	view(graph, stdout);

	memlayout_mapper_simple mapper(4);
	transform_address(&graph, mapper);

	graph.normalize();
	graph.prune();
	view(graph, stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addrtype*>(&node->output(i)->type()));
		}
	}

	auto sum = node_output::node(ex0->origin());
	assert(sum->operation() == bitadd_op(32));
	auto constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 0));

	sum = node_output::node(ex1->origin());
	assert(sum->operation() == bitadd_op(32));
	constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 2));

	sum = node_output::node(ex2->origin());
	assert(sum->operation() == bitadd_op(32));
	constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 4));

	sum = node_output::node(ex3->origin());
	assert(sum->operation() == bitadd_op(32));
	constant = node_output::node(sum->input(1)->origin());
	assert(constant->operation() == int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-memberof-transform", test_memberof_transform)
