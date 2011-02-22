#include <assert.h>
#include <locale.h>

#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/view.h>

int main()
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	setlocale(LC_ALL, "");
	
	jive_view(graph, stdout);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}