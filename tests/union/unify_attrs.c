#include <assert.h>

#include <jive/context.h>
#include <jive/vsdg.h>
#include <jive/bitstring.h>
#include <jive/vsdg/union.h>

int main()
{
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	static const jive_value_type * l_elements[] = { &bits8.base };
	static const jive_union_declaration l = {1, l_elements};

	jive_output * c = jive_bitconstant(graph, 8, "11110000");
	jive_output * g = jive_unify_create(&l, 0, c);

	const jive_unify_node_attrs * attrs = (const jive_unify_node_attrs *)
		jive_node_get_attrs(g->node);
	assert(attrs);
	assert(attrs->decl);

	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}
