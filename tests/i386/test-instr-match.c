/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>

#include <jive/arch/codegen.h>
#include <jive/arch/label-mapper.h>
#include <jive/arch/regselector.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/classifier.h>
#include <jive/backend/i386/instrmatch.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/machine.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/relocation.h>
#include <jive/backend/i386/subroutine.h>
#include <jive/regalloc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg.h>

static void *
compile_graph(jive_graph * graph)
{
	jive_i386_reg_classifier classifier;
	jive_regselector regselector;
	jive_regselector_init(&regselector, graph, &classifier);
	jive_regselector_process(&regselector);
	jive_i386_match_instructions(graph, &regselector);
	jive_regselector_fini(&regselector);
	jive_graph_prune(graph);
	
	jive_shaped_graph * shaped_graph = jive_regalloc(graph);
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_buffer buffer;
	jive_label_name_mapper * name_mapper = jive_label_name_mapper_simple_create(NULL, 0);
	jive_graph_generate_assembler(graph, name_mapper, &buffer);
	jive_label_name_mapper_destroy(name_mapper);
	fwrite(&buffer.data[0], buffer.data.size(), 1, stderr);

	jive_compilate compilate;
	jive_compilate_init(&compilate);
	jive_label_symbol_mapper * symbol_mapper = jive_label_symbol_mapper_simple_create();
	jive_graph_generate_code(graph, symbol_mapper, &compilate);
	jive_label_symbol_mapper_destroy(symbol_mapper);
	
	jive_compilate_map * map = jive_compilate_load(&compilate,
		NULL, jive_i386_process_relocation);
	void * result = jive_compilate_map_get_stdsection(map, jive_stdsectionid_code);
	jive_compilate_map_destroy(map);
	jive_compilate_fini(&compilate);
	
	jive_graph_destroy(graph);
	
	return result;
}

typedef uint32_t(*un_function_t)(uint32_t);
typedef jive::output * (un_op_factory_t)(jive::output *);

typedef uint32_t(*bin_function_t)(uint32_t, uint32_t);
typedef jive::output * (bin_op_factory_t)(jive::output *, jive::output *);

typedef struct {
	jive_graph * graph;
	jive::output * arg;
	jive_subroutine sub;
} un_graph;

typedef struct {
	jive_graph * graph;
	jive::output * arg1;
	jive::output * arg2;
	jive_subroutine sub;
} bin_graph;

static un_graph
prepare_un_graph()
{
	un_graph u;
	u.graph = jive_graph_create();
	jive_argument_type  tmparray0[] = { jive_argument_int };
	jive_argument_type  tmparray1[] = { jive_argument_int };
	
	u.sub = jive_i386_subroutine_begin(
		u.graph,
		1, tmparray0,
		1, tmparray1);
	
	u.arg = jive_subroutine_simple_get_argument(u.sub, 0);
	
	return u;
}

static un_function_t
generate_un_function(un_op_factory_t factory)
{
	un_graph u = prepare_un_graph();
	jive::output * result = factory(u.arg);
	jive_subroutine_simple_set_result(u.sub, 0, result);
	jive_graph_export(u.graph, jive_subroutine_end(u.sub)->outputs[0]);
	un_function_t function = (un_function_t) compile_graph(u.graph);
	
	return function;
}

static bin_graph
prepare_bin_graph()
{
	bin_graph b;
	b.graph = jive_graph_create();
	jive_argument_type  tmparray2[] = { jive_argument_int, jive_argument_int };
	jive_argument_type  tmparray3[] = { jive_argument_int };
	
	b.sub = jive_i386_subroutine_begin(
		b.graph,
		2, tmparray2,
		1, tmparray3);
	
	b.arg1 = jive_subroutine_simple_get_argument(b.sub, 0);
	b.arg2 = jive_subroutine_simple_get_argument(b.sub, 1);
	
	return b;
}

static bin_function_t
generate_bin_function(bin_op_factory_t factory)
{
	bin_graph b = prepare_bin_graph();
	jive::output * result = factory(b.arg1, b.arg2);
	jive_subroutine_simple_set_result(b.sub, 0, result);
	jive_graph_export(b.graph, jive_subroutine_end(b.sub)->outputs[0]);
	bin_function_t function = (bin_function_t) compile_graph(b.graph);
	
	return function;
}

