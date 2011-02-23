#include <jive/view.h>
#include <jive/view/graphview.h>
#include <jive/util/textcanvas.h>

void
jive_view(struct jive_graph * graph, FILE * out)
{
	jive_graphview * graphview = jive_graphview_create(graph);
	jive_graphview_draw(graphview);
	jive_textcanvas_write(&graphview->canvas, out);
	fflush(out);
	jive_graphview_destroy(graphview);
}

wchar_t *
jive_view_wstring(struct jive_graph * graph)
{
	jive_graphview * graphview = jive_graphview_create(graph);
	jive_graphview_draw(graphview);
	wchar_t * tmp = jive_textcanvas_as_wstring(&graphview->canvas);
	jive_graphview_destroy(graphview);
	
	return tmp;
}

char *
jive_view_string(struct jive_graph * graph)
{
	jive_graphview * graphview = jive_graphview_create(graph);
	jive_graphview_draw(graphview);
	char * tmp = jive_textcanvas_as_string(&graphview->canvas);
	jive_graphview_destroy(graphview);
	
	return tmp;
}

char *
jive_view_utf8(struct jive_graph * graph)
{
	jive_graphview * graphview = jive_graphview_create(graph);
	jive_graphview_draw(graphview);
	char * tmp = jive_textcanvas_as_utf8(&graphview->canvas);
	jive_graphview_destroy(graphview);
	
	return tmp;
}

