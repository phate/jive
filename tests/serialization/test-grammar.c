/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/token-stream.h>
#include <jive/types/bitstring.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>

static void
my_handle_error(jive_serialization_driver * ctx, const char msg[])
{
	fputs(msg, stderr);
	abort();
}

typedef struct serialize_ctx {
	jive_buffer buf;
	jive_token_ostream * os;
	jive_serialization_driver drv;
} serialize_ctx;

static bool
token_stream_equal(const char * str1, size_t len1, const char * str2, size_t len2)
{
	bool equals = true;
	jive_token_istream * is1 = jive_token_istream_simple_create(str1, str1 + len1);
	jive_token_istream * is2 = jive_token_istream_simple_create(str2, str2 + len2);
	
	for (;;) {
		const jive_token * tok1 = jive_token_istream_current(is1);
		const jive_token * tok2 = jive_token_istream_current(is2);
		if (!jive_token_equals(tok1, tok2)) {
			equals = false;
			break;
		}
		if (tok1->type == jive_token_end)
			break;
		jive_token_istream_advance(is1);
		jive_token_istream_advance(is2);
	}
	jive_token_istream_destroy(is2);
	jive_token_istream_destroy(is1);
	
	return equals;
}

static void
serialize_ctx_init(serialize_ctx * self)
{
	self->os = jive_token_ostream_simple_create(&self->buf);
	jive_serialization_driver_init(&self->drv);
	self->drv.error = my_handle_error;
}

static void
serialize_ctx_fini(serialize_ctx * self)
{
	jive_serialization_driver_fini(&self->drv);
	jive_token_ostream_destroy(self->os);
}

typedef struct deserialize_ctx {
	jive_token_istream * is;
	jive_serialization_driver drv;
} deserialize_ctx;

static void
deserialize_ctx_init(deserialize_ctx * self, const char * repr)
{
	self->is = jive_token_istream_simple_create(repr, repr + strlen(repr));
	jive_serialization_driver_init(&self->drv);
	self->drv.error = my_handle_error;
}

static void
deserialize_ctx_fini(deserialize_ctx * self)
{
	jive_serialization_driver_fini(&self->drv);
	jive_token_istream_destroy(self->is);
}

