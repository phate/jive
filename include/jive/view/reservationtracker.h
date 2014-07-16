/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VIEW_RESERVATIONTRACKER_H
#define JIVE_VIEW_RESERVATIONTRACKER_H

#include <stdint.h>
#include <stdlib.h>

class jive_reservationtracker {
public:
	int min_x, max_x, min_y, max_y;
	
	uint8_t * cells;
	unsigned int stride;
};

class jive_reservationrect {
public:
	int x, y, width, height;
};

void
jive_reservationtracker_init(jive_reservationtracker * self);

void
jive_reservationtracker_fini(jive_reservationtracker * self);

int
jive_reservationtracker_find_space(
	jive_reservationtracker * self,
	size_t nrects,
	const jive_reservationrect rects[],
	int x_offset);

void
jive_reservationtracker_reserve_boxes(
	jive_reservationtracker * self,
	size_t nrects,
	const jive_reservationrect rects[],
	int x_offset);

#endif
