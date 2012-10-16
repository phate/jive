/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <string.h>

#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/view.h>

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	setlocale(LC_ALL, "");
	
	wchar_t * ws = jive_view_wstring(graph);
	const wchar_t wreference[] = {9591, 9588, '\n', 9590, 9589, '\n', 0};
	assert(wcslen(ws) == wcslen(wreference));
	assert(wcscmp(ws, wreference) == 0);
	free(ws);
	
	char * s = jive_view_utf8(graph);
	const char reference[] = "\xe2\x95\xb7\xe2\x95\xb4\n\xe2\x95\xb6\xe2\x95\xb5\n";
	assert(strlen(s) == strlen(reference));
	assert(strcmp(s, reference) == 0);
	fputs(s, stdout);
	free(s);
	
	/* locale-dependent representation, nothing can be said about it */
	char * ls = jive_view_string(graph);
	assert(strlen(ls) >= 3 * 2);
	free(ls);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}
JIVE_UNIT_TEST_REGISTER("vsdg/test-empty-graph", test_main);