static void
verify_serialize_type(const jive::base::type * type, const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	jive_serialize_type(&ctx.drv, type, ctx.os);
	
	assert(token_stream_equal(&ctx.buf.data[0], ctx.buf.data.size(),
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_type(const char * repr, const jive::base::type * expect_type)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	jive::base::type * type;
	assert(jive_deserialize_type(&ctx.drv, ctx.is, &type));
	assert(*type == *expect_type);
	delete type;
	
	deserialize_ctx_fini(&ctx);
}

static void
verify_serialize_rescls(const jive_resource_class * rescls, const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	jive_serialize_rescls(&ctx.drv, rescls, ctx.os);
	
	assert(token_stream_equal(&ctx.buf.data[0], ctx.buf.data.size(),
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_rescls(const char * repr, const jive_resource_class * expect_rescls)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	const jive_resource_class * rescls;
	assert(jive_deserialize_rescls(&ctx.drv, ctx.is, &rescls));
	assert(rescls == expect_rescls);
	
	deserialize_ctx_fini(&ctx);
}

static void
verify_serialize_gateexpr(jive::gate * gate, const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	jive_serialize_gateexpr(&ctx.drv, gate, ctx.os);
	
	assert(token_stream_equal(&ctx.buf.data[0], ctx.buf.data.size(),
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_gateexpr(const char * repr, jive::gate * expect_gate)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	jive_graph * g = jive_graph_create();
	
	jive::gate * gate;
	assert(jive_deserialize_gateexpr(&ctx.drv, ctx.is, g, &gate));
	assert(gate->name() == expect_gate->name());
	assert(gate->type() == expect_gate->type());
	assert(gate->required_rescls == expect_gate->required_rescls);
	
	jive_graph_destroy(g);
	
	deserialize_ctx_fini(&ctx);
}

static void
verify_serialize_nodeexpr(jive_node * node,
	size_t ngates,
	const char * const gate_names[],
	jive::gate * const gates[],
	const char * const * input_names,
	const char * const * output_names,
	const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	size_t n;
	for (n = 0; n < ngates; ++n)
		jive_serialization_symtab_insert_gatesym(&ctx.drv.symtab, gates[n], gate_names[n]);
	for (n = 0; n < node->ninputs(); ++n)
		jive_serialization_symtab_insert_outputsym(&ctx.drv.symtab,
			node->input(n)->origin(), input_names[n]);
	for (n = 0; n < node->noutputs; ++n)
		jive_serialization_symtab_insert_outputsym(&ctx.drv.symtab,
			node->outputs[n], output_names[n]);
	
	jive_serialize_nodeexpr(&ctx.drv, node, ctx.os);
	
	assert(token_stream_equal(&ctx.buf.data[0], ctx.buf.data.size(),
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_nodeexpr(
	const char * repr,
	jive_region * region,
	const char * const input_names[],
	jive::output * const input_origins[],
	const char * const output_names[],
	jive_node * expected_node)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	size_t n;
	for (n = 0; n < expected_node->ninputs(); ++n)
		jive_serialization_symtab_insert_outputsym(&ctx.drv.symtab, input_origins[n], input_names[n]);
	
	jive_node * node;
	assert(jive_deserialize_nodeexpr(&ctx.drv, ctx.is, region, &node));
	assert(node->operation() == expected_node->operation());
	assert(node->ninputs() == expected_node->ninputs());
	assert(node->noperands() == expected_node->noperands());
	assert(node->noutputs == expected_node->noutputs);
	
	for (n = 0; n < node->ninputs(); ++n) {
		assert(node->input(n)->origin() == expected_node->input(n)->origin());
		assert(node->input(n)->required_rescls == expected_node->input(n)->required_rescls);
		assert(node->input(n)->gate == expected_node->input(n)->gate);
	}
	
	for (n = 0; n < expected_node->noutputs; ++n) {
		const jive_serialization_outputsym * sym =
			jive_serialization_symtab_name_to_output(&ctx.drv.symtab, output_names[n]);
		assert(sym && sym->output == node->outputs[n]);
		const jive::base::type * type = &node->outputs[n]->type();
		const jive::base::type * expected_type = &expected_node->outputs[n]->type();
		assert(*type == *expected_type);
		assert(node->outputs[n]->required_rescls == expected_node->outputs[n]->required_rescls);
		assert(node->outputs[n]->gate == expected_node->outputs[n]->gate);
	}
	
	deserialize_ctx_fini(&ctx);
}

static int test_main(void)
{
	verify_serialize_rescls(&jive_root_resource_class, "root<>");
	verify_serialize_rescls(&jive_root_register_class, "register<>");
	verify_serialize_rescls(jive_stackslot_size_class_get(4, 4), "stackslot<4,4>");
	verify_serialize_rescls(jive_fixed_stackslot_class_get(4, 4, -12), "stack_frameslot<4,4,-12>");
	verify_serialize_rescls(jive_callslot_class_get(4, 4, 12), "stack_callslot<4,4,12>");
	
	verify_deserialize_rescls("root<>", &jive_root_resource_class);
	verify_deserialize_rescls("register<>", &jive_root_register_class);
	verify_deserialize_rescls("stackslot<4,32>", jive_stackslot_size_class_get(4, 32));
	verify_deserialize_rescls("stack_frameslot<4,4,-32>", jive_fixed_stackslot_class_get(4, 4, -32));
	verify_deserialize_rescls("stack_callslot<4,4,32>", jive_callslot_class_get(4, 4, 32));
	verify_deserialize_rescls("stack_callslot<4,4,-32>", jive_callslot_class_get(4, 4, -32));
	
	jive::bits::type bits8(8);
	verify_serialize_type(&bits8, "bits<8>");
	verify_deserialize_type("bits<8>", &bits8);
	
	jive::ctl::type ctl(3);
	verify_serialize_type(&ctl, "control<3>");
	verify_deserialize_type("control<3>", &ctl);
	
	jive_graph * graph = jive_graph_create();
	
	jive::gate * bit8gate = jive_graph_create_gate(graph, "bit8gate", bits8);
	verify_serialize_gateexpr(bit8gate, "\"bit8gate\" root<> bits<8>");
	verify_deserialize_gateexpr("\"bit8gate\" root<> bits<8>", bit8gate);
	
	jive::gate * stackgate = jive_resource_class_create_gate(jive_stackslot_size_class_get(4, 4),
		graph, "stackgate");
	verify_serialize_gateexpr(stackgate, "\"stackgate\" stackslot<4,4> memory<>");
	verify_deserialize_gateexpr("\"stackgate\" stackslot<4,4> memory<>", stackgate);
	
	/* inhibit implicit optimization transformations */
	jive_graph_get_nodeclass_form(graph, typeid(jive::operation))->set_mutable(false);
	
	jive::output * zero8 = jive_bitconstant(graph->root_region, 8, "00000000");
	const char * tmparray0[] = {"out"};
	verify_serialize_nodeexpr(zero8->node(),
		0, NULL, NULL, /* gates */
		NULL, /* inputs */
		tmparray0 /* outputs */,
		"(;) bitconstant<\"00000000\"> (out:root<>;)");
	const char * tmparray1[] = {"out"};
	verify_deserialize_nodeexpr("(;) bitconstant<\"00000000\"> (out:root<>;)",
		graph->root_region,
		NULL, NULL, /* input names & origins */
		tmparray1, /* output names */
		zero8->node());
	
	jive::output * one8 = jive_bitconstant(graph->root_region, 8, "10000000");
	jive::output * two8 = jive_bitconstant(graph->root_region, 8, "01000000");
	jive::output * tmparray2[] = {one8, two8};
	jive::output * add8 = jive_bitsum(2, tmparray2);
	assert(dynamic_cast<const jive::bits::add_op *>(&add8->node()->operation()));
	assert(add8->node() != zero8->node());
	const char * tmparray3[] = {"a", "b"};
	const char * tmparray4[] = {"sum"};
	verify_serialize_nodeexpr(add8->node(),
		0, NULL, NULL, /* gates */
		tmparray3, tmparray4,
		"(a:root<> b:root<>;) bitsum<8,2> (sum:root<>;)");
	const char * tmparray5[] = {"a", "b"};
	jive::output * tmparray6[] = {one8, two8};
	const char * tmparray7[] = {"sum"};
	verify_deserialize_nodeexpr("(a:root<> b:root<>;) bitsum<8,2> (sum:root<>;)",
		graph->root_region,
		tmparray5, tmparray6, /* input names & origins */
		tmparray7, /* output names */
		add8->node());
	jive::output * tmparray8[] = {one8, two8};
	
	jive::output * cat16 = jive_bitconcat(2, tmparray8);
	jive_node * cat16n = cat16->node();
	cat16n->add_input(bit8gate, zero8);
	cat16n->add_output(stackgate);
	const char * tmparray9[] = {"bit8gate", "stackgate"};
	jive::gate* tmparray10[] = {bit8gate, stackgate};
	const char * tmparray11[] = {"a", "b", "c"};
	const char * tmparray12[] = {"out", "r"};
	verify_serialize_nodeexpr(cat16n,
		2, tmparray9, tmparray10, /* gates */
		tmparray11, tmparray12,
		"(a:root<> b:root<>;c:root<>:bit8gate) bitconcat<8,8> (out:root<>;r:stackslot<4,4>:stackgate)");
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("serialization/test-grammar", test_main);