static un_function_t
generate_bin_function_curryleft(bin_op_factory_t factory, uint32_t op1)
{
	un_graph u = prepare_un_graph();
	jive::output * c = jive_bitconstant_unsigned(u.graph, 32, op1);
	jive::output * result = factory(c, u.arg);
	jive_subroutine_simple_set_result(u.sub, 0, result);
	jive_graph_export(u.graph, jive_subroutine_end(u.sub)->outputs[0]);
	un_function_t function = (un_function_t) compile_graph(u.graph);
	
	return function;
}

static un_function_t
generate_bin_function_curryright(bin_op_factory_t factory, uint32_t op2)
{
	un_graph u = prepare_un_graph();
	jive::output * c = jive_bitconstant_unsigned(u.graph, 32, op2);
	jive::output * result = factory(u.arg, c);
	jive_subroutine_simple_set_result(u.sub, 0, result);
	jive_graph_export(u.graph, jive_subroutine_end(u.sub)->outputs[0]);
	un_function_t function = (un_function_t) compile_graph(u.graph);
	
	return function;
}

static bin_function_t
generate_bin_cmp_function(bin_op_factory_t factory)
{
	bin_graph b = prepare_bin_graph();
	jive::output * cmp = factory(b.arg1, b.arg2);
	jive::output * zero = jive_bitconstant_unsigned(b.graph, 32, 0);
	jive::output * one = jive_bitconstant_unsigned(b.graph, 32, 1);

	jive::output * pred = jive::ctl::match(1, {{1,0}}, 1, 2, cmp);
	
	jive::bits::type bits32(32);
	jive::output * result = jive_gamma(pred, {&bits32}, {{one}, {zero}})[0];
	
	jive_subroutine_simple_set_result(b.sub, 0, result);
	jive_graph_export(b.graph, jive_subroutine_end(b.sub)->outputs[0]);
	bin_function_t function = (bin_function_t) compile_graph(b.graph);
	
	return function;
}

static un_function_t
generate_bin_cmp_function_curryleft(bin_op_factory_t factory, uint32_t op1)
{
	un_graph u = prepare_un_graph();
	jive::output * c = jive_bitconstant_unsigned(u.graph, 32, op1);
	jive::output * cmp = factory(c, u.arg);
	jive::output * zero = jive_bitconstant_unsigned(u.graph, 32, 0);
	jive::output * one = jive_bitconstant_unsigned(u.graph, 32, 1);
	
	jive::output * pred = jive::ctl::match(1, {{1,0}}, 1, 2, cmp);

	jive::bits::type bits32(32);
	jive::output * result = jive_gamma(pred, {&bits32}, {{one}, {zero}})[0];

	jive_subroutine_simple_set_result(u.sub, 0, result);
	jive_graph_export(u.graph, jive_subroutine_end(u.sub)->outputs[0]);
	un_function_t function = (un_function_t) compile_graph(u.graph);
	
	return function;
}

static un_function_t
generate_bin_cmp_function_curryright(bin_op_factory_t factory, uint32_t op2)
{
	un_graph u = prepare_un_graph();
	jive::output * c = jive_bitconstant_unsigned(u.graph, 32, op2);
	jive::output * cmp = factory(u.arg, c);
	jive::output * zero = jive_bitconstant_unsigned(u.graph, 32, 0);
	jive::output * one = jive_bitconstant_unsigned(u.graph, 32, 1);

	jive::output * pred = jive::ctl::match(1, {{1, 0}}, 1, 2, cmp);
	
	jive::bits::type bits32(32);
	jive::output * result = jive_gamma(pred, {&bits32}, {{one}, {zero}})[0];

	jive_subroutine_simple_set_result(u.sub, 0, result);
	jive_graph_export(u.graph, jive_subroutine_end(u.sub)->outputs[0]);
	un_function_t function = (un_function_t) compile_graph(u.graph);
	
	return function;
}

static uint32_t ops[] = { 0, 1, 2, 3, 31, 32, 256, 65535, 65536, (uint32_t) -256, (uint32_t) -1 };

