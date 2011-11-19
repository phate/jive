#include <locale.h>
#include <assert.h>

#include <jive/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/address.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/call.h>
#include <jive/arch/loadstore.h>
#include <jive/arch/memlayout-simple.h>
#include <jive/arch/memory.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/label.h>

int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_ADDRESS_TYPE(addr);
	JIVE_DECLARE_MEMORY_TYPE(mem);
	JIVE_DECLARE_BITSTRING_TYPE(bits64, 64);
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		3, (const jive_type *[]){bits64, bits64, mem});

	jive_output * address0 = jive_bitstring_to_address_create(top->outputs[0], 64);
	jive_output * address1 = jive_bitstring_to_address_create(top->outputs[1], 64);

	jive_record_declaration decl = {2,
		(const jive_value_type *[]){jive_value_type_cast(addr), jive_value_type_cast(addr)}};	

	jive_output * memberof = jive_memberof(address0, &decl, 0);
	jive_output * containerof = jive_containerof(address1, &decl, 1);

	jive_label_external write_label;
	jive_label_external_init(&write_label, context, "write", (intptr_t) &write);	
	jive_output * label = jive_label_to_address_create(graph, &write_label.base);
	jive_node * call = jive_call_by_address_node_create(graph->root_region,
		label, NULL,
		2, (jive_output *[]){memberof, containerof},
		2, (const jive_type *[]){addr, addr});

	jive_output * constant = jive_bitconstant_unsigned(graph, 64, 1);	
	jive_output * arraysub = jive_arraysubscript(call->outputs[0], jive_value_type_cast(addr),
		constant); 

	jive_output * arrayindex = jive_arrayindex(call->outputs[0], call->outputs[1],
		jive_value_type_cast(addr), bits64);
	
	jive_output * load = jive_load_by_address_create(arraysub, jive_value_type_cast(addr),
		1, &top->outputs[2]);
	jive_node * store = jive_store_by_address_node_create(graph->root_region, arraysub,
		jive_value_type_cast(bits64), arrayindex, 1, &top->outputs[2]);

	jive_output * o_addr = jive_address_to_bitstring_create(load, 64);
	
	jive_node * bottom = jive_node_create(graph->root_region,
		2, (const jive_type *[]){bits64, mem}, (jive_output *[]){o_addr, store->outputs[0]},
		0, NULL);
	jive_node_reserve(bottom);

	jive_view(graph, stdout);

	jive_memlayout_mapper_simple mapper;
	jive_memlayout_mapper_simple_init(&mapper, context, 64);

	jive_graph_address_transform(graph, &mapper.base.base);

	jive_graph_prune(graph);
	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	jive_label_external_fini(&write_label);
	jive_memlayout_mapper_simple_fini(&mapper);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}