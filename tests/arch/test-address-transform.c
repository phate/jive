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
#include <jive/arch/memorytype.h>
#include <jive/arch/store.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/label.h>

#include "testnodes.h"

static int
test_address_transform(void)
{
	jive::graph graph;

	jive::addr::type addr;
	jive::mem::type mem;
	jive::bits::type bits64(64);
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {&bits64, &bits64, &mem});

	auto address0 = jive_bitstring_to_address_create(top->output(0), 64, &addr);
	auto address1 = jive_bitstring_to_address_create(top->output(1), 64, &addr);

	std::shared_ptr<const jive::rcd::declaration> decl(new jive::rcd::declaration({&addr, &addr}));

	auto memberof = jive_memberof(address0, decl, 0);
	auto containerof = jive_containerof(address1, decl, 1);

	jive_linker_symbol write_symbol;
	jive_label_external write_label;
	jive_label_external_init(&write_label, "write", &write_symbol);
	auto label = jive_label_to_address_create(graph.root(), &write_label.base);
	jive::oport * tmparray2[] = {memberof, containerof};
	const jive::base::type * tmparray3[] = {&addr, &addr};
	jive::node * call = jive_call_by_address_node_create(graph.root(),
		label, NULL,
		2, tmparray2,
		2, tmparray3);

	auto constant = jive_bitconstant_unsigned(graph.root(), 64, 1);
	auto arraysub = jive_arraysubscript(call->output(0), &addr, constant);

	auto arrayindex = jive_arrayindex(call->output(0), call->output(1), &addr, &bits64);

	jive::oport * state = top->output(2);
	auto load = jive_load_by_address_create(arraysub, &addr, 1, &state);
	auto store = jive_store_by_address_create(arraysub, &bits64, arrayindex, 1, &state)[0]->node();

	auto o_addr = jive_address_to_bitstring_create(load, 64, &load->type());

	auto bottom = jive::test::simple_node_create(graph.root(),
		{&bits64, &mem}, {o_addr, store->output(0)}, {&bits64});
	graph.export_port(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(8);
	jive_graph_address_transform(&graph, &mapper);

	graph.prune();
	jive::view(graph.root(), stdout);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform", test_address_transform);

static int
test_address_transform_nodes(void)
{
	jive::graph graph;

	jive::addr::type addrtype;
	jive::bits::type bits32(32);
	jive::bits::type bits64(64);
	auto i0 = graph.import(addrtype, "i0");
	auto i1 = graph.import(bits32, "i1");
	auto i2 = graph.import(bits64, "i2");

	auto b0 = jive_address_to_bitstring_create(i0, 32, &i0->type());
	auto a0 = jive_bitstring_to_address_create(b0, 32, &addrtype);

	auto a1 = jive_bitstring_to_address_create(i1, 32, &addrtype);
	auto b1 = jive_address_to_bitstring_create(a1, 32, &addrtype);

	auto x0 = graph.export_port(a0, "x0");
	auto x1 = graph.export_port(b1, "x1");

	assert(x0->origin() == i0);
	assert(x1->origin() == i1);

	auto b2 = jive_bitstring_to_address_create(i1, 32, &addrtype);
	auto b3 = jive_bitstring_to_address_create(i1, 32, &addrtype);
	auto a2 = jive_address_to_bitstring_create(i0, 32, &i0->type());
	auto a3 = jive_address_to_bitstring_create(i0, 32, &i0->type());

	assert(a2->node()->operation() == a3->node()->operation());
	assert(b2->node()->operation() == b3->node()->operation());

	auto b4 = jive_bitstring_to_address_create(i2, 64, &addrtype);
	auto a4 = jive_address_to_bitstring_create(i0, 64, &addrtype);

	assert(a2->node()->operation() != a4->node()->operation());
	assert(b2->node()->operation() != b4->node()->operation());

	jive::view(graph.root(), stderr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-address-transform-nodes",
	test_address_transform_nodes);

static int
test_apply_transform(void)
{
	jive::graph graph;

	jive::addr::type addrtype;
	jive::fct::type fcttype({&addrtype}, {&addrtype});
	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {&fcttype, &addrtype});

	jive::oport * address = top->output(1);
	auto results = jive_apply_create(top->output(0), 1, &address);

	auto bottom = jive::test::simple_node_create(graph.root(), {&addrtype}, {results.begin(),
		results.end()}, {});

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(4);
	jive_node_address_transform(results[0]->node(), &mapper);

	jive::view(graph.root(), stdout);

	assert(bottom->input(0)->origin()->node()->operation()
		== jive::bitstring_to_address_operation(32, addrtype));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-apply_transform", test_apply_transform);

static int
test_containerof_transform(void)
{
	jive::graph graph;

	jive::addr::type addrtype;
	jive::bits::type bits8(8);
	jive::bits::type bits16(16);
	jive::bits::type bits32(32);
	std::shared_ptr<const jive::rcd::declaration> decl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32, &bits32}));

	auto top = jive::test::simple_node_create(graph.root(), {}, {},
		std::vector<const jive::base::type*>(4, &bits32));

	auto address0 = jive_bitstring_to_address_create(top->output(0), 32, &addrtype);
	auto address1 = jive_bitstring_to_address_create(top->output(1), 32, &addrtype);
	auto address2 = jive_bitstring_to_address_create(top->output(2), 32, &addrtype);
	auto address3 = jive_bitstring_to_address_create(top->output(3), 32, &addrtype);

	auto container0 = jive_containerof(address0, decl, 0);
	auto container1 = jive_containerof(address1, decl, 1);
	auto container2 = jive_containerof(address2, decl, 2);
	auto container3 = jive_containerof(address3, decl, 3);

	auto offset0 = jive_address_to_bitstring_create(container0, 32, &container0->type());
	auto offset1 = jive_address_to_bitstring_create(container1, 32, &container1->type());
	auto offset2 = jive_address_to_bitstring_create(container2, 32, &container2->type());
	auto offset3 = jive_address_to_bitstring_create(container3, 32, &container3->type());

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<const jive::base::type*>(4, &bits32), {offset0, offset1, offset2, offset3},
		{&bits32});
	graph.export_port(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(4);
	jive_graph_address_transform(&graph, &mapper);

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addr::type*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addr::type*>(&node->output(i)->type()));
		}
	}

	auto sum = bottom->input(0)->origin()->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	auto constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 0));

	sum = bottom->input(1)->origin()->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 2));

	sum = bottom->input(2)->origin()->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 4));

	sum = bottom->input(3)->origin()->node();
	assert(sum->operation() == jive::bits::sub_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-containerof-transform",
	test_containerof_transform);

