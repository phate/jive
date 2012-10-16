/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <stdlib.h>
#include <stdint.h>
#include <jive/util/textcanvas.h>

/* each square may contain a line from the center to the four
principal directions outwards; combinations of lines are encoded
in the following way: 
- 0x01: thin line left
- 0x02: thin line up
- 0x04: thin line right
- 0x08: thin line down
- 0x10: thick line left
- 0x20: thick line up
- 0x40: thick line right
- 0x80: thick line down
*/

/* map encoded lines to unicode line drawing characters */
static const wchar_t code_to_char[256] = {
	32, 9588, 9589, 9496, 9590, 9472, 9492, 9524, 9591, 9488, 9474, 9508, 9484, 9516, 9500, 9532,
	9592, 9592, 9497, 9497, 9598, 9598, 9525, 9525, 9489, 9489, 9509, 9509, 9517, 9517, 9533, 9533,
	9593, 9498, 9593, 9498, 9494, 9528, 9494, 9528, 9599, 9510, 9599, 9510, 9502, 9536, 9502, 9536,
	9499, 9499, 9499, 9499, 9529, 9529, 9529, 9529, 9513, 9513, 9513, 9513, 9539, 9539, 9539, 9539,
	9594, 9596, 9493, 9526, 9594, 9596, 9493, 9526, 9485, 9518, 9501, 9534, 9485, 9518, 9501, 9534,
	9473, 9473, 9527, 9527, 9473, 9473, 9527, 9527, 9519, 9519, 9535, 9535, 9519, 9519, 9535, 9535,
	9495, 9530, 9495, 9530, 9495, 9530, 9495, 9530, 9505, 9540, 9505, 9540, 9505, 9540, 9505, 9540,
	9531, 9531, 9531, 9531, 9531, 9531, 9531, 9531, 9543, 9543, 9543, 9543, 9543, 9543, 9543, 9543,
	9595, 9490, 9597, 9511, 9486, 9520, 9503, 9537, 9595, 9490, 9597, 9511, 9486, 9520, 9503, 9537,
	9491, 9491, 9514, 9514, 9521, 9521, 9541, 9541, 9491, 9491, 9514, 9514, 9521, 9521, 9541, 9541,
	9475, 9512, 9475, 9512, 9504, 9538, 9504, 9538, 9475, 9512, 9475, 9512, 9504, 9538, 9504, 9538,
	9515, 9515, 9515, 9515, 9545, 9545, 9545, 9545, 9515, 9515, 9515, 9515, 9545, 9545, 9545, 9545,
	9487, 9522, 9506, 9542, 9487, 9522, 9506, 9542, 9487, 9522, 9506, 9542, 9487, 9522, 9506, 9542,
	9523, 9523, 9544, 9544, 9523, 9523, 9544, 9544, 9523, 9523, 9544, 9544, 9523, 9523, 9544, 9544,
	9507, 9546, 9507, 9546, 9507, 9546, 9507, 9546, 9507, 9546, 9507, 9546, 9507, 9546, 9507, 9546,
	9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547, 9547,
};

static inline wchar_t
map_code_to_char(uint8_t code)
{
	return code_to_char[code];
}

#define LOWEST_CODE 9472
#define HIGHEST_CODE 9599
/* map unicode line drawing characters to line encoding */
static const uint8_t char_to_code[HIGHEST_CODE+1 - LOWEST_CODE] = {
	[9472-9472] = 0x05,
	[9473-9472] = 0x50,
	[9474-9472] = 0x0a,
	[9475-9472] = 0xa0,
	[9484-9472] = 0x0c,
	[9485-9472] = 0x48,
	[9486-9472] = 0x84,
	[9487-9472] = 0xc0,
	[9488-9472] = 0x09,
	[9489-9472] = 0x18,
	[9490-9472] = 0x81,
	[9491-9472] = 0x90,
	[9492-9472] = 0x06,
	[9493-9472] = 0x42,
	[9494-9472] = 0x24,
	[9495-9472] = 0x60,
	[9496-9472] = 0x03,
	[9497-9472] = 0x12,
	[9498-9472] = 0x21,
	[9499-9472] = 0x30,
	[9500-9472] = 0x0e,
	[9501-9472] = 0x4a,
	[9502-9472] = 0x2c,
	[9503-9472] = 0x86,
	[9504-9472] = 0xa4,
	[9505-9472] = 0x68,
	[9506-9472] = 0xc2,
	[9507-9472] = 0xe0,
	[9508-9472] = 0x0b,
	[9509-9472] = 0x1a,
	[9510-9472] = 0x29,
	[9511-9472] = 0x83,
	[9512-9472] = 0xa1,
	[9513-9472] = 0x38,
	[9514-9472] = 0x92,
	[9515-9472] = 0xb0,
	[9516-9472] = 0x0d,
	[9517-9472] = 0x1c,
	[9518-9472] = 0x49,
	[9519-9472] = 0x58,
	[9520-9472] = 0x85,
	[9521-9472] = 0x94,
	[9522-9472] = 0xc1,
	[9523-9472] = 0xd0,
	[9524-9472] = 0x07,
	[9525-9472] = 0x16,
	[9526-9472] = 0x43,
	[9527-9472] = 0x52,
	[9528-9472] = 0x25,
	[9529-9472] = 0x34,
	[9530-9472] = 0x61,
	[9531-9472] = 0x70,
	[9532-9472] = 0x0f,
	[9533-9472] = 0x1e,
	[9534-9472] = 0x4b,
	[9535-9472] = 0x5a,
	[9536-9472] = 0x2d,
	[9537-9472] = 0x87,
	[9538-9472] = 0xa5,
	[9539-9472] = 0x3c,
	[9540-9472] = 0x69,
	[9541-9472] = 0x96,
	[9542-9472] = 0xc3,
	[9543-9472] = 0x78,
	[9544-9472] = 0xd2,
	[9545-9472] = 0xb4,
	[9546-9472] = 0xe1,
	[9547-9472] = 0xf0,
	[9588-9472] = 0x01,
	[9589-9472] = 0x02,
	[9590-9472] = 0x04,
	[9591-9472] = 0x08,
	[9592-9472] = 0x10,
	[9593-9472] = 0x20,
	[9594-9472] = 0x40,
	[9595-9472] = 0x80,
	[9596-9472] = 0x41,
	[9597-9472] = 0x82,
	[9598-9472] = 0x14,
	[9599-9472] = 0x28,
};

