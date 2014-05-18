/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <jive/types/bitstring.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/arch/stackslot.h>
#include <jive/serialization/grammar.h>
#include <jive/serialization/token-stream.h>
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
	jive_context * ctx;
	jive_buffer buf;
	jive_token_ostream * os;
	jive_serialization_driver drv;
} serialize_ctx;

static bool
token_stream_equal(jive_context * ctx,
	const char * str1, size_t len1,
	const char * str2, size_t len2)
{
	bool equals = true;
	jive_token_istream * is1 = jive_token_istream_simple_create(
		ctx, str1, str1 + len1);
	jive_token_istream * is2 = jive_token_istream_simple_create(
		ctx, str2, str2 + len2);
	
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
	self->ctx = jive_context_create();
	jive_buffer_init(&self->buf, self->ctx);
	self->os = jive_token_ostream_simple_create(&self->buf);
	jive_serialization_driver_init(&self->drv, self->ctx);
	self->drv.error = my_handle_error;
}

static void
serialize_ctx_fini(serialize_ctx * self)
{
	jive_serialization_driver_fini(&self->drv);
	jive_token_ostream_destroy(self->os);
	jive_buffer_fini(&self->buf);
	assert(jive_context_is_empty(self->ctx));
	jive_context_destroy(self->ctx);
}

typedef struct deserialize_ctx {
	jive_context * ctx;
	jive_token_istream * is;
	jive_serialization_driver drv;
} deserialize_ctx;

static void
deserialize_ctx_init(deserialize_ctx * self, const char * repr)
{
	self->ctx = jive_context_create();
	self->is = jive_token_istream_simple_create(
		self->ctx, repr, repr + strlen(repr));
	jive_serialization_driver_init(&self->drv, self->ctx);
	self->drv.error = my_handle_error;
}

static void
deserialize_ctx_fini(deserialize_ctx * self)
{
	jive_serialization_driver_fini(&self->drv);
	jive_token_istream_destroy(self->is);
	assert(jive_context_is_empty(self->ctx));
	jive_context_destroy(self->ctx);
}