static void
exercise_bin_function(bin_function_t ref, bin_function_t f, bool allow_2nd_zero)
{
	size_t m, n;
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op2 = ops[n];
		if (!allow_2nd_zero && op2 == 0)
			continue;
		for (m = 0; m < sizeof(ops)/sizeof(ops[0]); ++m) {
			uint32_t op1 = ops[m];
			uint32_t r1 = ref(op1, op2);
			uint32_t r2 = f(op1, op2);
			assert(r1 == r2);
		}
	}
}

static void
exercise_bin_function_curryleft(
	bin_function_t ref, un_function_t f, uint32_t op1, bool allow_2nd_zero)
{
	size_t n;
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op2 = ops[n];
		if (!allow_2nd_zero && op2 == 0)
			continue;
		uint32_t r1 = ref(op1, op2);
		uint32_t r2 = f(op2);
		assert(r1 == r2);
	}
}

static void
exercise_bin_function_curryright(bin_function_t ref, un_function_t f, uint32_t op2)
{
	size_t n;
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op1 = ops[n];
		uint32_t r1 = ref(op1, op2);
		uint32_t r2 = f(op1);
		assert(r1 == r2);
	}
}

static void
exercise_un_function(un_function_t ref, un_function_t f)
{
	size_t n;
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op = ops[n];
		uint32_t r1 = ref(op);
		uint32_t r2 = f(op);
		assert(r1 == r2);
	}
}

static void
verify_un_function(un_function_t ref, un_op_factory_t factory)
{
	un_function_t f = generate_un_function(factory);
	exercise_un_function(ref, f);
}

static void
verify_bin_function(bin_function_t ref, bin_op_factory_t factory, bool allow_2nd_zero)
{
	bin_function_t f = generate_bin_function(factory);
	exercise_bin_function(ref, f, allow_2nd_zero);
	
	size_t n;
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op1 = ops[n];
		un_function_t fl = generate_bin_function_curryleft(factory, op1);
		exercise_bin_function_curryleft(ref, fl, op1, allow_2nd_zero);
	}
	
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op2 = ops[n];
		if (op2 == 0 && !allow_2nd_zero)
			continue;
		un_function_t fl = generate_bin_function_curryright(factory, op2);
		exercise_bin_function_curryright(ref, fl, op2);
	}
}

static void
verify_bin_cmp_function(bin_function_t ref, bin_op_factory_t factory)
{
	bin_function_t f = generate_bin_cmp_function(factory);
	exercise_bin_function(ref, f, true);
	
	size_t n;
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op1 = ops[n];
		un_function_t fl = generate_bin_cmp_function_curryleft(factory, op1);
		exercise_bin_function_curryleft(ref, fl, op1, true);
	}
	
	for (n = 0; n < sizeof(ops)/sizeof(ops[0]); ++n) {
		uint32_t op2 = ops[n];
		un_function_t fr = generate_bin_cmp_function_curryright(factory, op2);
		exercise_bin_function_curryright(ref, fr, op2);
	}
}

static uint32_t
ref_add(uint32_t a, uint32_t b) { return a + b; }

static uint32_t
ref_sub(uint32_t a, uint32_t b) { return a - b; }

static uint32_t
ref_band(uint32_t a, uint32_t b) { return a & b; }

static uint32_t
ref_bor(uint32_t a, uint32_t b) { return a | b; }

static uint32_t
ref_bxor(uint32_t a, uint32_t b) { return a ^ b; }

static uint32_t
ref_mul(uint32_t a, uint32_t b) { return a * b; }

static uint32_t
ref_muluhi(uint32_t a, uint32_t b)
{
	uint64_t r = (((uint64_t) a) * (uint64_t) b);
	return r >> (32ULL);
}

static uint32_t
ref_mulshi(uint32_t a, uint32_t b)
{
	uint64_t r = (((int64_t)(int32_t)a) * (int64_t)(int32_t)b);
	return r >> (32ULL);
}

static uint32_t
ref_udiv(uint32_t a, uint32_t b) { return a / b; }

static uint32_t
ref_sdiv(uint32_t a, uint32_t b) { return (int32_t)a / (int32_t)b; }

static uint32_t
ref_umod(uint32_t a, uint32_t b) { return a % b; }

static uint32_t
ref_smod(uint32_t a, uint32_t b) { return (int32_t)a % (int32_t)b; }

static uint32_t
ref_shr(uint32_t a, uint32_t b) { return a >> b; }