static inline uint8_t
map_char_to_code(wchar_t character)
{
	if (character < LOWEST_CODE || character > HIGHEST_CODE)
		return 0;
	else
		return char_to_code[character - LOWEST_CODE];
}

bool
jive_textcanvas_init(jive_textcanvas * canvas, unsigned int width, unsigned int height)
{
	canvas->width = width;
	canvas->stride = width + 1;
	canvas->height = height;
	canvas->data = malloc((canvas->stride * canvas->height + 1) * sizeof(wchar_t));
	if (!canvas->data) return false;
	
	wchar_t * ptr = canvas->data;
	unsigned int x, y;
	for(y=0; y<height; y++) {
		for(x=0; x<width; x++)
			*ptr++ = L' ';
		*ptr++ = L'\n';
	}
	*ptr++ = 0;
	
	return canvas->data;
}

void
jive_textcanvas_fini(jive_textcanvas * canvas)
{
	free(canvas->data);
}

void
jive_textcanvas_write(jive_textcanvas * canvas, FILE * stream)
{
	fputws(canvas->data, stream);
}

void
jive_textcanvas_put_ascii(jive_textcanvas * canvas, int x, int y,
	const char * string)
{
	wchar_t * ptr = &canvas->data[y * canvas->stride + x];
	
	while(x < (int)canvas->width && *string) {
		*ptr++ = *string++;
		x++;
	}
}

static inline void
jive_textcanvas_put_line_code(jive_textcanvas * canvas,
	unsigned int x,
	unsigned int y,
	uint8_t code)
{
	wchar_t c = canvas->data[y * canvas->stride + x];
	code = code | map_char_to_code(c);
	canvas->data[y * canvas->stride + x] = map_code_to_char(code);
}

void
jive_textcanvas_hline(jive_textcanvas * canvas,
	int x1, int y, int x2,
	bool thick, bool stipple)
{
	uint8_t code = thick ? 0x55 : 0x05;
	uint8_t stipple_mask = 0x11;
	
	if (x1 == x2) return;
	if (x1 > x2) {
		int tmp = x1;
		x1 = x2;
		x2 = tmp;
		stipple_mask = 0x44;
	}
	
	if (stipple) code &= stipple_mask;
	
	jive_textcanvas_put_line_code(canvas, x1, y, code & 0x44);
	jive_textcanvas_put_line_code(canvas, x2, y, code & 0x11);
	
	int x;
	for(x=x1+1; x<x2; x++)
		jive_textcanvas_put_line_code(canvas, x, y, code);
}

void
jive_textcanvas_vline(jive_textcanvas * canvas,
	int x, int y1, int y2,
	bool thick,
	bool stipple)
{
	uint8_t code = thick ? 0xaa : 0x0a;
	uint8_t stipple_mask = 0x22;
	
	if (y1 == y2) return;
	if (y1 > y2) {
		int tmp = y1;
		y1 = y2;
		y2 = tmp;
		stipple_mask = 0x88;
	}
	
	if (stipple) code &= stipple_mask;
	
	jive_textcanvas_put_line_code(canvas, x, y1, code & 0x88);
	jive_textcanvas_put_line_code(canvas, x, y2, code & 0x22);
	
	int y;
	for(y=y1+1; y<y2; y++)
		jive_textcanvas_put_line_code(canvas, x, y, code);
}

