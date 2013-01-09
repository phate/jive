/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>

#include <jive/types/bitstring.h>
#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/arch/codegen.h>
#include <jive/arch/dataobject.h>
#include <jive/arch/instruction.h>
#include <jive/arch/label-mapper.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/objdef.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/types/union/unnunify.h>
#include <jive/types/union/unntype.h>

typedef jive_output *(*data_def_fn)(jive_graph *);

static void
verify_asm_definition(jive_context * ctx, data_def_fn data_def, const char * expected_data)
{
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_memlayout_mapper_simple layout_mapper;
	jive_memlayout_mapper_simple_init(&layout_mapper, ctx, 32);
	
	jive_output * value = data_def(graph);
	jive_output * dataobj = jive_dataobj(value, &layout_mapper.base.base);
	jive_node * name = jive_objdef_node_create(dataobj, "my_label");
	jive_node_reserve(name);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer, ctx);
	jive_label_name_mapper * name_mapper = jive_label_name_mapper_simple_create(ctx);
	jive_graph_generate_assembler(graph, name_mapper, &buffer);
	jive_label_name_mapper_destroy(name_mapper);
	jive_buffer_putbyte(&buffer, 0);
	
	static const char expected_header[] =
		".section .data\n"
		".globl my_label\n"
		"my_label:\n"
	;
	
	char * buffer_str = (char *) buffer.data;
	
	assert(strncmp(buffer_str, expected_header, strlen(expected_header)) == 0);
	assert(strncmp(buffer_str + strlen(expected_header), expected_data, strlen(expected_data)) == 0);
	
	jive_buffer_fini(&buffer);
	
	jive_memlayout_mapper_simple_fini(&layout_mapper);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
}

static const char bits[] = "01010101010101010101010101010101";

static jive_output *
make_8bit_const(jive_graph * graph)
{
	return jive_bitconstant(graph, 8, bits);
}

static jive_output *
make_16bit_const(jive_graph * graph)
{
	return jive_bitconstant(graph, 16, bits);
}

static jive_output *
make_32bit_const(jive_graph * graph)
{
	return jive_bitconstant(graph, 32, bits);
}

static jive_output *
make_diff(jive_graph * graph)
{
	jive_output * c1 = jive_bitconstant(graph, 32, bits);
	jive_output * foo = jive_bitsymbolicconstant(graph, 32, "foo");
	return jive_bitsum(2, (jive_output *[]){c1, jive_bitnegate(foo)});
}

static jive_output *
make_record1(jive_graph * graph)
{
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	
	static const jive_value_type * elements1[] = {&bits32.base, &bits16.base, &bits8.base};
	static const jive_record_declaration decl = {
		.nelements = 3,
		.elements = elements1
	};
	
	jive_output * c1 = jive_bitconstant(graph, 32, bits);
	jive_output * c2 = jive_bitconstant(graph, 16, bits);
	jive_output * c3 = jive_bitconstant(graph, 8, bits);
	
	return jive_group_create(&decl, 3, (jive_output * []){c1, c2, c3});
}

static jive_output *
make_record2(jive_graph * graph)
{
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	
	static const jive_value_type * elements1[] = {&bits16.base, &bits16.base};
	static const jive_record_declaration decl1 = {
		.nelements = 2,
		.elements = elements1
	};
	static const jive_record_type rec1 = {{{&JIVE_RECORD_TYPE}}, &decl1};
	
	static const jive_value_type * elements2[] = {&rec1.base, &bits32.base};
	static const jive_record_declaration decl2 = {
		.nelements = 2,
		.elements = elements2
	};
	
	jive_output * c1 = jive_bitconstant(graph, 32, bits);
	jive_output * c2 = jive_bitconstant(graph, 16, bits);
	
	jive_output * tmp = jive_group_create(&decl1, 2, (jive_output *[]){c2, c2});
	
	return jive_group_create(&decl2, 2, (jive_output * []){tmp, c1});
}

static jive_output *
make_union1(jive_graph * graph)
{
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	
	static const jive_value_type * elements1[] = {&bits16.base, &bits32.base};
	static const jive_union_declaration decl1 = {
		.nelements = 2,
		.elements = elements1,
	};
	
	jive_output * c = jive_bitconstant(graph, 16, bits);
	
	return jive_unify_create(&decl1, 0, c);
}

static jive_output *
make_union2(jive_graph * graph)
{
	static const jive_bitstring_type bits32 = {{{&JIVE_BITSTRING_TYPE}}, 32};
	static const jive_bitstring_type bits16 = {{{&JIVE_BITSTRING_TYPE}}, 16};
	
	static const jive_value_type * elements1[] = {&bits16.base, &bits32.base};
	static const jive_union_declaration decl1 = {
		.nelements = 2,
		.elements = elements1,
	};
	
	jive_output * c = jive_bitconstant(graph, 32, bits);
	
	return jive_unify_create(&decl1, 1, c);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * ctx = jive_context_create();
	
	verify_asm_definition(ctx, make_8bit_const, "\t.byte 0xaa\n");
	verify_asm_definition(ctx, make_16bit_const, "\t.value 0xaaaa\n");
	verify_asm_definition(ctx, make_32bit_const, "\t.long 0xaaaaaaaa\n");
	verify_asm_definition(ctx, make_diff, "\t.long (0xaaaaaaaa + (- foo))\n");
	verify_asm_definition(ctx, make_record1, "\t.long 0xaaaaaaaa\n" "\t.value 0xaaaa\n" "\t.byte 0xaa\n" "\t.byte 0x00\n");
	verify_asm_definition(ctx, make_record2, "\t.value 0xaaaa\n" "\t.value 0xaaaa\n" "\t.long 0xaaaaaaaa\n");
	verify_asm_definition(ctx, make_union1, "\t.value 0xaaaa\n" "\t.byte 0x00\n" "\t.byte 0x00\n");
	verify_asm_definition(ctx, make_union2, "\t.long 0xaaaaaaaa\n");
	
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("arch/test-dataobj", test_main);
