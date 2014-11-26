/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_REGIONVIEW_H
#define JIVE_VIEW_REGIONVIEW_H

#include <vector>

struct jive_region;
struct jive_textcanvas;

class jive_graphview;
class jive_nodeview;
class jive_reservationtracker;

class jive_regionview {
public:
	~jive_regionview() noexcept;

	jive_regionview(jive_graphview * graphview, jive_region * region);

	struct jive_region * region;
	jive_graphview * graphview;
	int x;
	unsigned int width, start_row_index, end_row_index;
	int upper_border_offset, lower_border_offset;

	std::vector<jive_nodeview*> nodes;
	std::vector<jive_regionview*> subregions;
};

void
jive_regionview_move_horizontal(jive_regionview * self, int offset);

void
jive_regionview_layout(jive_regionview * self, jive_reservationtracker * parent_reservation);

void
jive_regionview_draw(jive_regionview * self, struct jive_textcanvas * dst);

#endif
