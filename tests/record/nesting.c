#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/record.h>
#include <jive/bitstring.h>

int main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * c0 = jive_bitconstant(graph, 8, "00000000");
	jive_output * c1 = jive_bitconstant(graph, 8, "00000001");
	jive_output * c2 = jive_bitconstant(graph, 4, "0010");
	jive_output * c3 = jive_bitconstant(graph, 6, "000011");

	const jive_record_layout_element l0_elements[] = {
		{(jive_value_type *)jive_output_get_type(c1), 0},
		{(jive_value_type *)jive_output_get_type(c2), 1}
	};
	jive_record_layout * l0 = jive_record_layout_create(context,
		2, l0_elements, 4, 2);

	jive_output * g0 = jive_group_create(l0, 2, (jive_output * []){c1, c2});
	
	const jive_record_layout_element l1_elements[] = {
		{(jive_value_type*)jive_output_get_type(c0),0},
		{(jive_value_type*)jive_output_get_type(g0),1},
		{(jive_value_type*)jive_output_get_type(c3),4}
	};
	jive_record_layout * l1 = jive_record_layout_create(context,
		3, l1_elements, 4, 4);

	jive_output * g1 = jive_group_create(l1, 3, (jive_output * []){c0, g0, c3});
	
	jive_output * s0 = jive_select_create(2, g1);
	jive_output * s1 = jive_select_create(1, g1);
	jive_output * s2 = jive_select_create(1, s1);

	assert(jive_type_equals(jive_output_get_type(s0), jive_output_get_type(c3)));
	assert(jive_type_equals(jive_output_get_type(s1), jive_output_get_type(g0)));
	assert(jive_type_equals(jive_output_get_type(s2), jive_output_get_type(c2)));

	jive_record_layout_destroy(l0);
	jive_record_layout_destroy(l1);
	jive_view(graph, stderr);

	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);	

	return 0;
}