static int
test_memberof_transform(void)
{
	jive::graph graph;

	jive::addr::type addrtype;
	jive::bits::type bits8(8);
	jive::bits::type bits16(16);
	jive::bits::type bits32(32);
	std::shared_ptr<const jive::rcd::declaration> decl(
		new jive::rcd::declaration({&bits8, &bits16, &bits32, &bits32}));

	auto top = jive::test::simple_node_create(graph.root(), {}, {}, {&bits32});

	auto address = jive_bitstring_to_address_create(top->output(0), 32, &addrtype);

	auto member0 = jive_memberof(address, decl, 0);
	auto member1 = jive_memberof(address, decl, 1);
	auto member2 = jive_memberof(address, decl, 2);
	auto member3 = jive_memberof(address, decl, 3);

	auto offset0 = jive_address_to_bitstring_create(member0, 32, &member0->type());
	auto offset1 = jive_address_to_bitstring_create(member1, 32, &member1->type());
	auto offset2 = jive_address_to_bitstring_create(member2, 32, &member2->type());
	auto offset3 = jive_address_to_bitstring_create(member3, 32, &member3->type());

	auto bottom = jive::test::simple_node_create(graph.root(),
		std::vector<const jive::base::type*>(4, &bits32), {offset0, offset1, offset2, offset3},
		{&bits32});
	graph.export_port(bottom->output(0), "dummy");

	jive::view(graph.root(), stdout);

	jive::memlayout_mapper_simple mapper(4);
	jive_graph_address_transform(&graph, &mapper);

	graph.normalize();
	graph.prune();
	jive::view(graph.root(), stdout);

	for (jive::node * node : jive::topdown_traverser(graph.root())) {
		for (size_t i = 0; i < node->ninputs(); i++){
			assert(!dynamic_cast<const jive::addr::type*>(&node->input(i)->type()));
		}
		for (size_t i = 0; i < node->noutputs(); i++){
			assert(!dynamic_cast<const jive::addr::type*>(&node->output(i)->type()));
		}
	}

	auto sum = bottom->input(0)->origin()->node();
	assert(sum->operation() == jive::bits::add_op(32));
	auto constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 0));

	sum = bottom->input(1)->origin()->node();
	assert(sum->operation() == jive::bits::add_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 2));

	sum = bottom->input(2)->origin()->node();
	assert(sum->operation() == jive::bits::add_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 4));

	sum = bottom->input(3)->origin()->node();
	assert(sum->operation() == jive::bits::add_op(32));
	constant = sum->input(1)->origin()->node();
	assert(constant->operation() == jive::bits::int_constant_op(32, 8));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/address-transform/test-memberof-transform", test_memberof_transform);
