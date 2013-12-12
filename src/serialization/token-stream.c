/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/serialization/token-stream.h>

#include <inttypes.h>
#include <stdio.h>

#include <jive/util/buffer.h>

bool
jive_token_equals(const jive_token * self, const jive_token * other)
{
	if (self->type != other->type)
		return false;
	switch (self->type) {
		case jive_token_identifier:
			return strcmp(self->v.identifier, other->v.identifier) == 0;
		case jive_token_string:
			if (self->v.string.len != other->v.string.len)
				return false;
			return memcmp(self->v.string.str, other->v.string.str, self->v.string.len) == 0;
		case jive_token_integral:
			return self->v.integral == other->v.integral;
		default:
			return true;
	}
}

typedef struct cached_string cached_string;

struct cached_string {
	char * data;
	size_t space;
};

static void
cached_string_init(cached_string * self)
{
	self->data = 0;
	self->space = 0;
}

static void
cached_string_fini(cached_string * self, jive_context * context)
{
	jive_context_free(context, self->data);
}

static void
cached_string_swap(cached_string * self, cached_string * other)
{
	char * tmp_data = self->data;
	self->data = other->data;
	other->data = tmp_data;
	
	size_t tmp_space = self->space;
	self->space = other->space;
	other->space = tmp_space;
}

static void
cached_string_assign(cached_string * self, const char * str, size_t len, jive_context * context)
{
	if (self->space <= len) {
		self->space = len + 1;
		self->data = jive_context_realloc(context, self->data, self->space);
	}
	memcpy(self->data, str, len);
	self->data[len] = 0;
}


typedef struct jive_token_ostream_simple jive_token_ostream_simple;

struct jive_token_ostream_simple {
	jive_token_ostream base;
	
	jive_buffer * buffer;
	jive_context * context;
	size_t indent;
	size_t paren_nest;
	bool need_indent;
	bool need_whitespace;
};

static void
jive_token_ostream_simple_destroy_(jive_token_ostream * self_)
{
	jive_token_ostream_simple * self = (jive_token_ostream_simple *) self_;
	jive_context_free(self->context, self);
}

static void
jive_token_ostream_simple_newline(jive_token_ostream_simple * self)
{
	jive_buffer_putbyte(self->buffer, '\n');
	self->need_indent = true;
	self->need_whitespace = false;
}

static void
jive_token_ostream_simple_flush_indent(jive_token_ostream_simple * self)
{
	if (!self->need_indent)
		return;
	size_t n;
	for (n = 0; n < self->indent; n++)
		jive_buffer_putbyte(self->buffer, ' ');
	self->need_indent = false;
	self->need_whitespace = false;
}

static void
jive_token_ostream_simple_flush_whitespace(jive_token_ostream_simple * self)
{
	if (!self->need_whitespace)
		return;
	jive_buffer_putbyte(self->buffer, ' ');
	self->need_whitespace = false;
}

static void
jive_token_ostream_simple_put_(jive_token_ostream * self_, const jive_token * token)
{
	jive_token_ostream_simple * self = (jive_token_ostream_simple *) self_;
	switch (token->type) {
		case jive_token_end: {
			break;
		}
		case jive_token_identifier: {
			jive_token_ostream_simple_flush_indent(self);
			jive_token_ostream_simple_flush_whitespace(self);
			jive_buffer_putstr(self->buffer,
				token->v.identifier);
			self->need_whitespace = true;
			break;
		}
		case jive_token_string: {
			jive_token_ostream_simple_flush_indent(self);
			jive_token_ostream_simple_flush_whitespace(self);
			jive_buffer_putbyte(self->buffer, '"');
			jive_buffer_put(self->buffer,
				token->v.string.str,
				token->v.string.len);
			jive_buffer_putbyte(self->buffer, '"');
			self->need_whitespace = true;
			break;
		}
		case jive_token_integral: {
			jive_token_ostream_simple_flush_whitespace(self);
			char repr[64];
			snprintf(repr, sizeof(repr), "%"PRId64, token->v.integral);
			jive_buffer_putstr(self->buffer, repr);
			self->need_whitespace = true;
			break;
		};
		default: {
			if (token->type == jive_token_openparen || token->type == jive_token_lt)
				self->paren_nest ++;
			else if (token->type == jive_token_closeparen || token->type == jive_token_gt)
				self->paren_nest --;
			
			self->need_whitespace = false;
			if (token->type == jive_token_closebrace) {
				self->indent --;
			}
			jive_token_ostream_simple_flush_indent(self);
			jive_buffer_putbyte(self->buffer, token->type);
			if (token->type == jive_token_openbrace) {
				self->indent ++;
				jive_token_ostream_simple_newline(self);
			} else if (token->type == jive_token_semicolon) {
				if (!self->paren_nest)
					jive_token_ostream_simple_newline(self);
			} else if (token->type == jive_token_closebrace) {
				jive_token_ostream_simple_newline(self);
			}
			break;
		}
	}
}

static const jive_token_ostream_class JIVE_TOKEN_OSTREAM_SIMPLE = {
	.destroy = jive_token_ostream_simple_destroy_,
	.put = jive_token_ostream_simple_put_
};

jive_token_ostream *
jive_token_ostream_simple_create(jive_buffer * buffer)
{
	jive_token_ostream_simple * self;
	self = jive_context_malloc(buffer->context, sizeof(*self));
	self->base.class_ = &JIVE_TOKEN_OSTREAM_SIMPLE;
	self->context = buffer->context;
	self->buffer = buffer;
	self->indent = 0;
	self->paren_nest = 0;
	self->need_indent = true;
	self->need_whitespace = false;
	return &self->base;
}

