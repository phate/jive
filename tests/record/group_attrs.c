#include <assert.h>

#include <jive/context.h>
#include <jive/vsdg.h>
#include <jive/bitstring.h>
#include <jive/vsdg/recordlayout.h>
#include <jive/vsdg/record.h>

int main()
{
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits8, 8);

	jive_record_layout * l = jive_record_layout_create(context,
		1, (const jive_record_layout_element[]){{(jive_value_type *)bits8, 0}},
		1, 1);	

	jive_output * c = jive_bitconstant(graph, 8, "11110000");
	jive_output * g = jive_group_create(l, 1, &c);

	const jive_group_node_attrs * attrs = (const jive_group_node_attrs *)
		jive_node_get_attrs(g->node);
	assert(attrs);
	assert(attrs->layout);

	jive_record_layout_destroy(l);
	jive_graph_destroy(graph);

	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
