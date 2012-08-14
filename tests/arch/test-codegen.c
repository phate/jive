#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/arch/instruction.h>
#include <jive/types/bitstring/constant.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/dataobject.h>
#include <jive/arch/codegen_buffer.h>

static int test_main(void)
{
	setlocale(LC_ALL, "");	

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * c8 = jive_bitconstant_unsigned(graph, 8, 8);
	jive_output * c16 = jive_bitconstant_unsigned(graph, 16, 16);
	jive_output * c32 = jive_bitconstant_unsigned(graph, 32, 32);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, context, 32);
	jive_dataobj(c8, &mapper.base.base);
	jive_rodataobj(c16, &mapper.base.base);
	jive_bssobj(c32, &mapper.base.base);

	jive_view(graph, stderr);

	jive_codegen_buffer buffer;
	jive_codegen_buffer_init(&buffer, context);
	jive_graph_generate_code(graph, &buffer);

	assert(buffer.data_buffer.size == 1);
	assert(((int8_t *)buffer.data_buffer.data)[0] == 8);

	assert(buffer.rodata_buffer.size == 2);
	assert(((int16_t *)buffer.rodata_buffer.data)[0] == 16); 

	assert(buffer.bss_buffer.size == 4);
	assert(((int32_t *)buffer.bss_buffer.data)[0] == 32);
	
	jive_codegen_buffer_fini(&buffer);
	jive_memlayout_mapper_simple_fini(&mapper);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-codegen", test_main);