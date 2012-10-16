/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_REGIONVIEW_H
#define JIVE_VIEW_REGIONVIEW_H

typedef struct jive_regionview jive_regionview;

struct jive_region;
struct jive_nodeview;
struct jive_reservationtracker;
struct jive_textcanvas;

struct jive_regionview {
	struct jive_region * region;
	struct jive_graphview * graphview;
	int x;
	unsigned int width, start_row_index, end_row_index;
	int upper_border_offset, lower_border_offset;
	
	struct {
		jive_regionview * first;
		jive_regionview * last;
	} subregions;
	
	struct {
		jive_regionview * prev;
		jive_regionview * next;
	} regionview_subregions_list;
	
	struct {
		struct jive_nodeview * first;
		struct jive_nodeview * last;
	} nodes;
};

jive_regionview *
jive_regionview_create(struct jive_graphview * graphview, struct jive_region * region);

void
jive_regionview_destroy(jive_regionview * self);

void
jive_regionview_move_horizontal(jive_regionview * self, int offset);

void
jive_regionview_layout(jive_regionview * self, struct jive_reservationtracker * parent_reservation);

void
jive_regionview_draw(jive_regionview * self, struct jive_textcanvas * dst);

#endif
