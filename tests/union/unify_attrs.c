#include <assert.h>

#include <jive/context.h>
#include <jive/vsdg.h>
#include <jive/bitstring.h>
#include <jive/vsdg/unionlayout.h>
#include <jive/vsdg/union.h>

int main()
{
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_BITSTRING_TYPE(bits8, 8);

	jive_union_layout * l = jive_union_layout_create(context,
		1, (const jive_union_layout_element * []){(jive_value_type *)bits8},
		1, 1);

	jive_output * c = jive_bitconstant(graph, 8, "11110000");
	jive_output * g = jive_unify_create(l, 0, c);

	const jive_unify_node_attrs * attrs = (const jive_unify_node_attrs *)
		jive_node_get_attrs(g->node);
	assert(attrs);
	assert(attrs->layout);

	jive_union_layout_destroy(l);
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
