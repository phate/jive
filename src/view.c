#include <jive/view.h>
#include <jive/view/graphview.h>
#include <jive/util/textcanvas.h>

void
jive_view(struct jive_graph * graph, FILE * out)
{
	jive_graphview * graphview = jive_graphview_create(graph);
	jive_graphview_draw(graphview);
	//printf("%d %d\n", graphview->width, graphview->height);
	jive_textcanvas_write(&graphview->canvas, out);
	fflush(out);
	jive_graphview_destroy(graphview);
}
