/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/codegen.h>
#include <jive/arch/dataobject.h>
#include <jive/arch/instruction.h>
#include <jive/arch/label-mapper.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/types/bitstring.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/types/union/unntype.h>
#include <jive/types/union/unnunify.h>
#include <jive/util/buffer.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/objdef.h>

typedef jive::output *(*data_def_fn)(jive_graph *);

static void
verify_asm_definition(jive_context * ctx, data_def_fn data_def, const char * expected_data)
{
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_memlayout_mapper_simple layout_mapper;
	jive_memlayout_mapper_simple_init(&layout_mapper, 32);
	
	jive::output * value = data_def(graph);
	jive::output * dataobj = jive_dataobj(value, &layout_mapper.base.base);
	jive_linker_symbol my_label_symbol;
	jive::output * name = jive_objdef_create(
		dataobj,
		"my_label",
		&my_label_symbol);
	jive_graph_export(graph, name);
	
	jive_buffer buffer;
	jive_symbol_name_pair symtab[] = {{&my_label_symbol, "my_label"}};
	jive_label_name_mapper * name_mapper = jive_label_name_mapper_simple_create(symtab, 1);
	jive_graph_generate_assembler(graph, name_mapper, &buffer);
	jive_label_name_mapper_destroy(name_mapper);
	jive_buffer_putbyte(&buffer, 0);
	
	static const char expected_header[] =
		".section .data\n"
		".globl my_label\n"
		"my_label:\n"
	;
	
	char * buffer_str = static_cast<char*>(&buffer.data[0]);
	
	assert(strncmp(buffer_str, expected_header, strlen(expected_header)) == 0);
	assert(strncmp(buffer_str + strlen(expected_header), expected_data, strlen(expected_data)) == 0);
	
	jive_memlayout_mapper_simple_fini(&layout_mapper);
	
	jive_graph_destroy(graph);
	jive_context_assert_clean(ctx);
}

static const char bits[] = "01010101010101010101010101010101";

static jive::output *
make_8bit_const(jive_graph * graph)
{
	return jive_bitconstant(graph, 8, bits);
}

static jive::output *
make_16bit_const(jive_graph * graph)
{
	return jive_bitconstant(graph, 16, bits);
}

static jive::output *
make_32bit_const(jive_graph * graph)
{
	return jive_bitconstant(graph, 32, bits);
}

static jive::output *
make_record1(jive_graph * graph)
{
	static const jive::bits::type bits32(32);
	static const jive::bits::type bits16(16);
	static const jive::bits::type bits8(8);
	
	static const jive::value::type * elements1[] = {&bits32, &bits16, &bits8};
	static const jive::rcd::declaration decl = {
		nelements : 3,
		elements : elements1
	};
	
	jive::output * c1 = jive_bitconstant(graph, 32, bits);
	jive::output * c2 = jive_bitconstant(graph, 16, bits);
	jive::output * c3 = jive_bitconstant(graph, 8, bits);
	jive::output *  tmparray0[] = {c1, c2, c3};
	
	return jive_group_create(&decl, 3, tmparray0);
}

static jive::output *
make_record2(jive_graph * graph)
{
	static const jive::bits::type bits32(32);
	static const jive::bits::type bits16(16);
	
	static const jive::value::type * elements1[] = {&bits16, &bits16};
	static const jive::rcd::declaration decl1 = {
		nelements : 2,
		elements : elements1
	};
	static jive::rcd::type rec1(&decl1);
	
	static const jive::value::type * elements2[] = {&rec1, &bits32};
	static const jive::rcd::declaration decl2 = {
		nelements : 2,
		elements : elements2
	};
	
	jive::output * c1 = jive_bitconstant(graph, 32, bits);
	jive::output * c2 = jive_bitconstant(graph, 16, bits);
	jive::output * tmparray1[] = {c2, c2};
	
	jive::output * tmp = jive_group_create(&decl1, 2, tmparray1);
	jive::output *  tmparray2[] = {tmp, c1};
	
	return jive_group_create(&decl2, 2, tmparray2);
}

static jive::output *
make_union1(jive_graph * graph)
{
	static const jive::bits::type bits32(32);
	static const jive::bits::type bits16(16);
	
	static const jive::value::type * elements1[] = {&bits16, &bits32};
	static const jive::unn::declaration decl1 = {2, elements1};
	
	jive::output * c = jive_bitconstant(graph, 16, bits);
	
	return jive_unify_create(&decl1, 0, c);
}

static jive::output *
make_union2(jive_graph * graph)
{
	static const jive::bits::type bits32(32);
	static const jive::bits::type bits16(16);
	
	static const jive::value::type * elements1[] = {&bits16, &bits32};
	static const jive::unn::declaration decl1 = {2, elements1};
	
	jive::output * c = jive_bitconstant(graph, 32, bits);
	
	return jive_unify_create(&decl1, 1, c);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * ctx = jive_context_create();
	
	verify_asm_definition(
		ctx,
		make_8bit_const,
		"\t.byte 0xaa\n");
	verify_asm_definition(
		ctx,
		make_16bit_const,
		"\t.value 0xaaaa\n");
	verify_asm_definition(
		ctx,
		make_32bit_const,
		"\t.long 0xaaaaaaaa\n");
	verify_asm_definition(
		ctx,
		make_record1,
		"\t.long 0xaaaaaaaa\n" "\t.value 0xaaaa\n" "\t.byte 0xaa\n" "\t.byte 0x0\n");
	verify_asm_definition(
		ctx,
		make_record2,
		"\t.value 0xaaaa\n" "\t.value 0xaaaa\n" "\t.long 0xaaaaaaaa\n");
	verify_asm_definition(
		ctx,
		make_union1,
		"\t.value 0xaaaa\n" "\t.byte 0x0\n" "\t.byte 0x0\n");
	verify_asm_definition(
		ctx,
		make_union2,
		"\t.long 0xaaaaaaaa\n");
	
	assert(jive_context_is_empty(ctx));
	
	jive_context_destroy(ctx);
	
	return 0;
}


JIVE_UNIT_TEST_REGISTER("arch/test-dataobj", test_main);