static uint32_t
ref_ashr(uint32_t a, uint32_t b) { return (int32_t) a >> (int32_t) b; }

static uint32_t
ref_shl(uint32_t a, uint32_t b) { return a << b; }

static uint32_t
ref_neg(uint32_t a) { return -a; }

static uint32_t
ref_not(uint32_t a) { return ~a; }

static uint32_t
ref_eq(uint32_t a, uint32_t b) { return a == b; }

static uint32_t
ref_neq(uint32_t a, uint32_t b) { return a != b; }

static uint32_t
ref_slt(uint32_t a, uint32_t b) { return (int32_t)a < (int32_t)b; }

static uint32_t
ref_ult(uint32_t a, uint32_t b) { return a < b; }

static uint32_t
ref_sle(uint32_t a, uint32_t b) { return (int32_t)a <= (int32_t)b; }

static uint32_t
ref_ule(uint32_t a, uint32_t b) { return a <= b; }

static uint32_t
ref_sgt(uint32_t a, uint32_t b) { return (int32_t)a > (int32_t)b; }

static uint32_t
ref_ugt(uint32_t a, uint32_t b) { return a > b; }

static uint32_t
ref_sge(uint32_t a, uint32_t b) { return (int32_t)a >= (int32_t)b; }

static uint32_t
ref_uge(uint32_t a, uint32_t b) { return a >= b; }

static jive::output *
wrap_bitand(jive::output * a, jive::output * b)
{
	jive::output * tmparray7[] = {a, b};
	return jive_bitand(2, tmparray7);
}

static jive::output *
wrap_bitor(jive::output * a, jive::output * b)
{
	jive::output * tmparray8[] = {a, b};
	return jive_bitor(2, tmparray8);
}

static jive::output *
wrap_bitxor(jive::output * a, jive::output * b)
{
	jive::output * tmparray9[] = {a, b};
	return jive_bitxor(2, tmparray9);
}

static jive::output *
wrap_bitsum(jive::output * a, jive::output * b)
{
	jive::output * tmparray10[] = {a, b};
	return jive_bitsum(2, tmparray10);
}

static jive::output *
wrap_bitmultiply(jive::output * a, jive::output * b)
{
	jive::output * tmparray11[] = {a, b};
	return jive_bitmultiply(2, tmparray11);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	verify_bin_function(ref_band, wrap_bitand, true);
	verify_bin_function(ref_bor, wrap_bitor, true);
	verify_bin_function(ref_bxor, wrap_bitxor, true);
	verify_bin_function(ref_add, wrap_bitsum, true);
	verify_bin_function(ref_sub, jive_bitdifference, true);
	verify_bin_function(ref_mul, wrap_bitmultiply, true);
	verify_bin_function(ref_muluhi, jive_bituhiproduct, true);
	verify_bin_function(ref_mulshi, jive_bitshiproduct, true);
	verify_bin_function(ref_udiv, jive_bituquotient, false);
	verify_bin_function(ref_sdiv, jive_bitsquotient, false);
	verify_bin_function(ref_umod, jive_bitumod, false);
	verify_bin_function(ref_smod, jive_bitsmod, false);
	verify_bin_function(ref_shr, jive_bitshr, true);
	verify_bin_function(ref_ashr, jive_bitashr, true);
	verify_bin_function(ref_shl, jive_bitshl, true);
	
	verify_un_function(ref_neg, jive_bitnegate);
	verify_un_function(ref_not, jive_bitnot);
	
	verify_bin_cmp_function(ref_eq, jive_bitequal);
	verify_bin_cmp_function(ref_neq, jive_bitnotequal);
	verify_bin_cmp_function(ref_slt, jive_bitsless);
	verify_bin_cmp_function(ref_ult, jive_bituless);
	verify_bin_cmp_function(ref_sle, jive_bitslesseq);
	verify_bin_cmp_function(ref_ule, jive_bitulesseq);
	verify_bin_cmp_function(ref_sgt, jive_bitsgreater);
	verify_bin_cmp_function(ref_ugt, jive_bitugreater);
	verify_bin_cmp_function(ref_sge, jive_bitsgreatereq);
	verify_bin_cmp_function(ref_uge, jive_bitugreatereq);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("i386/test-instr-match", test_main);