static void
verify_serialize_type(const jive_type * type, const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	jive_serialize_type(&ctx.drv, type, ctx.os);
	
	assert(token_stream_equal(ctx.ctx, ctx.buf.data, ctx.buf.size,
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_type(const char * repr, const jive_type * expect_type)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	jive_type * type;
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
	
	assert(token_stream_equal(ctx.ctx, ctx.buf.data, ctx.buf.size,
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
verify_serialize_gateexpr(jive_gate * gate, const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	jive_serialize_gateexpr(&ctx.drv, gate, ctx.os);
	
	assert(token_stream_equal(ctx.ctx, ctx.buf.data, ctx.buf.size,
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_gateexpr(const char * repr, jive_gate * expect_gate)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	jive_graph * g = jive_graph_create(ctx.ctx);
	
	jive_gate * gate;
	assert(jive_deserialize_gateexpr(&ctx.drv, ctx.is, g, &gate));
	assert(strcmp(gate->name, expect_gate->name) == 0);
	assert(*jive_gate_get_type(gate) == *jive_gate_get_type(expect_gate));
	assert(gate->required_rescls == expect_gate->required_rescls);
	
	jive_graph_destroy(g);
	
	deserialize_ctx_fini(&ctx);
}

static void
verify_serialize_nodeexpr(jive_node * node,
	size_t ngates,
	const char * const gate_names[],
	jive_gate * const gates[],
	const char * const * input_names,
	const char * const * output_names,
	const char * expect_repr)
{
	serialize_ctx ctx;
	serialize_ctx_init(&ctx);
	
	size_t n;
	for (n = 0; n < ngates; ++n)
		jive_serialization_symtab_insert_gatesym(&ctx.drv.symtab,
			gates[n],
			jive_serialization_symtab_strdup(&ctx.drv.symtab, gate_names[n]));
	for (n = 0; n < node->ninputs; ++n)
		jive_serialization_symtab_insert_outputsym(&ctx.drv.symtab,
			node->inputs[n]->origin(),
			jive_serialization_symtab_strdup(&ctx.drv.symtab, input_names[n]));
	for (n = 0; n < node->noutputs; ++n)
		jive_serialization_symtab_insert_outputsym(&ctx.drv.symtab,
			node->outputs[n],
			jive_serialization_symtab_strdup(&ctx.drv.symtab, output_names[n]));
	
	jive_serialize_nodeexpr(&ctx.drv, node, ctx.os);
	
	assert(token_stream_equal(ctx.ctx, ctx.buf.data, ctx.buf.size,
		expect_repr, strlen(expect_repr)));
	
	serialize_ctx_fini(&ctx);
}

static void
verify_deserialize_nodeexpr(
	const char * repr,
	jive_region * region,
	const char * const input_names[],
	jive_output * const input_origins[],
	const char * const output_names[],
	jive_node * expected_node)
{
	deserialize_ctx ctx;
	deserialize_ctx_init(&ctx, repr);
	
	size_t n;
	for (n = 0; n < expected_node->ninputs; ++n) {
		jive_serialization_symtab_insert_outputsym(&ctx.drv.symtab,
			input_origins[n],
			jive_serialization_symtab_strdup(&ctx.drv.symtab, input_names[n]));
	}
	
	jive_node * node;
	assert(jive_deserialize_nodeexpr(&ctx.drv, ctx.is, region, &node));
	assert(node->class_ == expected_node->class_);
	assert(jive_node_match_attrs(node, jive_node_get_attrs(expected_node)));
	assert(node->ninputs == expected_node->ninputs);
	assert(node->noperands == expected_node->noperands);
	assert(node->noutputs == expected_node->noutputs);
	
	for (n = 0; n < node->ninputs; ++n) {
		assert(node->inputs[n]->origin() == expected_node->inputs[n]->origin());
		assert(node->inputs[n]->required_rescls == expected_node->inputs[n]->required_rescls);
		assert(node->inputs[n]->gate == expected_node->inputs[n]->gate);
	}
	
	for (n = 0; n < expected_node->noutputs; ++n) {
		const jive_serialization_outputsym * sym =
			jive_serialization_symtab_name_to_output(&ctx.drv.symtab, output_names[n]);
		assert(sym && sym->output == node->outputs[n]);
		const jive_type * type = jive_output_get_type(node->outputs[n]);
		const jive_type * expected_type = jive_output_get_type(expected_node->outputs[n]);
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
	
	jive_bitstring_type bits8(8);
	verify_serialize_type(&bits8, "bits<8>");
	verify_deserialize_type("bits<8>", &bits8);
	
	jive_control_type ctl;
	verify_serialize_type(&ctl, "control<>");
	verify_deserialize_type("control<>", &ctl);
	
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_gate * bit8gate = jive_type_create_gate(&bits8, graph, "bit8gate");
	verify_serialize_gateexpr(bit8gate, "\"bit8gate\" root<> bits<8>");
	verify_deserialize_gateexpr("\"bit8gate\" root<> bits<8>", bit8gate);
	
	jive_gate * stackgate = jive_resource_class_create_gate(jive_stackslot_size_class_get(4, 4), graph, "stackgate");
	verify_serialize_gateexpr(stackgate, "\"stackgate\" stackslot<4,4> memory<>");
	verify_deserialize_gateexpr("\"stackgate\" stackslot<4,4> memory<>", stackgate);
	
	/* inhibit implicit optimization transformations */
	jive_node_normal_form_set_mutable(
		jive_graph_get_nodeclass_form(graph, &JIVE_NODE), false);
	
	jive_output * zero8 = jive_bitconstant(graph, 8, "00000000");
	const char * tmparray0[] = {"out"};
	verify_serialize_nodeexpr(zero8->node,
		0, NULL, NULL, /* gates */
		NULL, /* inputs */
		tmparray0 /* outputs */,
		"(;) bitconstant<\"00000000\"> (out:root<>;)");
	const char * tmparray1[] = {"out"};
	verify_deserialize_nodeexpr("(;) bitconstant<\"00000000\"> (out:root<>;)",
		graph->root_region,
		NULL, NULL, /* input names & origins */
		tmparray1, /* output names */
		zero8->node);
	
	jive_output * one8 = jive_bitconstant(graph, 8, "10000000");
	jive_output * two8 = jive_bitconstant(graph, 8, "01000000");
	jive_output * tmparray2[] = {one8, two8};
	jive_output * add8 = jive_bitsum(2, tmparray2);
	assert(add8->node->class_ == &JIVE_BITSUM_NODE);
	assert(add8->node != zero8->node);
	const char * tmparray3[] = {"a", "b"};
	const char * tmparray4[] = {"sum"};
	verify_serialize_nodeexpr(add8->node,
		0, NULL, NULL, /* gates */
		tmparray3, tmparray4,
		"(a:root<> b:root<>;) bitsum<> (sum:root<>;)");
	const char * tmparray5[] = {"a", "b"};
	jive_output * tmparray6[] = {one8, two8};
	const char * tmparray7[] = {"sum"};
	verify_deserialize_nodeexpr("(a:root<> b:root<>;) bitsum<> (sum:root<>;)",
		graph->root_region,
		tmparray5, tmparray6, /* input names & origins */
		tmparray7, /* output names */
		add8->node);
	jive_output * tmparray8[] = {one8, two8};
	
	jive_output * cat16 = jive_bitconcat(2, tmparray8);
	jive_node * cat16n = cat16->node;
	jive_node_gate_input(cat16n, bit8gate, zero8);
	jive_node_gate_output(cat16n, stackgate);
	const char * tmparray9[] = {"bit8gate", "stackgate"};
	jive_gate* tmparray10[] = {bit8gate, stackgate};
	const char * tmparray11[] = {"a", "b", "c"};
	const char * tmparray12[] = {"out", "r"};
	verify_serialize_nodeexpr(cat16n,
		2, tmparray9, tmparray10, /* gates */
		tmparray11, tmparray12,
		"(a:root<> b:root<>;c:root<>:bit8gate) bitconcat<> (out:root<>;r:stackslot<4,4>:stackgate)");
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("serialization/test-grammar", test_main);
