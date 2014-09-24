/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_UTIL_TEXTCANVAS_H
#define JIVE_UTIL_TEXTCANVAS_H

#include <stdbool.h>
#include <wchar.h>
#include <stdio.h>

#include <string>
#include <vector>

typedef struct jive_textcanvas jive_textcanvas;

struct jive_textcanvas {
	unsigned short width, height, stride;
	std::vector<wchar_t> data;
};

bool
jive_textcanvas_init(jive_textcanvas * canvas, unsigned int width, unsigned int height);

void
jive_textcanvas_write(jive_textcanvas * canvas, FILE * stream);

void
jive_textcanvas_put_ascii(jive_textcanvas * canvas,
	int x, int y,
	const char * ascii_string);

void
jive_textcanvas_hline(jive_textcanvas * canvas,
	int x1, int y, int x2,
	bool thick, bool stipple);

void
jive_textcanvas_vline(jive_textcanvas * canvas,
	int x, int y1, int y2,
	bool thick, bool stipple);

void
jive_textcanvas_box(jive_textcanvas * canvas,
	int x1, int y1, int x2, int y2,
	bool thick, bool stipple);

/**
	\brief Return contents of text-canvas as unicode string
*/
std::vector<wchar_t>
jive_textcanvas_as_wstring(jive_textcanvas * canvas);

/**
	\brief Return contents of text-canvas as (locale-dependent) string
*/
std::string
jive_textcanvas_as_string(jive_textcanvas * canvas);

/**
	\brief Return contents of text-canvas as utf-8 encoded unicode string
*/
std::string
jive_textcanvas_as_utf8(jive_textcanvas * canvas);

#endif