typedef struct jive_token_istream_simple jive_token_istream_simple;
struct jive_token_istream_simple {
	jive_token_istream base;
	jive_context * context;
	const char * begin;
	const char * end;
	const char * parse_point;
	
	cached_string current_str;
	cached_string next_str;
};

/* character classification functions, need to be locale-independent */
static inline bool
is_whitespace(char c)
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static inline bool
is_idstart(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static inline bool
is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static inline bool
is_idcont(char c)
{
	return is_idstart(c) || is_digit(c);
}

static inline bool
is_operator(char c)
{
	return
		c == '(' || c == ')' ||
		c == '{' || c == '}' ||
		c == '<' || c == '>' ||
		c == ';' || c == ',' || c == '=' || c == ':' || c == '.' ||
		c == '-' || c == '+';
}

static void
jive_token_istream_simple_skip_whitespace(jive_token_istream_simple * self)
{
	while (self->parse_point != self->end && is_whitespace(*self->parse_point))
		self->parse_point ++;
}

static inline bool
match_keyword(const char * begin, const char * end, const char * keyword)
{
	size_t len = strlen(keyword);
	return (end - begin == len) && (memcmp(begin, keyword, len) == 0);
}

struct keyword_tokens {
	jive_token_type token;
	char keyword[16];
};

static const struct keyword_tokens keywords[] = {
	{ jive_token_gate, "gate"},
	{ jive_token_node, "node"},
	{ jive_token_region, "region"},
	{ jive_token_label, "label"},
	{ jive_token_stackptr, "stackptr"},
	{ jive_token_frameptr, "frameptr"}
};

static void
jive_token_istream_simple_parse(jive_token_istream_simple * self)
{
	jive_token_istream_simple_skip_whitespace(self);
	if (self->parse_point == self->end) {
		self->base.next.type = jive_token_end;
		return;
	}
	
	char c = *self->parse_point;
	
	if (is_idstart(c)) {
		const char * p = self->parse_point + 1;
		while (p != self->end && is_idcont(*p))
			++p;
		
		jive_token_type keyword = jive_token_keyword_last_plus1;
		for (size_t n = 0; n < sizeof(keywords)/sizeof(keywords[0]); ++n) {
			if (match_keyword(self->parse_point, p, keywords[n].keyword)) {
				keyword = keywords[n].token;
				break;
			}
		}
		
		if (keyword != jive_token_keyword_last_plus1) {
			self->base.next.type = keyword;
		} else {
			self->base.next.type = jive_token_identifier;
			size_t len = p - self->parse_point;
			cached_string_assign(&self->next_str, self->parse_point, len, self->context);
			self->base.next.v.identifier = self->next_str.data;
		}
		self->parse_point = p;
		return;
	}
	
	if (is_digit(c)) {
		const char * p = self->parse_point;
		int64_t value = 0;
		while (p != self->end && is_digit(*p)) {
			value = 10 * value + (*p - '0');
			++p;
		}
		self->base.next.type = jive_token_integral;
		self->base.next.v.integral = value;
		self->parse_point = p;
		return;
	}
	
	if (c == '"') {
		const char * begin = self->parse_point + 1;
		const char * end = begin;
		while (end != self->end && *end != '"')
			++end;
		const char * next = end;
		if (end == self->end) {
			self->base.next.type = jive_token_error;
			return;
		}
		cached_string_assign(&self->next_str, begin, end - begin, self->context);
		self->base.next.type = jive_token_string;
		self->base.next.v.string.str = self->next_str.data;
		self->base.next.v.string.len = end - begin;
		self->parse_point = next + 1;
		return;
	}
	
	if (is_operator(c)) {
		self->base.next.type = (jive_token_type) c;
		self->parse_point ++;
		return;
	}
	
	self->base.next.type = jive_token_error;
}

static void
jive_token_istream_simple_destroy_(jive_token_istream * self_)
{
	jive_token_istream_simple * self = (jive_token_istream_simple *) self_;
	cached_string_fini(&self->current_str, self->context);
	cached_string_fini(&self->next_str, self->context);
	jive_context_free(self->context, self);
}

static void
jive_token_istream_simple_advance_(jive_token_istream * self_)
{
	jive_token_istream_simple * self = (jive_token_istream_simple *) self_;
	self->base.current = self->base.next;
	cached_string_swap(&self->current_str, &self->next_str);
	jive_token_istream_simple_parse(self);
}

static const jive_token_istream_class JIVE_TOKEN_ISTREAM_SIMPLE = {
	.destroy = jive_token_istream_simple_destroy_,
	.advance = jive_token_istream_simple_advance_
};

jive_token_istream *
jive_token_istream_simple_create(jive_context * context,
	const char * begin, const char * end)
{
	jive_token_istream_simple * self = jive_context_malloc(context, sizeof(*self));
	self->base.class_ = &JIVE_TOKEN_ISTREAM_SIMPLE;
	self->context = context;
	self->begin = begin;
	self->end = end;
	self->parse_point = begin;
	cached_string_init(&self->current_str);
	cached_string_init(&self->next_str);
	jive_token_istream_simple_parse(self);
	self->base.current = self->base.next;
	cached_string_swap(&self->current_str, &self->next_str);
	jive_token_istream_simple_parse(self);
	return &self->base;
}
