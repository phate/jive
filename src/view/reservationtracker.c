/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <stdbool.h>
#include <string.h>
#include <jive/view/reservationtracker.h>

void
jive_reservationtracker_init(jive_reservationtracker * self)
{
	self->min_x = 0;
	self->max_x = 0;
	self->min_y = 32767;
	self->max_y = -32767;
	
	self->cells = 0;
	self->stride = 0;
}

void
jive_reservationtracker_fini(jive_reservationtracker * self)
{
	delete[] self->cells;
}

static void
ensure_size(
	jive_reservationtracker * self,
	int new_min_x,
	int new_max_x,
	int new_min_y,
	int new_max_y)
{
	bool need_resize = false;
	if (new_min_x >= self->min_x) new_min_x = self->min_x; else need_resize = true;
	if (new_max_x <= self->max_x) new_max_x = self->max_x; else need_resize = true;
	if (new_min_y >= self->min_y) new_min_y = self->min_y; else need_resize = true;
	if (new_max_y <= self->max_y) new_max_y = self->max_y; else need_resize = true;
	
	if (!need_resize) return;
	
	if (new_max_x > new_min_x && new_max_y > new_min_y) {
		uint8_t * cells = new uint8_t[(new_max_x - new_min_x) * (new_max_y - new_min_y)];
		memset(cells, 0, (new_max_x - new_min_x) * (new_max_y - new_min_y));
		unsigned int stride = new_max_x - new_min_x;
		int y;
		
		for(y=self->min_y; y<self->max_y; y++) {
			memcpy(cells + stride * (y-new_min_y) + (self->min_x - new_min_x), self->cells + self->stride * (y-self->min_y), self->stride);
		}
		delete[] self->cells;
		self->cells = cells;
		self->stride = stride;
	}
	self->min_x = new_min_x;
	self->max_x = new_max_x;
	self->min_y = new_min_y;
	self->max_y = new_max_y;
}

static bool
available_rect(
	const jive_reservationtracker * self,
	int min_x, int max_x, int min_y, int max_y)
{
	if (min_x < self->min_x) min_x = self->min_x;
	if (max_x > self->max_x) max_x = self->max_x;
	if (min_y < self->min_y) min_y = self->min_y;
	if (max_y > self->max_y) max_y = self->max_y;
	
	if (min_x >= max_x || min_y >= max_y) return true;
	
	int x, y;
	for(y=min_y; y<max_y; y++) {
		for(x=min_x; x<max_x; x++) {
			if (self->cells[(x-self->min_x) + (y-self->min_y)*self->stride]) return false;
		}
	}
	return true;
}

static bool
available_rects(
	const jive_reservationtracker * self,
	size_t nrects,
	const jive_reservationrect rects[],
	int x_offset)
{
	size_t n;
	for(n=0; n<nrects; n++) {
		if (!available_rect(self,
			rects[n].x + x_offset, rects[n].x + rects[n].width + x_offset,
			rects[n].y, rects[n].y + rects[n].height))
			return false;
	}
	return true;
}

int
jive_reservationtracker_find_space(
	jive_reservationtracker * self,
	size_t nrects,
	const jive_reservationrect rects[],
	int x_offset)
{
	int n = 0;
	for(;;) {
		if (available_rects(self, nrects, rects, x_offset + n)) return x_offset + n;
		if (available_rects(self, nrects, rects, x_offset - n)) return x_offset - n;
		n++;
	}
}

static void
reserve_rect(
	jive_reservationtracker * self,
	int min_x, int max_x, int min_y, int max_y)
{
	ensure_size(self, min_x, max_x, min_y, max_y);
	
	int x, y;
	for(y=min_y; y<max_y; y++) {
		for(x=min_x; x<max_x; x++) {
			self->cells[(x-self->min_x) + (y-self->min_y)*self->stride] = 1;
		}
	}
}

void
jive_reservationtracker_reserve_boxes(
	jive_reservationtracker * self,
	size_t nrects,
	const jive_reservationrect rects[],
	int x_offset)
{
	size_t n;
	for(n=0; n<nrects; n++) {
		reserve_rect(self,
			rects[n].x + x_offset, rects[n].x + rects[n].width + x_offset,
			rects[n].y, rects[n].y + rects[n].height);
	}
}
