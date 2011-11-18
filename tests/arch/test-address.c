#include <assert.h>
#include <locale.h>

#include <jive/context.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/arch/address.h>
#include <jive/bitstring.h>
#include <jive/vsdg/node-private.h>

int main()
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	JIVE_DECLARE_BITSTRING_TYPE(bits32, 32);
	
	jive_record_declaration rec = {
		.nelements = 2,
		.elements = (const jive_value_type *[]){(jive_value_type *)bits32, (jive_value_type *)bits32}
	};
	
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	
	jive_node * top = jive_node_create(graph->root_region,
		0, NULL, NULL,
		2, (const jive_type *[]) {addrtype, addrtype});
	
	jive_output * memb1 = jive_memberof(top->outputs[0], &rec, 0);
	jive_output * memb2 = jive_memberof(top->outputs[0], &rec, 1);
	
	jive_output * cont1 = jive_containerof(memb1, &rec, 0);
	jive_output * cont2 = jive_containerof(memb2, &rec, 0);
	
	jive_output * cont3 = jive_containerof(top->outputs[1], &rec, 0);
	
	jive_output * memb3 = jive_memberof(cont3, &rec, 0);
	jive_output * memb4 = jive_memberof(cont3, &rec, 1);
	
	jive_view(graph, stdout);
	
	assert(cont1 == top->outputs[0]);
	assert(cont2 != top->outputs[0]);
	
	assert(memb4 != top->outputs[1]);
	assert(memb3 == top->outputs[1]);
	
	jive_output * zero = jive_bitconstant(graph, 32, "00000000000000000000000000000000");
	jive_output * one = jive_bitconstant(graph, 32, "10000000000000000000000000000000");
	jive_output * minus_one = jive_bitconstant(graph, 32, "11111111111111111111111111111111");
	
	jive_output * a0 = jive_arraysubscript(top->outputs[0], jive_value_type_cast(bits32), zero);
	assert(a0 == top->outputs[0]);
	jive_output * a1 = jive_arraysubscript(top->outputs[0], jive_value_type_cast(bits32), one);
	assert(a1 != top->outputs[0]);
	jive_output * tmp = jive_arraysubscript(a1, jive_value_type_cast(bits32), minus_one);
	jive_view(graph, stdout);
	assert(tmp == a0);
	
	jive_output * diff = jive_arrayindex(a1, a0, jive_value_type_cast(bits32), bits32);
	assert(diff == one);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}