void
jive_textcanvas_box(jive_textcanvas * canvas,
	int x1, int y1, int x2, int y2,
	bool thick, bool stipple)
{
	jive_textcanvas_hline(canvas, x1, y1, x2, thick, stipple);
	jive_textcanvas_vline(canvas, x2, y1, y2, thick, stipple);
	jive_textcanvas_hline(canvas, x2, y2, x1, thick, stipple);
	jive_textcanvas_vline(canvas, x1, y2, y1, thick, stipple);
}

wchar_t *
jive_textcanvas_as_wstring(jive_textcanvas * canvas)
{
	size_t nchars = canvas->height * (canvas->width + 1) + 1;
	wchar_t * data = malloc(sizeof(wchar_t) * nchars);
	if (!data) return NULL;
	
	size_t x, y;
	for(y = 0; y < canvas->height; y++) {
		for(x = 0; x < canvas->width; x++)
			data[y * (canvas->width + 1) + x] = canvas->data[y * canvas->stride + x];
		data[y * (canvas->width + 1) + x] = '\n';
	}
	data[nchars - 1] = 0;
	return data;
}

typedef struct dynstr {
	char * data;
	size_t size, space;
} dynstr;

static inline void
dynstr_init(dynstr * s)
{
	s->data = 0;
	s->size = 0;
	s->space = 0;
}

static inline void
dynstr_fini(dynstr * s)
{
	free(s->data);
}

static inline bool
dynstr_append(dynstr * s, const char * chars, size_t count)
{
	if (s->size + count >= s->space) {
		size_t space = s->space * 2 + count + 1;
		char * data = realloc(s->data, space);
		if (!data) return false;
		s->data = data;
		s->space = space;
	}
	size_t n;
	for(n = 0; n < count; n++)
		s->data[s->size + n] = chars[n];
	s->data[s->size + n] = 0;
	s->size = s->size + count;
	return true;
}

char *
jive_textcanvas_as_string(jive_textcanvas * canvas)
{
	dynstr ds;
	dynstr_init(&ds);
	
	mbstate_t state;
	wcrtomb(NULL, 0, &state);
	
	size_t x, y;
	for(y = 0; y < canvas->height; y++) {
		for(x = 0; x < canvas->width; x++) {
			char tmp[MB_CUR_MAX];
			size_t count = wcrtomb(tmp, canvas->data[y * canvas->stride + x], &state);
			if (count == (size_t)-1) {
				tmp[0] = '?';
				count = 1;
			}
			if (!dynstr_append(&ds, tmp, count)) {
				dynstr_fini(&ds);
				return NULL;
			}
		}
		if (!dynstr_append(&ds, "\n", 1)) {
			dynstr_fini(&ds);
			return NULL;
		}
	}
	
	return ds.data;
}

char *
jive_textcanvas_as_utf8(jive_textcanvas * canvas)
{
	dynstr ds;
	dynstr_init(&ds);
	
	mbstate_t state;
	wcrtomb(NULL, 0, &state);
	
	size_t x, y;
	for(y = 0; y < canvas->height; y++) {
		for(x = 0; x < canvas->width; x++) {
			wchar_t c = canvas->data[y * canvas->stride + x];
			
			size_t count = 0;
			char tmp[MB_CUR_MAX];
			
			if (c <= 0x7f) tmp[count++] = c;
			else if (c <= 0x7ff) {
				tmp[count++] = 0xc0 | (c >> 6);
				tmp[count++] = 0x80 | ((c >> 0) & 0x3f);
			} else if (c <= 0xffff) {
				tmp[count++] = 0xe0 | (c >> 12);
				tmp[count++] = 0x80 | ((c >> 6) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 0) & 0x3f);
			} else if (c <= 0x1fffff) {
				tmp[count++] = 0xf0 | (c >> 18);
				tmp[count++] = 0x80 | ((c >> 12) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 6) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 0) & 0x3f);
			} else if (c <= 0x3ffffff) {
				tmp[count++] = 0xf8 | (c >> 24);
				tmp[count++] = 0x80 | ((c >> 18) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 12) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 6) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 0) & 0x3f);
			} else {
				tmp[count++] = 0xfc | (c >> 30);
				tmp[count++] = 0x80 | ((c >> 24) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 18) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 12) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 6) & 0x3f);
				tmp[count++] = 0x80 | ((c >> 0) & 0x3f);
			}
			if (!dynstr_append(&ds, tmp, count)) {
				dynstr_fini(&ds);
				return NULL;
			}
		}
		if (!dynstr_append(&ds, "\n", 1)) {
			dynstr_fini(&ds);
			return NULL;
		}
	}
	
	return ds.data;
}
