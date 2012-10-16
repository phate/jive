/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_H
#define JIVE_VIEW_H

#include <stdio.h>
#include <wchar.h>

struct jive_graph;

void
jive_view(struct jive_graph * graph, FILE * out);

/**
	\brief Return graph represented as unicode string
*/
wchar_t *
jive_view_wstring(struct jive_graph * graph);

/**
	\brief Return graph represented as (locale-dependent) string
*/
char *
jive_view_string(struct jive_graph * graph);

/**
	\brief Return graph represented as utf8 string
*/
char *
jive_view_utf8(struct jive_graph * graph);

#endif
