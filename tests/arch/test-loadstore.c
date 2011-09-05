#include <assert.h>
#include <locale.h>

#include <jive/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/arch/loadstore.h>
#include <jive/arch/memory.h>
#include <jive/vsdg/node-private.h>
#include <jive/arch/addresstype.h>

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	JIVE_DECLARE_BITSTRING_TYPE(valuetype, 32);
	JIVE_DECLARE_MEMORY_TYPE(memtype);
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]) {addrtype, memtype});
	
	jive_output * address = top->outputs[0];
	jive_output * memstate = top->outputs[1];
	
	jive_node * load = jive_load_node_create(graph->root_region,
		address, valuetype,
		1, (jive_output *[]) {memstate});
	
	jive_output * value = load->outputs[0];
	
	jive_node * store = jive_store_node_create(graph->root_region,
		address, valuetype, value,
		1, (jive_output *[]) {memstate});
	
	memstate = store->outputs[0];
	
	jive_node * bottom = jive_node_create(graph->root_region,
		1, (const jive_type *[]) {memtype}, (jive_output *[]){memstate},
		0, NULL);
	(void) bottom;
	
	jive_view(graph, stdout);
	
	jive_context * context2 = jive_context_create();
	jive_graph * graph2 = jive_graph_copy(graph, context2);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	jive_view(graph2, stdout);
	
	jive_graph_destroy(graph2);
	assert(jive_context_is_empty(context2));
	jive_context_destroy(context2);
	
	return 0;
}
