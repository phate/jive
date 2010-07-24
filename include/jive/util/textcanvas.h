#ifndef JIVE_UTIL_TEXTCANVAS_H
#define JIVE_UTIL_TEXTCANVAS_H

#include <stdbool.h>
#include <wchar.h>
#include <stdio.h>

typedef struct jive_textcanvas jive_textcanvas;

struct jive_textcanvas {
	unsigned short width, height, stride;
	wchar_t * data;
};

bool
jive_textcanvas_init(jive_textcanvas * canvas, unsigned int width, unsigned int height);

void
jive_textcanvas_fini(jive_textcanvas * canvas);

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

#endif
